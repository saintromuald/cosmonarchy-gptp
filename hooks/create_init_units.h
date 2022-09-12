#pragma once
#include "../SCBW/structures.h"

namespace hooks
{
	void create_initial_melee_buildings(u8 raceId, u32 player);					 // 0049D7C0
	void create_initial_melee_units();												 // 0049DA40
	void create_initial_ovileth(u8 player);									 // 0049D6C0

	Unit* create_unit_xy(u16 player_id, u16 id, u32 x, u32 y, bool prevent_creep); // 004CD360
	Unit* create_preplaced_unit(CHKUnit* chkunit);								 // 004CD740

	void inject_create_initial_units_hooks();
}