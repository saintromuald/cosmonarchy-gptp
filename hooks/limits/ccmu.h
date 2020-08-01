#pragma once
#pragma pack(1)
#include "SCBW\structures.h"
#include "extender.h"
#include <vector>
const int NEW_ARRAY_LENGTH = 1700;

struct CCMUExtender : public Extender {
public:
	CUnitLayout units[UNIT_ARRAY_LENGTH+1];
	virtual std::vector<u32> get_address_list();
	virtual std::vector<u32> get_size_list();
	std::vector<u32> table_start_list();
	std::vector<u32> table_end_list();
	virtual void extend();
};

namespace hooks {
	void injectLimitHooks();
}

extern CCMUExtender new_unit_table;