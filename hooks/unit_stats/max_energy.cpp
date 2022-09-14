#include "max_energy.h"
#include <SCBW/api.h>

namespace hooks {

	/// Replaces the Unit::get_max_energy() function.
	/// Return the amount of maximum energy that a unit can have.
	/// Note: 1 energy displayed in-game equals 256 energy.
	/// Should be equivalent to GetUnitMaxEnergy @ 00491870  
	u16 getUnitMaxEnergyHook(Unit* const unit) {
		u32 energy = units_dat::base_property[unit->id] & UnitProperty::Hero ? 250 : 200;

		switch (unit->id) {
		//	case UnitId::TerranAncile: {
		//		energy = 200;
		//	} break;
		}

		return std::min(energy * 256, (u32)USHRT_MAX);
	}
}
