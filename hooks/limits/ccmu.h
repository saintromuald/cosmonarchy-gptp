#pragma once
#pragma pack(1)
#include "SCBW\structures.h"
#include "extender.h"
#include <vector>
const int NEW_ARRAY_LENGTH = 1700;

struct CCMUExtender : public Extender {
	virtual std::vector<u32> get_address_list();
	virtual std::vector<u32> get_size_list();
	std::vector<u32> table_start_list();
	std::vector<u32> table_end_list();
	std::vector<u32> sprite_offset_list();
public:
	CUnitLayout units[NEW_ARRAY_LENGTH];
	virtual void extend();
};

namespace hooks {
	void injectLimitHooks();
}

extern CCMUExtender new_unit_table;