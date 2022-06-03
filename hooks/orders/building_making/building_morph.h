#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	bool is_morphing(Unit* building);							// 0045CD00
	void orders_zerg_build_self(Unit* building);				// 0045D500
	void zerg_place_building(Unit* drone);						// 0045DA40	
	
	// from unhooked
	bool is_morphed_building(u16 unit_id);
	s32 get_morph_building_id_count(const Unit* unit, u16 unit_id, bool ignoreIncomplete);
	void set_special_state_1(Sprite* sprite);

	void injectBuildingMorphHooks();
}