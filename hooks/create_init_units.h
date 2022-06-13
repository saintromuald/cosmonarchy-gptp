#pragma once
#include "../SCBW/structures.h"

namespace hooks
{
	void CreateInitialMeleeBuildings(u8 raceId, u32 player);					 // 0049D7C0
	void CreateInitialMeleeUnits();												 // 0049DA40
	void CreateInitialOverlord(u8 player);									 // 0049D6C0

	Unit* CreateUnitXY(u16 player_id, u16 id, u32 x, u32 y, bool prevent_creep); // 004CD360
	Unit* CreatePreplacedUnit(CHKUnit* chkunit);								 // 004CD740

	void injectCreateInitUnitsHooks();
}