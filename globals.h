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
	int example_placeholder;
	int example_test[5];
	SaveState aise_save();
	void aise_load(const char* ptr, u32 len);
	globals() {
		example_placeholder = 3;
	}
};

void injectGlobalsHooks();

extern globals Globals;