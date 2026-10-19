#pragma once
#include <BW/structures/Unit.h>

// Effective armor: base (or the trigger override) plus bonuses, minus armor rends, never below 0
s32 get_armor(Unit* unit);

// Armor shown next to the units.dat base in the status panel and armor tooltip
// Dffective armor minus the base, or the full effective armor when a trigger overrides the base
s32 get_armor_bonus_display(Unit* unit);

namespace hooks
{
    u8 get_armor_bonus(Unit* unit); // 00453FC0

    void inject_armor_bonus_hooks();
}