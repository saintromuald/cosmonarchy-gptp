#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{
	void unit_attacks_from_inside_bunker(Unit* unit);
	void create_bunker_attack_overlay(Unit* unit);

	void inject_bunker_hooks();
}