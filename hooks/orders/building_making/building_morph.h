#pragma once
#include "../../../SCBW/structures/CUnit.h"

namespace hooks
{
	bool isMorphing(CUnit* building);							// 0x0045CD00
	void orders_ZergBuildSelf(CUnit* building);					// 0x0045D500
	void ZergPlaceBuilding(CUnit* drone);						// 0x0045DA40	
	//from unhooked
	bool isMorphedBuildingHook(u16 unitId);
	s32 getMorphBuildingTypeCountHook(const CUnit* unit, u16 unitId, bool ignoreIncomplete);
	void injectBuildingMorphHooks();
	void SetSpecialState1(CSprite* sprite);
}