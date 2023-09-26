#include "sight_range.h"
#include <SCBW/api.h>

const s32 DEFAULT_SIGHT_RANGE = 4;
const s32 BLIND_SIGHT_RANGE = 2;
const s32 MAX_SIGHT_RANGE = 11;

namespace hooks
{
	/// Returns the modified sight distance of the unit, measured in matrices.
	/// StarCraft passes 1 for is_for_spellcasting when using Feedback, Mind Control, and Hallucination (but not when launching Nukes).
	/// Note: sight ranges cannot exceed 11, unless extended.
	/// Equivalent to getUpgradedSightRange @ 004E5B40  
	u32 get_sight_range(Unit* unit, bool is_for_spellcasting) {
		// Check if the unit is a constructing building (exclude remorphing buildings)
		if  (unit->status & UnitStatus::GroundedBuilding
		&& !(unit->status & UnitStatus::Completed)
		&&  !unit->is_remorphing_building()) {
			return DEFAULT_SIGHT_RANGE;
		}
		if (!is_for_spellcasting && unit->is_blind) { // Check if the unit is blinded (don't bother if this is for spellcasting)
			return BLIND_SIGHT_RANGE;
		}
		u32 sight_range = units_dat::sight_range[unit->id];
		if (unit->status & UnitStatus::Burrowed && !scbw::unit_is_trap(unit)) {
			sight_range /= 2;
		}
		return sight_range;
	}
}