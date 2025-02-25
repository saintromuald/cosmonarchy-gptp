// Support for adding/removing psi fields. To use, you will also have to modify the following hooks:
//   hooks/unit_destructor_special.cpp
//   hooks/unit_selection_graphics.cpp

#pragma once
#include "../BW/structures/Unit.h"

namespace hooks {
    // Edit these functions in psi_field.cpp.
    bool is_ready_to_make_psi_field(Unit* unit);
    bool unit_can_show_power_for(Unit* unit, u32 player);

    bool unit_has_power(u32 unit_id, s32 x, s32 y, u8 player);		// 004936B0
    b32 add_psi_field_sprite(Unit* unit);						    // 00493BF0
    void create_pylon_aura();										// 00493640
    void hide_all_psi_fields();								        // 00494100
    void create_all_power_auras();                                  // 00494030
    void orders_init_psi_provider(Unit* unit);                      // 00493F70
    void create_pylon_aura_witness(u8 player);

    // This function must be called once per frame in next_frame().
    // This uses canMakePsiField() and isReadyToMakePsiField() internally.
    void update_psi_field_providers();

    // Call this in initialize.h.
    void inject_psi_field_hooks();
}