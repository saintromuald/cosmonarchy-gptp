#include "max_energy.h"
#include <SCBW/api.h>

namespace hooks
{
	// Replaces the Unit::get_max_energy() function. Returns the amount of maximum energy that a unit can have.
	// Note: 1 energy displayed in-game equals 256 energy, and energy cannot be more than 255.
	u16 get_unit_max_energy(Unit* const unit) // 00491870
	{
		u32 energy = 200;
		switch (unit->id)
		{
			case UnitId::ProtossDemiurge:
			{
				energy = 50;
			} break;
			case UnitId::TerranSevenstep:
			{
				energy = 100;
			} break;
			case UnitId::TerranApostle:
			case UnitId::TerranHypnagogue:
			case UnitId::TerranAncile:
			{
				energy = 150;
			} break;
			case UnitId::ProtossMindTyrant:
			{
				energy = 250;
			} break;
		}
		return std::min(energy * 256, (u32)USHRT_MAX);
	}
}