#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {
	void update_unit_energy(Unit* unit);											// 004EB4B0
	void update_unit_state(Unit* unit);												// 004EC290
	void update_units();															// 004ED000
	void update_burrowing_cloaking_units();											// 00492EB0
	void do_cloak(Unit* unit, s32 soundId);											// 0049B440
	void progress_unit_frame(Unit* unit);											// 004ECF70

	void injectUpdateUnitState();
}
