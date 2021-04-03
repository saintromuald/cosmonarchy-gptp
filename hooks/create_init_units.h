#pragma once
#include "../SCBW/structures/CUnit.h"

namespace hooks {

void CreateInitialMeleeBuildings(uint8 raceId, uint32 playerId);	//0049D7C0
void CreateInitialMeleeUnits();								//0049DA40
void CreateInitialOverlord(uint8 playerId);					//0049D6C0

void injectCreateInitUnitsHooks();

} //hooks