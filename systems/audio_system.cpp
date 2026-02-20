#include "audio_system.h"
#include "../BW/api.h"
#include "../BW/bwdata.h"
#include "../BW/bwhelper.h"
#include "../BW/enumerations.h"
#include "../utility.h"
#include "../hooks/limits/sfx_extender.h"
#include "../hooks/game_hooks.h"
#include <algorithm>
#include <format>

static constexpr s32 ScreenW = 640;
static constexpr s32 ScreenH = 400;

static bool is_on_screen(Point16 pos) {
	return pos.x >= g_move_to_x && pos.x <= g_move_to_x + ScreenW
		&& pos.y >= g_move_to_y && pos.y <= g_move_to_y + ScreenH;
}

bool SoundBuffer::is_blocking_play(SoundBuffer* other, Point16 other_pos) {
	auto* main = get_root_buffer();
	if (main != other->get_root_buffer()) return false;
	if (!last_play_time) return false;

	if (other_pos) {
		if (auto pos = settings.get_updated_position()) {
			if (abs(pos.x - other_pos.x) > ScreenW || abs(pos.y - other_pos.y) > ScreenH) {
				return false;
			}
		} else {
			if (is_on_screen(other_pos)) {
				return false;
			}
		}
	}

	return is_playing() && GetTickCount() - last_play_time < 80;
}

// TODO: Move this to bwhelper or reimplement
typedef int __cdecl getMusicVolume_Func();
static getMusicVolume_Func* getMusicVolume = (getMusicVolume_Func*)0x004A5EF0;

AudioSystem::AudioSystem() {
	if (!m_direct_sound) {
		initialize_direct_sound();
	}

	initialize_sound_paths();
}

AudioSystem::~AudioSystem() {
	stop_playing_sounds();

	for (auto& sound : m_dup_channels) {
		sound.free_buffer();
	}

	for (auto& sound : m_loaded_sounds) {
		sound.free_buffer();
	}

	if (m_direct_sound) {
		m_direct_sound->Release();
		m_direct_sound = nullptr;
	}
}

void AudioSystem::initialize_direct_sound() {
	if (SUCCEEDED(DirectSoundCreate(nullptr, &m_direct_sound, nullptr))) {
		WAVEFORMATEX wave_format = {};
		wave_format.wFormatTag = WAVE_FORMAT_PCM;
		wave_format.nChannels = 2;
		wave_format.nSamplesPerSec = 48000;
		wave_format.wBitsPerSample = 16;
		wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
		wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
		wave_format.cbSize = 0;

		if (SUCCEEDED(m_direct_sound->SetCooperativeLevel(g_main_window, DSSCL_PRIORITY))) {
			DSBUFFERDESC buffer_description = {};
			buffer_description.dwSize = sizeof(DSBUFFERDESC);
			buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

			// @Todo: what is a primary buffer needed for? seems like sounds play without too 
			LPDIRECTSOUNDBUFFER primary_buffer = nullptr;
			if (SUCCEEDED(m_direct_sound->CreateSoundBuffer(&buffer_description, &primary_buffer, nullptr))) {
				primary_buffer->SetFormat(&wave_format);
				primary_buffer->Release();
			}
		}
	}
}

void AudioSystem::initialize_sound_paths() {
	std::string file = mpq_load_file("arr\\sounds.txt");

	u32 line_count = std::count(file.begin(), file.end(), '\n');
	// Add one more for the last line if file doesn't end with newline
	if (!file.empty() && file.back() != '\n') {
		++line_count;
	}
	line_count = std::min(line_count, (u32)SoundIdLimit);

	m_sound_paths.resize(line_count);
	for (auto& path : m_sound_paths) {
		path.fill(0);
	}

	u16 sound_id = 0;
	u16 char_index = 0;
	for (const char* c = file.data(); c != file.data() + file.size(); c++) {
		if (c[0] == '\r') {
			continue;
		}
		if (c[0] == '\n') {
			char_index = 0;
			++sound_id;
			if (sound_id >= SoundIdLimit) {
				break;
			}
			continue;
		}

		if (char_index < MAX_PATH) {
			m_sound_paths[sound_id][char_index++] = *c;
		}
	}
}

void AudioSystem::update() {
	// Clean up buffers that are done playing
	for (u32 i = 0; i < m_loaded_sounds_count; ++i) {
		SoundMainBuffer* sound = &m_loaded_sounds[i];
		if (sound->ds_buffer && GetTickCount() - sound->last_play_time >= 10 * 1000) {
			DWORD status = 0;
			sound->ds_buffer->GetStatus(&status);
			if (!(status & DSBSTATUS_PLAYING)) {
				sound->ds_buffer->Release();
				sound->ds_buffer = nullptr;
			}
		}
	}

	// Check if current music is still playing
	if (m_music_playback.current_music && m_music_playback.current_music->ds_buffer) {
		DWORD status = 0;
		m_music_playback.current_music->ds_buffer->GetStatus(&status);
		if (!(status & DSBSTATUS_PLAYING)) {
			m_music_playback.current_music = nullptr;
		}
	} else {
		m_music_playback.current_music = nullptr;
	}

	// Handle fade out
	if (m_music_playback.target_fade == 0) {
		if (m_music_playback.fade_out) {
			m_music_playback.fade -= (1.0f / m_music_playback.fade_out) / 1secs;
		} else {
			m_music_playback.fade = 0;
		}
		if (m_music_playback.fade <= 0) {
			m_music_playback.fade = 0;
			stop_music(m_music_playback.paused);
		}
	}

	// Start next music if current music stopped
	if (!m_music_playback.current_music && !m_music_playback.paused) {
		if (m_music_playback.next_music) {
			m_music_playback.current_music = m_music_playback.next_music;
			m_music_playback.next_music = nullptr;
		} else {
			m_music_playback.current_music = get_next_music();
		}
		if (m_music_playback.current_music) {
			m_music_playback.target_fade = 1;
			play_music(false);
		}
	}

	// Manage BW music vs custom music
	if (m_music_playback.current_music || m_music_playback.paused) {
		if (g_music_playing) { // TODO: Fully disable built-in playback and stop using g_music_playing
			g_music_playing = false; // Disable built-in playback so we don't freeze on fade
			helper::stop_bw_music();
		}
	} else if (!g_music_playing) {
		g_music_playing = true;
		helper::play_next_bw_music();
	}

	// Handle fade in
	if (m_music_playback.target_fade == 1) {
		if (m_music_playback.fade_in) {
			m_music_playback.fade += (1.0f / m_music_playback.fade_in) / 1secs;
		} else {
			m_music_playback.fade = 1;
		}
		if (m_music_playback.fade >= 1) {
			m_music_playback.fade = 1;
		}
	}

	if (m_music_playback.volume_time) {
		f32 music_volume_delta = (1.0f / m_music_playback.volume_time) / 1secs;
		if (m_music_playback.volume < m_music_playback.target_volume) {
			m_music_playback.volume += music_volume_delta;
			if (m_music_playback.volume >= m_music_playback.target_volume) {
				m_music_playback.volume = m_music_playback.target_volume;
			}
		} else {
			m_music_playback.volume -= music_volume_delta;
			if (m_music_playback.volume <= m_music_playback.target_volume) {
				m_music_playback.volume = m_music_playback.target_volume;
			}
		}
	} else {
		m_music_playback.volume = m_music_playback.target_volume;
	}

	if (m_music_playback.current_music && m_music_playback.current_music->ds_buffer) {
		m_music_playback.current_music->ds_buffer->SetVolume(get_playback_volume());
	}
}

bool AudioSystem::load_ds_buffer(SoundMainBuffer* sound) {
	if (!m_direct_sound) {
		return false;
	}

	std::string file = mpq_load_file(sound->path);
	if (file.empty()) {
		bw::printf("Error: File %s missing!", sound->path);
		return false;
	}

	WAVEFORMATEX wave_format = {};
	size_t size = 0;
	s8* data = nullptr;

	u8* at = (u8*)file.data();
	void* end = file.data() + file.size();
	while (at < end) {
		u32* type = (u32*)at;
		switch (*type) {
			case RiffFormat_Riff: {
				WaveHeader* header = (WaveHeader*)at;
				at += sizeof(*header);
			} break;
			case RiffFormat_Data: {
				WaveData* wave_data = (WaveData*)at;
				at += wave_data->chunk_size + 8;

				size = wave_data->chunk_size;
				data = (s8*)(wave_data + 1);
			} break;
			case RiffFormat_Fmt: {
				WaveFormat* format = (WaveFormat*)at;
				at += format->chunk_size + 8;
				wave_format.wFormatTag = format->format_type;
				wave_format.nChannels = format->channels;
				wave_format.nSamplesPerSec = format->samples_per_sec;
				wave_format.nAvgBytesPerSec = format->average_bytes_per_sec;
				wave_format.nBlockAlign = format->block_align;
				wave_format.wBitsPerSample = format->bits_per_sample;
			} break;
			default: {
				at = (u8*)end;
			} break;
		}
	}

	DSBUFFERDESC buffer_description = {};
	buffer_description.dwSize = sizeof(DSBUFFERDESC);
	buffer_description.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;
	buffer_description.dwBufferBytes = size;
	buffer_description.lpwfxFormat = &wave_format;

	if (!SUCCEEDED(m_direct_sound->CreateSoundBuffer(&buffer_description, &sound->ds_buffer, nullptr))) {
		return false;
	}

	u32 play_cursor = 0;
	u32 write_cursor = 0;
	if (SUCCEEDED(sound->ds_buffer->GetCurrentPosition((LPDWORD)&play_cursor, (LPDWORD)&write_cursor))) {
		u8* ptr1 = nullptr;
		u32 bytes1 = 0;
		u8* ptr2 = nullptr;
		u32 bytes2 = 0;

		if (SUCCEEDED(sound->ds_buffer->Lock(write_cursor, size, (LPVOID*)&ptr1, (LPDWORD)&bytes1, (LPVOID*)&ptr2, (LPDWORD)&bytes2, 0))) {
			if (ptr1) {
				for (u32 i = 0; i < bytes1; ++i) {
					ptr1[i] = data[i];
				}
			}
			if (ptr2) {
				for (u32 i = 0; i < bytes2; ++i) {
					ptr2[i] = data[i];
				}
			}

			sound->ds_buffer->Unlock(ptr1, bytes1, ptr2, bytes2);
			return true;
		}
	}

	sound->ds_buffer->Release();
	sound->ds_buffer = nullptr;
	return false;
}

SoundMainBuffer* AudioSystem::load_sound(const std::string& filename) {
	if (filename.empty()) {
		return nullptr;
	}
	if (m_loaded_sounds_count < MaxSound) {
		if (mpq_file_exists(filename.c_str())) {
			SoundMainBuffer* sound = &m_loaded_sounds[m_loaded_sounds_count++];
			auto result = std::format_to_n(sound->path, MaxPath - 1, "{}", filename);
			sound->path[result.size] = '\0';
			m_loaded_sounds_map[filename] = sound;
			return sound;
		}
	}
	return nullptr;
}

SoundMainBuffer* AudioSystem::get_sound(const std::string& filename) {
	if (auto it = m_loaded_sounds_map.find(filename); it != m_loaded_sounds_map.end()) {
		return it->second;
	}
	return load_sound(filename);
}

SoundMainBuffer* AudioSystem::get_sound(SoundId::Enum sound_id) {
	if (sound_id >= m_sound_paths.size()) return nullptr;
	auto filename = std::format("sound\\{}", m_sound_paths[sound_id].data());
	return get_sound(filename);
}

PlayingSound AudioSystem::play_sound(SoundMainBuffer* sound_buffer, PlaySoundSettings settings, SoundId::Enum sound_id) {
	if (!sound_buffer) {
		return {};
	}
	if (!sound_buffer->ds_buffer) {
		if (!load_ds_buffer(sound_buffer)) {
			return {};
		}
	}

	const auto request_pos = settings.get_updated_position();
	if (g_sc_main_state == MainState::Run) {
		for (auto& sound : m_playing_sounds) {
			if (auto* buffer = sound.GetBuffer()) {
				if (buffer->is_blocking_play(sound_buffer, request_pos)) {
					return {};
				}
			}
		}
	}

	if (sound_id != SoundId::None) {
		u32 flags = sfx_dat::sound_flags[sound_id];
		if (flags & DatSoundFlags_OneAtATime) {
			if (sound_buffer->is_playing()) {
				return {};
			}
		}
		if (flags & DatSoundFlags_UnitResponse) {
			if (settings.unit) {
				settings.unit_response_id = settings.unit->id;
				if (const auto pos = settings.get_updated_position()) {
					const auto [volume, pan] = settings.calculate_volume_pan();
					if (volume == DSBVOLUME_MIN) {
						// atm we don't support starting banter outside of camera, for the sake of not having global collisions,
						// potentially could filter them by distance, by mimicking the math from	
						// helper::get_sfx_volume_from_screen_distance, but between two points
						return {};
					}
				}
				for (const auto& playing_sound : m_playing_sounds) {
					auto* buffer = playing_sound.GetBuffer();
					if (!buffer || buffer->settings.unit_response_id != settings.unit_response_id) {
						continue;
					}
					if (buffer->is_playing()) {
						return {};
					}
				}
			}
		}
	}

	DWORD status = 0;
	sound_buffer->ds_buffer->GetStatus(&status);
	if (!(status & DSBSTATUS_PLAYING)) {
		const auto [volume, pan] = settings.calculate_volume_pan();
		sound_buffer->ds_buffer->SetVolume(volume);
		sound_buffer->ds_buffer->SetPan(pan);
		sound_buffer->ds_buffer->SetCurrentPosition(0);
		sound_buffer->ds_buffer->Play(0, 0, settings.looping ? DSBPLAY_LOOPING : 0);
		sound_buffer->settings = settings;
		sound_buffer->generation++;
		sound_buffer->last_play_time = GetTickCount();
		m_playing_sounds.emplace_back(sound_buffer);
		return {sound_buffer};
	}

	// Use duplicate channel
	SoundDupBuffer* dup_buffer = nullptr;
	for (u32 i = 0; i < MaxDupChannels; ++i) {
		if (m_dup_channels[i].ds_buffer) {
			DWORD dup_status = 0;
			m_dup_channels[i].ds_buffer->GetStatus(&dup_status);
			if (!(dup_status & DSBSTATUS_PLAYING)) {
				m_dup_channels[i].ds_buffer->Release();
				m_dup_channels[i].ds_buffer = nullptr;
			}
		}
		if (!m_dup_channels[i].ds_buffer) {
			dup_buffer = &m_dup_channels[i];
			break;
		}
	}
	if (!dup_buffer) {
		return {};
	}

	m_direct_sound->DuplicateSoundBuffer(sound_buffer->ds_buffer, &dup_buffer->ds_buffer);
	if (!dup_buffer->ds_buffer) {
		return {};
	}
	const auto [volume, pan] = settings.calculate_volume_pan();
	dup_buffer->ds_buffer->SetVolume(volume);
	dup_buffer->ds_buffer->SetPan(pan);
	dup_buffer->ds_buffer->SetCurrentPosition(0);
	dup_buffer->ds_buffer->Play(0, 0, settings.looping ? DSBPLAY_LOOPING : 0);
	dup_buffer->parent_buffer = sound_buffer;
	dup_buffer->settings = settings;
	dup_buffer->generation++;
	dup_buffer->last_play_time = GetTickCount();
	m_playing_sounds.emplace_back(dup_buffer);
	return {dup_buffer};
}

PlayingSound AudioSystem::play_sound(SoundId::Enum sound_id, const PlaySoundSettings& settings) {
	return play_sound(get_sound(sound_id), settings, sound_id);
}

void AudioSystem::play_music(bool resume) {
	if (m_music_playback.current_music) {
		if (!m_music_playback.current_music->ds_buffer) {
			load_ds_buffer(m_music_playback.current_music);
		}
		if (m_music_playback.current_music->ds_buffer) {
			m_music_playback.current_music->ds_buffer->SetVolume(get_playback_volume());
			m_music_playback.current_music->ds_buffer->SetPan(0);
			if (!resume) m_music_playback.current_music->ds_buffer->SetCurrentPosition(0);
			m_music_playback.current_music->ds_buffer->Play(0, 0, 0);
		}
	}
}

void AudioSystem::stop_music(bool pause) {
	if (m_music_playback.current_music && m_music_playback.current_music->ds_buffer) {
		m_music_playback.current_music->ds_buffer->Stop();
	}
	if (!pause) m_music_playback.current_music = nullptr;
}

void AudioSystem::shuffle_playlist() {
	for (u32 i = 0; i < m_playlist.size; ++i) {
		SoundMainBuffer* music_a = m_playlist.tracks[i];
		for (u32 j = i + 1; j < m_playlist.size; ++j) {
			if (i == 0 && j == m_playlist.size - 1) {
				// Avoid swapping first and last, so we don't hear the same sound twice
				continue;
			}
			SoundMainBuffer* music_b = m_playlist.tracks[j];
			if (rand() % 2 == 0) {
				SoundMainBuffer temp = *music_a;
				*music_a = *music_b;
				*music_b = temp;
			}
		}
	}
}

void AudioSystem::queue_music(const std::string& filename) {
	m_music_playback.next_music = get_sound(filename);
	m_music_playback.target_fade = 0;
}

void AudioSystem::stop_music() {
	m_music_playback.target_fade = 0;
}

void AudioSystem::pause_music() {
	m_music_playback.target_fade = 0;
	m_music_playback.paused = true;
}

void AudioSystem::resume_music() {
	m_music_playback.target_fade = 1;
	m_music_playback.paused = false;
	play_music(true);
}

void AudioSystem::set_music_fade_in(f32 seconds) {
	m_music_playback.fade_in = seconds;
}

void AudioSystem::set_music_fade_out(f32 seconds) {
	m_music_playback.fade_out = seconds;
}

void AudioSystem::set_music_volume(f32 volume, f32 transition_time) {
	if (transition_time == 0.0f) {
		m_music_playback.volume = volume;
	}
	m_music_playback.target_volume = volume;
	m_music_playback.volume_time = transition_time;
}

void AudioSystem::clear_playlist() {
	m_playlist = {};
}

bool AudioSystem::add_to_playlist(const std::string& filename) {
	if (m_playlist.size >= MaxSound) {
		return false;
	}
	SoundMainBuffer* music = get_sound(filename);
	if (!music) {
		return false;
	}
	m_playlist.tracks[m_playlist.size++] = music;
	return true;
}

void AudioSystem::set_playlist_shuffle(bool shuffle) {
	m_playlist.shuffle = shuffle;
}

s32 AudioSystem::get_sound_volume() const {
	s32 index = 99;
	if (g_use_quiet_sounds) {
		index /= 5;
	}
	return g_big_volume_array[index * g_options_list->sfx / 100];
}

s32 AudioSystem::get_sound_volume_at_pos(Point16 pos) const {
	s32 index = helper::get_sfx_volume_from_screen_distance(pos.x, pos.y);
	if (g_use_quiet_sounds) {
		index /= 5;
	}
	if (index <= 10) {
		return DSBVOLUME_MIN;
	}
	return g_big_volume_array[index * g_options_list->sfx / 100];
}

s32 AudioSystem::get_playback_volume() const {
	if (m_music_playback.fade == 0 || m_music_playback.volume == 0) {
		return DSBVOLUME_MIN;
	}

	s32 volume = getMusicVolume();
	volume += (-4000 - volume) * (1 - m_music_playback.fade * m_music_playback.volume);
	return volume;
}

SoundMainBuffer* AudioSystem::get_next_music() {
	if (m_playlist.size == 0) {
		return nullptr;
	}
	if (m_playlist.index == 0 && m_playlist.shuffle) {
		shuffle_playlist();
	}
	SoundMainBuffer* music = m_playlist.tracks[m_playlist.index++];
	if (m_playlist.index >= m_playlist.size) {
		m_playlist.index = 0;
	}
	return music;
}

static bool is_main_window_focused() {
	return GetForegroundWindow() == g_main_window;
}

VolumePan PlaySoundSettings::calculate_volume_pan() {
	if (!g_wmode_loaded && !is_main_window_focused()) {
		return {DSBVOLUME_MIN, 0};
	}
	if (is_bw_hook) {
		return {bw_volume, bw_pan};
	}

	VolumePan result{99, 0};
	if (get_updated_position()) {
		result.volume = helper::get_sfx_volume_from_screen_distance(position.x, position.y);
		result.pan = helper::get_sfx_pan_from_screen_distance(position.x);

		const auto& tile = bw::get_tile_at_coords(position);
		const auto flags = bw::in_replay() ? ~*g_replay_visions : (u8)(1 << (u32)g_local_nation_id);
		if (tile.visibility_flags & flags) {
			result.volume = DSBVOLUME_MIN;
			return result;
		}
	}
	result.volume = std::max<s32>(result.volume, min_volume);
	result.volume *= g_options_list->sfx;
	if (g_use_quiet_sounds) {
		result.volume /= 5;
	}
	if (result.volume > 0) {
		result.volume = g_big_volume_array[result.volume / 100];
	} else {
		result.volume = DSBVOLUME_MIN;
	}
	return result;
}

void AudioSystem::update_playing_sounds() {
	std::erase_if(m_playing_sounds, [](const PlayingSound& sound) { return !sound.GetBuffer(); });

	for (auto& sound : m_playing_sounds) {
		if (auto buffer = sound.GetBuffer()) {
			if (buffer->settings.is_bw_hook) {
				continue;
			}

			const auto [volume, pan] = buffer->settings.calculate_volume_pan();
			buffer->ds_buffer->SetVolume(volume);
			buffer->ds_buffer->SetPan(pan);
		}
	}
}

void AudioSystem::stop_playing_sounds() {
	for (auto& sound : m_playing_sounds) {
		if (auto buffer = sound.GetBuffer()) {
			buffer->ds_buffer->Stop();
			buffer->settings = {};
			buffer->generation = 0;
		}
	}
	m_playing_sounds.clear();
}


// Hooked to stop endless tick sound in scoreboard and other issues
void AudioSystem::stop_all_sounds_partial() {
	// Stop all loaded sounds
	for (u32 i = 0; i < m_loaded_sounds_count; ++i) {
		SoundMainBuffer* sound = &m_loaded_sounds[i];
		if (sound->ds_buffer) {
			sound->ds_buffer->Stop();
		}
	}
	// Stop all duplicate channels
	for (u32 i = 0; i < MaxDupChannels; ++i) {
		SoundDupBuffer* dup_channel = &m_dup_channels[i];
		if (dup_channel->ds_buffer) {
			dup_channel->ds_buffer->Stop();
		}
	}
}

void AudioSystem::reset() {
	stop_playing_sounds();

	for (auto& sound : m_dup_channels) {
		sound.free_buffer();
	}

	for (auto& sound : m_loaded_sounds) {
		sound.free_buffer();
	}

	m_loaded_sounds_map.clear();
	m_sound_paths.clear();
	m_playing_sounds.clear();
	m_loaded_sounds_count = 0;

	m_music_playback = {};
	m_playlist = {};

	initialize_sound_paths();
}

void PlayingSound::Stop() const {
	if (!*this) {
		return;
	}
	m_buffer->ds_buffer->Stop();
}

SoundBuffer* PlayingSound::GetBuffer() const {
	if (!*this) {
		return nullptr;
	}
	return m_buffer;
}

PlayingSound::operator bool() const {
	if (!m_buffer || !m_buffer->ds_buffer) {
		return false;
	}
	if (m_buffer->generation != m_generation) {
		return false;
	}

	DWORD status = 0;
	m_buffer->ds_buffer->GetStatus(&status);
	return status & DSBSTATUS_PLAYING;
}