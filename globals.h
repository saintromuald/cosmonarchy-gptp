#pragma once
#pragma pack(1)
#include <vector>
#include <sstream>
#include <ostream>
#include <istream>
#include "SCBW\api.h"

class globals;

struct SaveState {
	u32 len;
	const char* ptr;

	SaveState() {
		ptr = 0;
		len = 0;
	}
};

class globals {
	std::string output;
public:
	u32 extended_unit_table;
	u32 extended_sfx;
	bool tbl_extender;
	SaveState aise_save();
	void aise_load(const char* ptr, u32 len);
	globals() {
		extended_unit_table = 0;
		extended_sfx = 0;
		tbl_extender = false;
	}
};

void injectGlobalsHooks();

extern globals Globals;