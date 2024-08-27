#include "trigger_sound.h"
#include "BW/api.h"
#include "trigger_system.h"
#include "hooks/limits/sfx_extender.h"

static bool sound_paths_init;

static void init_sound_paths() {
	std::string file = mpq_load_file("arr\\sounds.txt");

	u16 sound_id = 0;
	u16 char_index = 0;
	for (const char* c = file.data(); c != file.data() + file.size(); c++) {
		if (c[0] == '\r' && c[1] == '\n') {
			char_index = 0;
			++sound_id;
			++c; // skip both
			continue;
		}
		if (c[0] == '\r' || c[0] == '\n') {
			char_index = 0;
			++sound_id;
			continue;
		}

		if (char_index < MAX_PATH) {
			sound_paths[sound_id][char_index++] = *c;
		}
	}
}

void init_sound() {
	if (!g_direct_sound) {
		init_direct_sound();
	}
	if (!sound_paths_init) {
		sound_paths_init = true;
		init_sound_paths();
	}

	for (u32 i = 0; i < MaxDupChannels; ++i) {
		if (g_dup_channels[i].ds_buffer) {
			g_dup_channels[i].ds_buffer->Release();
			g_dup_channels[i].ds_buffer = 0;
		}
	}

	for (u32 i = 0; i < g_loaded_sounds_count; ++i) {
		free_sound(&g_loaded_sounds[i]);
	}

	g_loaded_sounds_count = 0;
	memset(g_loaded_sounds, 0, sizeof(g_loaded_sounds));
	memset(g_loaded_sounds_hash_table, 0, sizeof(g_loaded_sounds_hash_table));

	memset(&g_current_playlist, 0, sizeof(g_current_playlist));
	memset(&g_music_playback, 0, sizeof(g_music_playback));
	g_music_playback.fade = 1;
	g_music_playback.target_fade = 1;
	g_music_playback.volume = 1;
	g_music_playback.target_volume = 1;
}

void init_direct_sound() {
	// Init DirectSound
	if (SUCCEEDED(DirectSoundCreate(0, &g_direct_sound, 0))) {
		WAVEFORMATEX wave_format = {};
		wave_format.wFormatTag = WAVE_FORMAT_PCM;
		wave_format.nChannels = 2;
		wave_format.nSamplesPerSec = 48000;
		wave_format.wBitsPerSample = 16;
		wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
		wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
		wave_format.cbSize = 0;

		if (SUCCEEDED(g_direct_sound->SetCooperativeLevel(*(HWND*)g_main_window, DSSCL_PRIORITY))) {
			DSBUFFERDESC buffer_description = {};
			buffer_description.dwSize = sizeof(DSBUFFERDESC);
			buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

			// @Todo: what is a primary buffer needed for? seems like sounds play without too 
			LPDIRECTSOUNDBUFFER primary_buffer = 0;
			if (SUCCEEDED(g_direct_sound->CreateSoundBuffer(&buffer_description, &primary_buffer, 0))) {
				primary_buffer->SetFormat(&wave_format);
			}
		}
	}
}

void update_sound() {
	for (u32 i = 0; i < g_loaded_sounds_count; ++i) {
		SoundMainBuffer* sound = &g_loaded_sounds[i];
		if (sound->ds_buffer && GetTickCount() - sound->last_play_time >= 10 * 1000) {
			DWORD status = 0;
			sound->ds_buffer->GetStatus(&status);
			if (status != DSBSTATUS_PLAYING) {
				sound->ds_buffer->Release();
				sound->ds_buffer = 0;
			}
		}
	}

	if (g_music_playback.current_music && g_music_playback.current_music->ds_buffer) {
		DWORD status = 0;
		g_music_playback.current_music->ds_buffer->GetStatus(&status);
		if (status != DSBSTATUS_PLAYING) {
			g_music_playback.current_music = 0;
		}
	} else {
		g_music_playback.current_music = 0;
	}

	if (g_music_playback.target_fade == 0) {
		if (g_music_playback.fade_out) {
			g_music_playback.fade -= (1.0 / g_music_playback.fade_out) / 24.0;
		} else {
			g_music_playback.fade = 0;
		}
		if (g_music_playback.fade <= 0) {
			g_music_playback.fade = 0;
			stop_music(g_music_playback.paused);
		}
	}

	if (!g_music_playback.current_music && !g_music_playback.paused) {
		if (g_music_playback.next_music) {
			g_music_playback.current_music = g_music_playback.next_music;
			g_music_playback.next_music = 0;
		} else {
			g_music_playback.current_music = get_next_music();
		}
		if (g_music_playback.current_music) {
			g_music_playback.target_fade = 1;
			play_music(false);
		}
	}

	if (g_music_playback.current_music || g_music_playback.paused) {
		if (g_music_playing) {
			g_music_playing = false; // So we don't do retarded lard fade which freezes the game
			helper::stop_bw_music();
		}
	} else if (!g_music_playing) {
		g_music_playing = true; // So we actually play the vanilla music
		helper::play_next_bw_music();
	}

	if (g_music_playback.target_fade == 1) {
		if (g_music_playback.fade_in) {
			g_music_playback.fade += (1.0 / g_music_playback.fade_in) / 24.0;
		} else {
			g_music_playback.fade = 1;
		}
		if (g_music_playback.fade >= 1) {
			g_music_playback.fade = 1;
		}
	}

	if (g_music_playback.volume_time) {
		f32 music_volume_delta = (1.0 / g_music_playback.volume_time) / 24.0;
		if (g_music_playback.volume < g_music_playback.target_volume) {
			g_music_playback.volume += music_volume_delta;
			if (g_music_playback.volume >= g_music_playback.target_volume) {
				g_music_playback.volume = g_music_playback.target_volume;
			}
		} else {
			g_music_playback.volume -= music_volume_delta;
			if (g_music_playback.volume <= g_music_playback.target_volume) {
				g_music_playback.volume = g_music_playback.target_volume;
			}
		}
	} else {
		g_music_playback.volume = g_music_playback.target_volume;
	}

	if (g_music_playback.current_music && g_music_playback.current_music->ds_buffer) {
		g_music_playback.current_music->ds_buffer->SetVolume(get_playback_volume());
	}
}

static bool load_ds_buffer(SoundMainBuffer* sound) {
	if (!g_direct_sound) {
		return false;
	}

	std::string file = mpq_load_file(sound->path);
	if (file.empty()) {
		bw::printf("Error: File %s missing!", sound->path);
		return false;
	}

	WAVEFORMATEX wave_format = {};
	size_t size = 0;
	s8* data = 0;

	u8* at = (u8*)file.data();
	void* end = file.data() + file.size();
	while (at < end) {
		u32* type = (u32*)at;
		switch (*type) {
			case RiffFormat_Riff: {
				WaveHeader* header = (WaveHeader*) at;
				at += sizeof(*header);
			} break;
			case RiffFormat_Data: {
				WaveData* wave_data = (WaveData*) at;
				at += wave_data->chunk_size + 8;

				size = wave_data->chunk_size;
				data = (s8*)(wave_data + 1);
			} break;
			case RiffFormat_Fmt: {
				WaveFormat* format = (WaveFormat*) at;
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
				// @Todo: log an error
			} break;
		}
	}

	DSBUFFERDESC buffer_description = {};
	buffer_description.dwSize = sizeof(DSBUFFERDESC);
	buffer_description.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;
	buffer_description.dwBufferBytes = size;
	buffer_description.lpwfxFormat = &wave_format;

	if (!SUCCEEDED(g_direct_sound->CreateSoundBuffer(&buffer_description, &sound->ds_buffer, 0))) {
		return false;
	}

	u32 play_cursor = 0;
	u32 write_cursor = 0;
	if (SUCCEEDED(sound->ds_buffer->GetCurrentPosition((LPDWORD)&play_cursor, (LPDWORD)&write_cursor))) {
		u8* ptr1 = 0;
		u32 bytes1 = 0;
		u8* ptr2 = 0;
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
	sound->ds_buffer = 0;
	return false;
}

SoundMainBuffer* load_sound(const char* filename) {
	if (!filename) {
		return 0;
	}
	if (g_loaded_sounds_count < MaxSound) {
		SoundMainBuffer* sound = &g_loaded_sounds[g_loaded_sounds_count++];
		snprintf(sound->path, MaxPath, "%s", filename);

		if (mpq_file_exists(sound->path)) {
			u32 hash_index = hash_string(filename) % MaxSoundHash;
			sound->hash_next = g_loaded_sounds_hash_table[hash_index];
			g_loaded_sounds_hash_table[hash_index] = sound;
			return sound;
		} else {
			g_loaded_sounds_count--;
			return 0;
		}
	}
	return 0;
}

SoundMainBuffer* get_sound(const char* filename) {
	u32 hash_index = hash_string(filename) % MaxSoundHash;
	SoundMainBuffer* sound = g_loaded_sounds_hash_table[hash_index];
	while (sound && strcmp(sound->path, filename) != 0) {
		sound = sound->hash_next;
	}
	if (!sound) {
		sound = load_sound(filename);
	}
	return sound;
}

SoundMainBuffer* get_sound(SoundId::Enum sound_id) {
    char buffer[MAX_PATH] = {};
    snprintf(buffer, MAX_PATH, "sound\\%s", sound_paths[sound_id]); // sfx_dat::sound_file[sound_id]);
    return get_sound(buffer);
}

void free_sound(SoundMainBuffer* sound) {
	if (sound->ds_buffer) {
		sound->ds_buffer->Release();
		sound->ds_buffer = 0;
	}
}

PlayingSound play_sound_raw(SoundMainBuffer* sound_buffer, s32 volume, s32 pan, Point16 position, s32 unit_id) {
	if (!sound_buffer) {
		return {};
	}
	if (!sound_buffer->ds_buffer) {
		if (!load_ds_buffer(sound_buffer)) {
			return {};
		}
	}
	if (sound_buffer->last_play_time && GetTickCount() - sound_buffer->last_play_time < 80) {
		return {};
	}

	DWORD status = 0;
	sound_buffer->ds_buffer->GetStatus(&status);
	if (status == DSBSTATUS_PLAYING || unit_id != -1) {
		// Use dup channel
		SoundDupBuffer* dup_buffer = 0;
		for (u32 i = 0; i < MaxDupChannels; ++i) {
			if (g_dup_channels[i].ds_buffer) {
				DWORD dup_status = 0;
				g_dup_channels[i].ds_buffer->GetStatus(&dup_status);
				if (dup_status != DSBSTATUS_PLAYING) {
					g_dup_channels[i].ds_buffer->Release();
					g_dup_channels[i].ds_buffer = 0;
				}
			}
			if (!g_dup_channels[i].ds_buffer) {
				dup_buffer = &g_dup_channels[i];
			}
		}

		if (dup_buffer) {
			g_direct_sound->DuplicateSoundBuffer(sound_buffer->ds_buffer, &dup_buffer->ds_buffer);
			if (dup_buffer->ds_buffer) {
				dup_buffer->ds_buffer->SetVolume(volume);
				dup_buffer->ds_buffer->SetPan(pan);
				dup_buffer->ds_buffer->SetCurrentPosition(0);
				dup_buffer->ds_buffer->Play(0, 0, 0);
				dup_buffer->unit_id = unit_id;
				dup_buffer->generation++;
				sound_buffer->last_play_time = GetTickCount();
				g_playing_sounds.emplace_back(dup_buffer);
				return {dup_buffer};
			}
		}
	} else {
		sound_buffer->ds_buffer->SetVolume(volume);
		sound_buffer->ds_buffer->SetPan(pan);
		sound_buffer->ds_buffer->SetCurrentPosition(0);
		sound_buffer->ds_buffer->Play(0, 0, 0);
		sound_buffer->last_play_time = GetTickCount();
		sound_buffer->generation++;
        g_playing_sounds.emplace_back(sound_buffer);
		return {sound_buffer};
	}
	return {};
}

PlayingSound play_sound_raw(SoundMainBuffer* sound, Point16 position, s32 unit_id) {
	return play_sound_raw(sound, helper::get_sfx_volume_from_screen_distance(position.x, position.y), helper::get_sfx_pan_from_screen_distance(position.x), position, unit_id);
}

PlayingSound play_sound_ext(SoundId::Enum sound_id) {
	return play_sound_raw(get_sound(sound_id), 1);
}

PlayingSound play_sound_ext(SoundId::Enum sound_id, Point16 position) {
	return play_sound_raw(get_sound(sound_id), position);
}

PlayingSound play_sound_ext(SoundId::Enum sound_id, Unit* unit) {
	return play_sound_raw(get_sound(sound_id), unit->position);
}

void play_music(bool resume) {
	if (g_music_playback.current_music) {
		if (!g_music_playback.current_music->ds_buffer) {
			load_ds_buffer(g_music_playback.current_music);
		}
		if (g_music_playback.current_music->ds_buffer) {
			g_music_playback.current_music->ds_buffer->SetVolume(get_playback_volume());
			g_music_playback.current_music->ds_buffer->SetPan(0);
			if (!resume) g_music_playback.current_music->ds_buffer->SetCurrentPosition(0);
			g_music_playback.current_music->ds_buffer->Play(0, 0, 0);
		}
	}
}

void stop_music(bool pause) {
	if (g_music_playback.current_music && g_music_playback.current_music->ds_buffer) {
		g_music_playback.current_music->ds_buffer->Stop();
	}
	if (!pause) g_music_playback.current_music = 0;
}

void shuffle_playlist() {
	for (u32 i = 0; i < g_current_playlist.size; ++i) {
		SoundMainBuffer* music_a = g_current_playlist.tracks[i];
		for (u32 j = i + 1; j < g_current_playlist.size; ++j) {
			if (i == 0 && j == g_current_playlist.size - 1) {
				// Avoid swapping first and last, so we don't hear the same sound twice
				continue;
			}
			SoundMainBuffer* music_b = g_current_playlist.tracks[j];
			if (rand() % 2 == 0) {
				SoundMainBuffer temp = *music_a;
				*music_a = *music_b;
				*music_b = temp;
			}
		}
	}
}

typedef int __cdecl getMusicVolume_Func();
static getMusicVolume_Func* getMusicVolume = (getMusicVolume_Func*)0x004A5EF0;

s32 get_sound_volume() {
	s32 index = 99;
	if (g_use_quiet_sounds) {
		index /= 5;
	}
	return g_big_volume_array[index * g_options_list->sfx / 100];
}

s32 get_sound_volume_at_pos(Point16 pos) {
	s32 index = helper::get_sfx_volume_from_screen_distance(pos.x, pos.y);
	if (g_use_quiet_sounds) {
		index /= 5;
	}
	if (index <= 10) {
		return DSBVOLUME_MIN;
	}
	return g_big_volume_array[index * g_options_list->sfx / 100];
}

s32 get_playback_volume() {
	if (g_music_playback.fade == 0 || g_music_playback.volume == 0) {
		return DSBVOLUME_MIN;
	}

	s32 volume = getMusicVolume();
	volume += (-4000 - volume) * (1 - g_music_playback.fade * g_music_playback.volume);
	return volume;
}

SoundMainBuffer* get_next_music() {
	if (!g_current_playlist.size) {
		return 0;
	}
	if (g_current_playlist.index == 0 && g_current_playlist.shuffle) {
		shuffle_playlist();
	}
	SoundMainBuffer* music = g_current_playlist.tracks[g_current_playlist.index++];
	if (g_current_playlist.index >= g_current_playlist.size) {
		g_current_playlist.index = 0;
	}
	return music;
}

void update_playing_sounds() {
	std::erase_if(g_playing_sounds, [](const PlayingSound& sound) { return !sound; });

	for (auto& sound : g_playing_sounds) {
		if (auto buffer = sound.GetBuffer(); buffer && buffer->position) {
			buffer->ds_buffer->SetVolume(helper::get_sfx_volume_from_screen_distance(buffer->position.x, buffer->position.y));
			buffer->ds_buffer->SetPan(helper::get_sfx_pan_from_screen_distance(buffer->position.x));
		}
	}
}

void PlayingSound::Stop() {
	if (!*this) {
		return;
	}
    m_buffer->ds_buffer->Stop();
}

SoundBuffer* PlayingSound::GetBuffer() {
	if (!*this) {
		return 0;
	}
	return m_buffer;
}

PlayingSound::operator bool() const {
	if (!m_buffer) {
		return false;
	}
	if (m_buffer->generation != m_generation) {
		return false;
	}

	DWORD status = 0;
	m_buffer->ds_buffer->GetStatus(&status);
	return status == DSBSTATUS_PLAYING;
}
