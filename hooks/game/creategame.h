#pragma once
#include "BW/structures.h"

namespace hooks {
	void generate_strength_table();							// 004313C0
	u32 filter_unit_strength(u32 unit_id, u32 strength);	// 00431150

	void inject_creategame_hooks();
}