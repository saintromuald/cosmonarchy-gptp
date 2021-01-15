#pragma once
#include "../../SCBW/structures/CUnit.h"

namespace hooks {

	void function_0049D660(CUnit* main_building);					//0x0049D660
	bool function_004E8C80(CUnit* unit, CUnit* main_building);		//0x004E8C80
	void secondaryOrd_SpawningLarva(CUnit* unit);					//0x004EA780
	void secondaryOrd_SpreadCreepSpawningLarva(CUnit* unit);		//0x004EA880

	//u8 spreadCreep(u16 unitId, s32 x, s32 y); // 0x00414680
	bool getCreepRegion(Box32* rect, u16 unitId, s32 x, s32 y, bool isComplete); // 0x00413DB0
	bool spreadsCreep(u16 unitId, bool baseRetVal); // 0x00413870
	bool inCreepArea(s32 x, s32 y); // inline function
	bool runCreepRandomizer(u16* tileBackup, u8* creepOverlayVal, u32* retval, u16* tile, u32 arg); // 0x004140A0
//	char applyCreepAtLocationFromUnitType(s16 a1, s32 a2, s32 a3);	// 004148F0

																									//The injector function that should be called in initialize.cpp
	void injectLarvaCreepSpawnHooks();

} //hooks
