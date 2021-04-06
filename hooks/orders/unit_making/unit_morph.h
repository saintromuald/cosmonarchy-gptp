#pragma once
#include "../../../SCBW/structures/CUnit.h"

namespace hooks {

	void orders_ZergBirth(CUnit* unit);	//0x0045DD60
	void orders_Morph1(CUnit* unit);	//0x0045DEA0
	bool unitCanMorphHook(CUnit* unit, uint16 morphUnitId);
	bool isRallyableEggUnitHook(uint16 unitId);
	uint16 getUnitMorphEggTypeHook(uint16 unitId, CUnit* unit, uint16 resultId);
	uint16 getCancelMorphRevertTypeHook(CUnit* eggUnit);
	int16 getUnitVerticalOffsetOnBirth(CUnit* unit);
	bool hasSuppliesForUnitHook(uint8 playerId, uint16 unitId, bool canShowErrorMessage);
	void injectUnitMorphHooks();
	uint32 remove_worker_or_building_ai(CUnit* unit, uint8 assert_is_building);
	CUnit* beginTrain(CUnit* unit, uint16 unit_id, bool notify_error);
} //hooks