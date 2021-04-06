#pragma once
#include "../../../SCBW/structures/CUnit.h"

namespace hooks {

	bool isMorphing(CUnit* building);			//0x0045CD00
	void orders_ZergBuildSelf(CUnit* building);	//0x0045D500
	void ZergPlaceBuilding(CUnit* drone);		//0x0045DA40	
	//from unhooked
	bool isMorphedBuildingHook(uint16 unitId);
	int getMorphBuildingTypeCountHook(const CUnit* unit, uint16 unitId, bool ignoreIncomplete);
	void injectBuildingMorphHooks();
	void SetSpecialState1(CSprite* sprite);
} //hooks
