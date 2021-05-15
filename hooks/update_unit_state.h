#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {
	void updateUnitEnergy(Unit* unit);			// 004EB4B0
	void updateUnitStateHook(Unit* unit);		// 004EC290
	void incrementUnitScoresEx(s32 count, Unit* unit, s32 updateMorphMergeUnitScore); // 00488D50
	void updateUnits();							// 004ED000
	void updateBurrowingCloakingUnits();		// 00492EB0
	void doCloak(Unit* unit, s32 soundId);		// 0049B440

	void injectUpdateUnitState();
}
