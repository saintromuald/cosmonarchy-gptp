#pragma once
#include <SCBW/structures/CUnit.h>

namespace hooks {
	void updateUnitEnergy(CUnit* unit);			// 004EB4B0
	void updateUnitStateHook(CUnit* unit);		// 004EC290
	void incrementUnitScoresEx(s32 count, CUnit* unit, s32 updateMorphMergeUnitScore); // 00488D50
	void updateUnits();							// 004ED000
	void updateBurrowingCloakingUnits();		// 00492EB0
	void doCloak(CUnit* unit, s32 soundId);	// 0049B440

	void injectUpdateUnitState();
}
