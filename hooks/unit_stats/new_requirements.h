#pragma once
#include <BW/api.h>

enum class CheckCreator {
    None      = 0, // Ignores direct creator
    Factories = 1, // Ignores non-factory creators (e.g. drone, larva, cesient)
    All       = 2, // Check all direct creators
};

namespace hooks {

s32 create_unit_id_allowed(u16 unit_id, s32 player_id, CheckCreator check_creator);
s32 create_unit_allowed_partial(u16 unit_id, Unit* unit, s32 player);
bool can_set_rally_opcode(Unit* unit);

u16 addon_datreq_fix(u32 dat_req_offset, u16 unit_id);

void inject_new_requirements_hooks();
}