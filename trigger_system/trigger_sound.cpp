#include "trigger_sound.h"
#include "hooks\limits\sfx_extender.h"

LPDIRECTSOUND direct_sound;
DupChannel dup_channels[MaxDupChannels];

u32 loaded_sounds_count;
Sound loaded_sounds[MaxSound];
Sound* loaded_sounds_hash_table[MaxSoundHash];

Playlist current_playlist;
Playback playback;

static bool sound_paths_init;
char sound_paths[SoundIdLimit][MAX_PATH];

static void init_sound_paths() {
	SFile file = SFile::load("arr\\sounds.txt");

	u16 sound_id = 0;
	u16 char_index = 0;
	for (const char* c = file.begin(); c != file.end(); c++) {
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
	if (!direct_sound) {
		init_direct_sound();
	}
	if (!sound_paths_init) {
		sound_paths_init = true;
		init_sound_paths();
	}

	for (u32 i = 0; i < MaxDupChannels; ++i) {
		if (dup_channels[i].ds_buffer) {
			dup_channels[i].ds_buffer->Release();
			dup_channels[i].ds_buffer = 0;
		}
	}

	for (u32 i = 0; i < loaded_sounds_count; ++i) {
		free_sound(&loaded_sounds[i]);
	}

	loaded_sounds_count = 0;
	memset(loaded_sounds, 0, sizeof(loaded_sounds));
	memset(loaded_sounds_hash_table, 0, sizeof(loaded_sounds_hash_table));

	memset(&current_playlist, 0, sizeof(current_playlist));
	memset(&playback, 0, sizeof(playback));
	playback.fade = 1;
	playback.target_fade = 1;
	playback.volume = 1;
	playback.target_volume = 1;
}

void init_direct_sound() {
	// Init DirectSound
	if (SUCCEEDED(DirectSoundCreate(0, &direct_sound, 0))) {
		WAVEFORMATEX wave_format = {};
		wave_format.wFormatTag = WAVE_FORMAT_PCM;
		wave_format.nChannels = 2;
		wave_format.nSamplesPerSec = 48000;
		wave_format.wBitsPerSample = 16;
		wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
		wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
		wave_format.cbSize = 0;

		if (SUCCEEDED(direct_sound->SetCooperativeLevel(*(HWND*)g_main_window, DSSCL_PRIORITY))) {
			DSBUFFERDESC buffer_description = {};
			buffer_description.dwSize = sizeof(DSBUFFERDESC);
			buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

			// @Todo: what is a primary buffer needed for? seems like sounds play without too 
			LPDIRECTSOUNDBUFFER primary_buffer = 0;
			if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &primary_buffer, 0))) {
				primary_buffer->SetFormat(&wave_format);
			}
		}
	}
}

void update_sound() {
	for (u32 i = 0; i < loaded_sounds_count; ++i) {
		Sound* sound = &loaded_sounds[i];
		if (sound->ds_buffer && GetTickCount() - sound->last_play_time >= 10 * 1000) {
			DWORD status = 0;
			sound->ds_buffer->GetStatus(&status);
			if (status != DSBSTATUS_PLAYING) {
				sound->ds_buffer->Release();
				sound->ds_buffer = 0;
			}
		}
	}

	if (playback.current_music && playback.current_music->ds_buffer) {
		DWORD status = 0;
		playback.current_music->ds_buffer->GetStatus(&status);
		if (status != DSBSTATUS_PLAYING) {
			playback.current_music = 0;
		}
	} else {
		playback.current_music = 0;
	}

	if (playback.target_fade == 0) {
		if (playback.fade_out) {
			playback.fade -= (1.0 / playback.fade_out) / 24.0;
		} else {
			playback.fade = 0;
		}
		if (playback.fade <= 0) {
			playback.fade = 0;
			stop_music(playback.paused);
		}
	}

	if (!playback.current_music && !playback.paused) {
		if (playback.next_music) {
			playback.current_music = playback.next_music;
			playback.next_music = 0;
		} else {
			playback.current_music = get_next_music();
		}
		if (playback.current_music) {
			playback.target_fade = 1;
			play_music(false);
		}
	}

	if (playback.current_music || playback.paused) {
		if (*g_music_playing) {
			*g_music_playing = false; // So we don't do retarded lard fade which freezes the game
			helper::stop_bw_music();
		}
	} else if (!*g_music_playing) {
		*g_music_playing = true; // So we actually play the vanilla music
		helper::play_next_bw_music();
	}

	if (playback.target_fade == 1) {
		if (playback.fade_in) {
			playback.fade += (1.0 / playback.fade_in) / 24.0;
		} else {
			playback.fade = 1;
		}
		if (playback.fade >= 1) {
			playback.fade = 1;
		}
	}

	if (playback.volume_time) {
		f32 music_volume_delta = (1.0 / playback.volume_time) / 24.0;
		if (playback.volume < playback.target_volume) {
			playback.volume += music_volume_delta;
			if (playback.volume >= playback.target_volume) {
				playback.volume = playback.target_volume;
			}
		} else {
			playback.volume -= music_volume_delta;
			if (playback.volume <= playback.target_volume) {
				playback.volume = playback.target_volume;
			}
		}
	} else {
		playback.volume = playback.target_volume;
	}

	if (playback.current_music && playback.current_music->ds_buffer) {
		playback.current_music->ds_buffer->SetVolume(get_playback_volume());
	}
}

static bool load_ds_buffer(Sound* sound) {
	if (!direct_sound) {
		return false;
	}

	SFile file = SFile::load(sound->path);
	if (!file.data()) {
		scbw::printf("Error: File %s missing!", sound->path);
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

	if (!SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &sound->ds_buffer, 0))) {
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

Sound* load_sound(char* filename) {
	if (!filename) {
		return 0;
	}
	if (loaded_sounds_count < MaxSound) {
		Sound* sound = &loaded_sounds[loaded_sounds_count++];
		snprintf(sound->path, MaxPath, "%s", filename);

		if (SFile::exists(sound->path)) {
			u32 hash_index = hash_string(filename) % MaxSoundHash;
			sound->hash_next = loaded_sounds_hash_table[hash_index];
			loaded_sounds_hash_table[hash_index] = sound;
			return sound;
		} else {
			loaded_sounds_count--;
			return 0;
		}
	}
	return 0;
}

Sound* get_sound(char* filename) {
	u32 hash_index = hash_string(filename) % MaxSoundHash;
	Sound* sound = loaded_sounds_hash_table[hash_index];
	while (sound && strcmp(sound->path, filename) != 0) {
		sound = sound->hash_next;
	}
	if (!sound) {
		sound = load_sound(filename);
	}
	return sound;
}

void free_sound(Sound* sound) {
	if (sound->ds_buffer) {
		sound->ds_buffer->Release();
		sound->ds_buffer = 0;
	}
}

bool play_sound(Sound* sound, s32 volume, s32 pan, s32 unit_id) {
	if (!sound) {
		return false;
	}
	if (!sound->ds_buffer) {
		if (!load_ds_buffer(sound)) {
			return false;
		}
	}
	if (sound->last_play_time && GetTickCount() - sound->last_play_time < 80) {
		return false;
	}

	DWORD status = 0;
	sound->ds_buffer->GetStatus(&status);
	if (status == DSBSTATUS_PLAYING || unit_id != -1) {
		// Use dup channel
		DupChannel* dup_buffer = 0;
		for (u32 i = 0; i < MaxDupChannels; ++i) {
			if (dup_channels[i].ds_buffer) {
				DWORD dup_status = 0;
				dup_channels[i].ds_buffer->GetStatus(&dup_status);
				if (dup_status != DSBSTATUS_PLAYING) {
					dup_channels[i].ds_buffer->Release();
					dup_channels[i].ds_buffer = 0;
				}
			}
			if (!dup_channels[i].ds_buffer) {
				dup_buffer = &dup_channels[i];
			}
		}

		if (dup_buffer) {
			direct_sound->DuplicateSoundBuffer(sound->ds_buffer, &dup_buffer->ds_buffer);
			if (dup_buffer->ds_buffer) {
				dup_buffer->ds_buffer->SetVolume(volume);
				dup_buffer->ds_buffer->SetPan(pan);
				dup_buffer->ds_buffer->SetCurrentPosition(0);
				dup_buffer->ds_buffer->Play(0, 0, 0);
				dup_buffer->unit_id = unit_id;
				sound->last_play_time = GetTickCount();
				return true;
			}
		}
	} else {
		sound->ds_buffer->SetVolume(volume);
		sound->ds_buffer->SetPan(pan);
		sound->ds_buffer->SetCurrentPosition(0);
		sound->ds_buffer->Play(0, 0, 0);
		sound->last_play_time = GetTickCount();
		return true;
	}
	return false;
}

void play_music(bool resume) {
	if (playback.current_music) {
		if (!playback.current_music->ds_buffer) {
			load_ds_buffer(playback.current_music);
		}
		if (playback.current_music->ds_buffer) {
			playback.current_music->ds_buffer->SetVolume(get_playback_volume());
			playback.current_music->ds_buffer->SetPan(0);
			if (!resume) playback.current_music->ds_buffer->SetCurrentPosition(0);
			playback.current_music->ds_buffer->Play(0, 0, 0);
		}
	}
}

void stop_music(bool pause) {
	if (playback.current_music && playback.current_music->ds_buffer) {
		playback.current_music->ds_buffer->Stop();
	}
	if (!pause) playback.current_music = 0;
}

void shuffle_playlist() {
	for (u32 i = 0; i < current_playlist.size; ++i) {
		Sound* music_a = current_playlist.tracks[i];
		for (u32 j = i + 1; j < current_playlist.size; ++j) {
			if (i == 0 && j == current_playlist.size - 1) {
				// Avoid swapping first and last, so we don't hear the same sound twice
				continue;
			}
			Sound* music_b = current_playlist.tracks[j];
			if (rand() % 2 == 0) {
				Sound temp = *music_a;
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
	if (*use_quiet_sounds) {
		index /= 5;
	}
	return g_big_volume_array[index * g_options_list->sfx / 100];
}

s32 get_sound_volume_at_pos(Point16 pos) {
	s32 index = helper::get_sfx_volume_from_screen_distance(pos.x, pos.y);
	if (*use_quiet_sounds) {
		index /= 5;
	}
	if (index <= 10) {
		return DSBVOLUME_MIN;
	}
	return g_big_volume_array[index * g_options_list->sfx / 100];
}

s32 get_playback_volume() {
	if (playback.fade == 0 || playback.volume == 0) {
		return DSBVOLUME_MIN;
	}

	s32 volume = getMusicVolume();
	volume += (-4000 - volume) * (1 - playback.fade * playback.volume);
	return volume;
}

Sound* get_next_music() {
	if (!current_playlist.size) {
		return 0;
	}
	if (current_playlist.index == 0 && current_playlist.shuffle) {
		shuffle_playlist();
	}
	Sound* music = current_playlist.tracks[current_playlist.index++];
	if (current_playlist.index >= current_playlist.size) {
		current_playlist.index = 0;
	}
	return music;
}