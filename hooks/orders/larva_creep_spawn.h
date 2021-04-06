#pragma once
#include "../../SCBW/structures/CUnit.h"

namespace hooks {

	void function_0049D660(CUnit* main_building);					//0x0049D660
	bool function_004E8C80(CUnit* unit, CUnit* main_building);		//0x004E8C80
	void secondaryOrd_SpawningLarva(CUnit* unit);					//0x004EA780
	void secondaryOrd_SpreadCreepSpawningLarva(CUnit* unit);		//0x004EA880

	//uint8 spreadCreep(uint16 unitId, int32 x, int32 y); // 0x00414680
	bool getCreepRegion(Box32* rect, uint16 unitId, int32 x, int32 y, bool isComplete); // 0x00413DB0
	bool spreadsCreep(uint16 unitId, bool baseRetVal); // 0x00413870
	bool inCreepArea(int32 x, int32 y); // inline function
	bool runCreepRandomizer(uint16* tileBackup, uint8* creepOverlayVal, uint32* retval, uint16* tile, uint32 arg); // 0x004140A0
//	char applyCreepAtLocationFromUnitType(int16 a1, int32 a2, int32 a3);	// 004148F0

																									//The injector function that should be called in initialize.cpp
	void injectLarvaCreepSpawnHooks();

} //hooks
