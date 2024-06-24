#include "max_energy.h"
#include <BW/api.h>

namespace hooks {
	// Replaces the Unit::get_max_energy() function. Returns the amount of maximum energy that a unit can have.
	// Note: 1 energy displayed in-game equals 256 energy, and energy cannot be more than 255.
	u16 get_unit_max_energy(Unit* const unit) { // 00491870 
		u32 energy = 200;
		switch (unit->id) {
			case UnitId::TerranPhobos: {
				energy = 100;
			} break;
			case UnitId::TerranHypnagogue:
			case UnitId::TerranAncile:
			case UnitId::TerranSeraph:
			case UnitId::TerranLuminary:
			case UnitId::ProtossCantavis:
			case UnitId::ProtossAnalogue: {
				energy = 150;
			} break;
			case UnitId::ProtossMindTyrant:
			case UnitId::Goska: {
				energy = 250;
			} break;
		}
		return std::min(energy * 256, (u32)USHRT_MAX);
	}
}