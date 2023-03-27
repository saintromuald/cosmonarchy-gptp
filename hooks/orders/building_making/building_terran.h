#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_terran_build_self(Unit* building);			// 00467760
	void orders_scv_build_2(Unit* unit);					// 00467A70
	void orders_scv_build(Unit* unit);						// 00467FD0
	char refresh_screen(s32 ecx0);							// 0048DDC0
	void btnsact_place_addon(u16 unit_id, char* buffer);	// 00423D10
	void play_building_complete_sound(Unit* unit);			// 0048F310
	void orders_place_addon(u32 unused, Unit* unit);		// 004E6880
	b32 unit_is_geyser(Unit* unit);							// 004688B0
	b32 is_constructing_addon(Unit* unit);					// 004E66B0

	void inject_building_terran_hook();
}