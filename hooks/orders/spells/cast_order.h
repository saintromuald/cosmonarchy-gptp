#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_spell(Unit* unit);																				// 00492850
	s32 spell_order(s32 default_error_string, Unit* caster, u16 tech_id, s32 range, u16* energy_cost);			// 004926D0
	bool can_target_spell(u16 tech_id, Unit* target, s16 fow_unit_id, Unit* caster, s16 x, s16 y, u16* error);	// 00492020
	void issue_order_targeting_ground(Unit* unit, u32 id, u16 coord_x, u16 coord_y);							// 00475260
	void issue_tri_target_order(u8 order_1, u8 order_2, u8 order_3);
	void orders_defensive_matrix(Unit* caster);																	// 004550A0

	void inject_cast_order_hooks();
}