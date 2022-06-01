// Support for adding/removing psi fields. To use, you will also have to modify the following hooks:
//   hooks/unit_destructor_special.cpp
//   hooks/unit_selection_graphics.cpp

// NOT FULLY REVALIDATED BY UNDEADSTAR

#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {

	// Edit these functions in psi_field.cpp.
	bool is_ready_to_make_psi_field(Unit* unit);
	bool can_make_psi_field(u16 unitId);
	bool unit_can_show_power_for(Unit* unit, u32 player);

	bool unit_has_power(u32 unit_id, s32 x, s32 y, u32 player);
	b32 show_power_sprite(Unit* unit);
	void create_pylon_aura();

	// This function must be called once per frame in next_frame().
	// This uses canMakePsiField() and isReadyToMakePsiField() internally.
	void update_psi_field_providers();

	// Call this in initialize.h.
	void inject_psi_field_hooks();

}