// Support for adding/removing psi fields. To use, you will also have to modify the following hooks:
//   hooks/unit_destructor_special.cpp
//   hooks/unit_selection_graphics.cpp

#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {
	// Edit these functions in psi_field.cpp.
	bool is_ready_to_make_psi_field(Unit* unit);
	bool can_make_psi_field(u16 unit_id);
	bool unit_can_show_power_for(Unit* unit, u32 player);

	bool unit_has_power(u32 unit_id, s32 x, s32 y, u8 player);		// 004936B0
	b32 show_power_sprite(Unit* unit);								// 00493BF0
	void create_pylon_aura();										// 00493640

	// This function must be called once per frame in next_frame().
	// This uses canMakePsiField() and isReadyToMakePsiField() internally.
	void update_psi_field_providers();

	// Call this in initialize.h.
	void inject_psi_field_hooks();
}