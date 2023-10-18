#pragma once
#include <SCBW/api.h>

extern s32 base_height;
extern s8 xHeight_table[];

namespace scbw {
	void weapon_tile_heights();
	bool tile_is_equal_or_lower(s32 height, u32 offset);
}