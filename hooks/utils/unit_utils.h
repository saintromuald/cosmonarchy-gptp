#pragma once
#include "../../BW/structures.h"

namespace hooks
{
bool unit_is_standard_and_movable(Unit* unit);			// 0047B770
Unit* get_active_player_first_selection();				// 0049A850

void inject_unit_utils_hooks();
}