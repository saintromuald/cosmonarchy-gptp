#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{
	void function_0049D660(Unit* main_building);													// 0049D660
	bool function_004E8C80(Unit* unit, Unit* main_building);										// 004E8C80
	void secondaryOrd_SpawningLarva(Unit* unit);													// 004EA780
	void secondaryOrd_SpreadCreepSpawningLarva(Unit* unit);											// 004EA880

//	u8 spread_creep(u16 unit_id, s32 x, s32 y);														// 00414680
	bool getCreepRegion(Box32* rect, u16 unit_id, s32 x, s32 y, bool isComplete);					// 00413DB0
	bool spreads_creep(u16 unit_id, bool baseRetVal);													// 00413870
	bool inCreepArea(s32 x, s32 y);																	// inline function
	bool runCreepRandomizer(u16* tileBackup, u8* creepOverlayVal, u32* retval, u16* tile, u32 arg); // 004140A0
//	char applyCreepAtLocationFromUnitType(s16 a1, s32 a2, s32 a3);									// 004148F0
	u32 BTNSCOND_ZergBuildingRally(u32 unused, u32 player, Unit* unit);								// 00429740
	s32 UpdateCreepDisappearanceUnit(Unit* unit);													// 0047DD60

	void injectLarvaCreepSpawnHooks();
}
