#pragma once
#include <SCBW/structures.h>

namespace hooks
{
	void applyUpgradeFlagsToNewUnitHook(Unit* unit);
	void applyUpgradeFlagsToExistingUnitsHook(Unit* unit, u8 upgradeId);

	// Applies @upgradeId to all units that are owned by the owner of @p unit.
	void applyUpgradeFlagsToExistingUnits(Unit* unit, u8 upgradeId);

	void inject_apply_upgrade_flags_hooks();
}
