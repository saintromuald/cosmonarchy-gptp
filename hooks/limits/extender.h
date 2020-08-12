#pragma once
#include <vector>
#include "SCBW\api.h"
#include "hook_tools.h"

//
const int SFX_Entries = 5;
//
const int SFX_Default_Length = 1144;

class Extender {
	virtual std::vector<u32> get_address_list() = 0;
	virtual std::vector<u32> get_size_list() = 0;
public:
	virtual void extend() = 0;
	void uplink(std::vector<u32> address_list, u32 address);
	int get_file_entry_count(const char* name, int entries);
	
//	identifier
//	get
//  parse entry value
//  parse - define here
//	parse-set (?)
//	file entry count
};

namespace hooks {
	void injectLimitHooks();
}