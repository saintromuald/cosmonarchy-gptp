#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_ZergBirth(Unit* unit);													// 0045DD60
	void orders_Morph1(Unit* unit);														// 0045DEA0
	bool unitCanMorphHook(Unit* unit, u16 morphUnitId);
	bool isRallyableEggUnitHook(u16 unitId);
	u16 getUnitMorphEggTypeHook(u16 unitId, Unit* unit, u16 resultId);
	u16 getCancelMorphRevertTypeHook(Unit* eggUnit);
	s16 getUnitVerticalOffsetOnBirth(Unit* unit);
	Unit* beginTrain(Unit* unit, u16 unit_id, bool notify_error);

	void injectUnitMorphHooks();
}