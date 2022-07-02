#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{
	void unit_attacks_from_inside_bunker(Unit* unit);			// 004790A0
	void create_bunker_attack_overlay(Unit* unit);				// 00477FD0
	u32 attack_apply_cooldown(Unit* unit, u32 holdpos_status);	// 00478B40

	void inject_bunker_hooks();
}