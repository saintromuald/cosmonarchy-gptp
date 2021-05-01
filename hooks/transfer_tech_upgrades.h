//Header file for the Transfer Tech & Upgrades hook module.
#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

void transferUnitTechToPlayerHook(CUnit* sourceUnit, uint8 targetPlayerId, uint32 researchTechFunc = 0x0047B3C0);	//0x004E2C00

//Default value for researchUpgradesFunc is set as increaseUpgradeLevel @ 0047B340
//The other possible value is sub_47B940 @ 0047B940 (called by GiveAllUnitPropertiesToPlayerAndMakeIdle)
void transferUnitUpgradesToPlayerHook(CUnit* sourceUnit, uint8 targetPlayerId, uint32 researchUpgradesFunc = 0x0047B340);		//0x004E2B50

void injectTransferTechAndUpgradesHooks();

} //hooks
