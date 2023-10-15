#pragma once

void augur_sacrifice(Unit* unit, s32 radius = 6 * 24);
void unit_on_death_behaviors(Unit* unit);

namespace hooks
{
	void inject_unit_destructor_special_hooks();
}