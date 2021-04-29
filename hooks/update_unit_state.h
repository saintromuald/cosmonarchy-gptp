#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {

void updateUnitEnergy(CUnit* unit);		// 004EB4B0
void updateUnitStateHook(CUnit* unit);	// 004EC290
void incrementUnitScoresEx(int count, CUnit* unit, int updateMorphMergeUnitScore); // 00488D50
void updateUnits();						// 004ED000
void updateBurrowingCloakingUnits();	// 00492EB0

void injectUpdateUnitState();

} //hooks
