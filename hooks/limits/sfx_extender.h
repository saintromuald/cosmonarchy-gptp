#pragma once
#include "hooks/limits/extender.h"

struct SoundData {
	u32 wav_data_size;
	u32 tick;
	u32 duration_msecs;
	void* buffer;
	SoundData() {
		wav_data_size = 0;
		tick = 0;
		duration_msecs = 0;
		buffer = NULL;
	}
};

struct SFXExtender : public Extender {
	virtual std::vector<u32> get_address_list();
	virtual std::vector<u32> get_size_list();
	

public:
	std::vector<u16> race;
	std::vector<u8> min_volume;
	std::vector<u8> flags;
	std::vector<u8> priority;
	std::vector<char*> sound_file_ptrs;
	std::vector<SoundData> unk;
	SoundData* ptr;
	std::vector<u32> get_flags_list();
	std::vector<u32> get_race_list();
	virtual void extend();
};



namespace hooks {
	void sfxExtender();
}


extern SFXExtender new_sfxdata;