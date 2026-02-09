#include "sight_range.h"
#include <BW/api.h>

namespace hooks {
	/// Returns the modified sight distance of the unit, measured in matrices.
	/// StarCraft passes 1 for is_for_spellcasting when using Feedback, Mind Control, and Hallucination (but not when launching Nukes).
	/// Note: sight ranges cannot exceed 11, unless extended.
	/// Equivalent to getUpgradedSightRange @ 004E5B40  
	u32 get_sight_range(Unit* unit, bool is_for_spellcasting) {
		// Check if the unit is a constructing building (exclude remorphing buildings)
		if  (unit->status & UnitStatus::GroundedBuilding && !(unit->status & UnitStatus::Completed) && !unit->is_remorphing_building()) {
			return DEFAULT_SIGHT_RANGE;
		}
		if (!is_for_spellcasting && unit->is_blind) { // Check if the unit is blinded (don't bother if this is for spellcasting)
			return BLIND_SIGHT_RANGE;
		}
		fp32 sight_range{units_dat::sight_range[unit->id]};
		if (bw::get_game_kind() == GameKind::DarkAges) {
			sight_range -= bw::calculate_dark_ages_penalty(sight_range);
		}
		if (unit->is_burrowed() && !unit_id_is_trap(unit->id)) {
			sight_range /= 2;
		}

		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueInfested] && bw::get_game_kind() != GameKind::Embraced) {
			sight_range /= 1.66fp;
		}
		return static_cast<u32>(sight_range);
	}
}