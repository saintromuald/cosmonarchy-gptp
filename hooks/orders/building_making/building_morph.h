#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	bool isMorphing(Unit* building);							// 0x0045CD00
	void orders_ZergBuildSelf(Unit* building);					// 0x0045D500
	void ZergPlaceBuilding(Unit* drone);						// 0x0045DA40	
	//from unhooked
	bool isMorphedBuildingHook(u16 unitId);
	s32 getMorphBuildingTypeCountHook(const Unit* unit, u16 unitId, bool ignoreIncomplete);
	void injectBuildingMorphHooks();
	void SetSpecialState1(Sprite* sprite);
}