#pragma once
#include "../BW/structures.h"

namespace hooks {
void create_initial_melee_buildings(u8 race_id, u32 player);					// 0049D7C0
void create_initial_melee_units();												// 0049DA40
void create_initial_ovileth(u8 player);											// 0049D6C0

Unit* create_unit_xy(u16 player_id, u16 id, u32 x, u32 y, bool prevent_creep);	// 004CD360
Unit* create_preplaced_unit(ChkUnit* chkunit);									// 004CD740
u32 create_initial_team_melee_units(s32 player);								// 0049D8E0
bool chkunit_invalid_for_melee(ChkUnit* chkunit);								// 004CBE20
u32 get_player_force(u8 player);												// 004A8B90

void inject_create_initial_units_hooks();
}