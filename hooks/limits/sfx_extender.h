#pragma once
#include "hooks/limits/extender.h"

const int NEW_MAX_SFX = 1144;

struct SFXExtender : public Extender {
	virtual std::vector<u32> get_address_list();
	virtual std::vector<u32> get_size_list();
public:
	std::vector<u16> addTime;
	std::vector<u8> min_volume;
	std::vector<u8> flags;
	std::vector<u8> priority;
	std::vector<char*> sound_file_ptrs;
	virtual void extend();
};

namespace hooks {
	void sfxExtender();
}


extern SFXExtender new_sfxdata;