#pragma once
#include "../../../SCBW/structures/CUnit.h"

namespace hooks {

	void orders_ZergBirth(CUnit* unit);	//0x0045DD60
	void orders_Morph1(CUnit* unit);	//0x0045DEA0
	bool unitCanMorphHook(CUnit* unit, u16 morphUnitId);
	bool isRallyableEggUnitHook(u16 unitId);
	u16 getUnitMorphEggTypeHook(u16 unitId, CUnit* unit, u16 resultId);
	u16 getCancelMorphRevertTypeHook(CUnit* eggUnit);
	s16 getUnitVerticalOffsetOnBirth(CUnit* unit);
	bool hasSuppliesForUnitHook(u8 playerId, u16 unitId, bool canShowErrorMessage);
	void injectUnitMorphHooks();
	u32 remove_worker_or_building_ai(CUnit* unit, u8 assert_is_building);
	CUnit* beginTrain(CUnit* unit, u16 unit_id, bool notify_error);
} //hooks