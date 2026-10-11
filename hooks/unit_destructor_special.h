#pragma once
#include "../BW/structures.h"

void augur_sacrifice(Unit* unit, s32 radius = 6 * 24);
void unit_on_death_behaviors(Unit* unit);
Thingy* iscript_create_sprite(Image* image, u16 id, u32 x, u32 y, s8 elevation); // 004D7120

namespace hooks {
void inject_unit_destructor_special_hooks();
}