#pragma once
#include <SCBW/structures/Unit.h>

namespace hooks {
	void update_unit_energy(Unit* unit);											// 004EB4B0
	void update_unit_state(Unit* unit);												// 004EC290
	void update_units();															// 004ED000
	void update_burrowing_cloaking_units();											// 00492EB0
	void do_cloak(Unit* unit, s32 sound_id);										// 0049B440
	void progress_unit_frame(Unit* unit);											// 004ECF70
	void sprite_to_iscript_loop(Sprite* sprite);									// 00497920
	u8 replace_unit_with_id(Unit* unit, u32 new_id);								// 0049FED0
	u32 update_unit_strength_and_apply_default_orders(Unit* unit);					// 0049FA40

	void inject_update_unit_state_hooks();
}
