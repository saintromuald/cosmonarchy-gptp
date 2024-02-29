#pragma once
#include <BW/structures/Unit.h>
#include <BW/structures.h>

namespace hooks {
	void update_unit_energy(Unit* unit);													// 004EB4B0
	void update_unit_state(Unit* unit);														// 004EC290
	void update_units();																	// 004ED000
	void update_burrowing_cloaking_units();													// 00492EB0
	void do_cloak(Unit* unit, s32 sound_id);												// 0049B440
	void progress_unit_frame(Unit* unit);													// 004ECF70
	void sprite_to_iscript_loop(Sprite* sprite);											// 00497920
	u8 replace_unit_with_id(Unit* unit, u32 new_id);										// 0049FED0
	u32 update_unit_strength_and_apply_default_orders(Unit* unit);							// 0049FA40
	u32 initialize_unit_base(Unit* unit, u32 unit_id, u32 pos_x, u32 pos_y, u32 player);	// 004A0320
	b32 set_thingy_visibility_flags(Thingy* thingy);										// 004878F0
	b32 initialize_unit(Unit* unit, s32 pos_y, u16 unit_id, s32 pos_x, u8 player);			// 0049ECF0

	void inject_update_unit_state_hooks();
}
