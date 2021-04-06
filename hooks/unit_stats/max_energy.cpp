#include "max_energy.h"
#include <SCBW/api.h>

const int ENERGY_INGAME_250 = 64000;
const int ENERGY_INGAME_200 = 51200;

namespace hooks {

	/// Replaces the CUnit::getMaxEnergy() function.
	/// Return the amount of maximum energy that a unit can have.
	/// Note: 1 energy displayed in-game equals 256 energy.
	/// Should be equivalent to GetUnitMaxEnergy @ 00491870  
	uint16 getUnitMaxEnergyHook(CUnit* const unit) {

		if (units_dat::BaseProperty[unit->id] & UnitProperty::Hero)
			return ENERGY_INGAME_250;

		return ENERGY_INGAME_200;
	}

} //hooks
