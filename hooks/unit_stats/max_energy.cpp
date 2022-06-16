#include "max_energy.h"
#include <SCBW/api.h>

const s32 ENERGY_INGAME_250 = 64000;
const s32 ENERGY_INGAME_200 = 51200;

namespace hooks
{

	/// Replaces the Unit::get_max_energy() function.
	/// Return the amount of maximum energy that a unit can have.
	/// Note: 1 energy displayed in-game equals 256 energy.
	/// Should be equivalent to GetUnitMaxEnergy @ 00491870  
	u16 getUnitMaxEnergyHook(Unit* const unit)
	{

		if (units_dat::base_property[unit->id] & UnitProperty::Hero)
			return ENERGY_INGAME_250;

		return ENERGY_INGAME_200;
	}

} //hooks
