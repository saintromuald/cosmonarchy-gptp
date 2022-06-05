#include "sight_range.h"
#include <SCBW/api.h>

const s32 DEFAULT_SIGHT_RANGE = 4;
const s32 BLIND_SIGHT_RANGE = 2;
const s32 MAX_SIGHT_RANGE = 11;

namespace hooks
{

	/// Returns the modified sight distance of the unit, measured in matrices.
	/// StarCraft passes 1 for isForSpellCasting when using Feedback, Mind Control,
	/// and Hallucination (but not when launching Nukes).
	/// Note: sight ranges cannot exceed 11, unless extended.
	/// Equivalent to getUpgradedSightRange @ 004E5B40  
	u32 getSightRangeHook(Unit* unit, bool isForSpellCasting)
	{
		u32 sightRange;

		//Check if the unit is a constructing building (exclude remorphing buildings)
		if (unit->status & UnitStatus::GroundedBuilding &&
			!(unit->status & UnitStatus::Completed) &&
			!unit->isRemorphingBuilding()
			)
			sightRange = DEFAULT_SIGHT_RANGE;
		else
			//Check if the unit is blinded (don't bother if this is for spellcasting)
			if (!isForSpellCasting && unit->is_blind)
				sightRange = BLIND_SIGHT_RANGE;
			else
				sightRange = units_dat::SightRange[unit->id];

		return sightRange;
	}

} //hooks
