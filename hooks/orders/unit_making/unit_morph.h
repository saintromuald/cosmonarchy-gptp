#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_zerg_birth(Unit* unit);													// 0045DD60
	void orders_morph_1(Unit* unit);														// 0045DEA0
	bool unit_can_morph(Unit* unit, u16 morphUnitId);
	bool is_rallyable_egg_unit(u16 unitId);
	u16 get_unit_morph_egg_id(u16 unitId, Unit* unit, u16 resultId);
	u16 get_cancel_morph_revert_id(Unit* eggUnit);
	s16 get_unit_vertical_offset_on_birth(Unit* unit);
	Unit* begin_train(Unit* unit, u16 unit_id, bool notify_error);

	void injectUnitMorphHooks();
}