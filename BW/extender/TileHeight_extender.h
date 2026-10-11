#pragma once
#include <BW/api.h>

inline s32 g_base_height;
inline s8 g_height_table[256*256];

namespace bw {

void init_weapon_tile_heights();
bool tile_is_equal_or_lower(s32 height, u32 offset);

}