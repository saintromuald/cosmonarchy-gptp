#pragma once
#include <SCBW/structures.h>

namespace hooks
{
	void apply_upgrade_flags_to_new_unit(Unit* unit);						// 00454370
	void apply_upgrade_flags_to_existing_units(Unit* unit, u8 upgrade_id);	// 00454540

	void inject_apply_upgrade_flags_hooks();
}