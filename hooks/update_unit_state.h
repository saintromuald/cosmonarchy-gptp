#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {
	void updateUnitEnergy(Unit* unit);													// 004EB4B0
	void update_unit_state(Unit* unit);												// 004EC290
	void updateUnits();																	// 004ED000
	void updateBurrowingCloakingUnits();												// 00492EB0
	void doCloak(Unit* unit, s32 soundId);												// 0049B440
	void progress_unit_dying_frame(Unit* unit);													// 004ECF70

	void injectUpdateUnitState();
}
