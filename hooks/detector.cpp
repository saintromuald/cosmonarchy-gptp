#include "detector.h"
#include "../SCBW/scbwdata.h"
#include "../SCBW/enumerations.h"

namespace hooks
{
	/// Determines whether this unit can detect cloaked/burrowed units.
	/// This affects Unit::can_detect().
	/// This overrides the EXE edit settings for Detectors in FireGraft.
	bool unit_can_detect(Unit* unit)
	{
		return  (units_dat::base_property[unit->id] & UnitProperty::Detector
			&&   unit->status & UnitStatus::Completed
			&& !(unit->status & UnitStatus::Frozen)
			&&   unit->lockdown_timer == 0
			&&   unit->stasis_timer == 0
			&&   unit->maelstrom_timer == 0
			&&   unit->is_blind == 0);
	}

	// Check related to the question of the @p detector being able to see the @p param->target (assuming target is cloaked).
	// Used in a IterateUnitsAtLocation function (thus the special structure)
	// Part of bigger logic, probably should not be touched
	void get_cloaked_target_visibility(Unit* detector, DetectorCheckParam* param)
	{
		Unit* target = param->target;
		if (detector->can_detect() && detector != target)
		{
			u8 visibility_flags = (1 << detector->player);
			// Only continue if the target sprite is visible (else it's not a cloaking issue that hide the unit)
			if (target->sprite->visibility_status & visibility_flags)
			{
				u32 detection_range;
				if (detector->status & UnitStatus::GroundedBuilding)
				{
					detection_range = 224; //0xE0, 32 * 7
				}
				else
				{
					detection_range = 32 * detector->get_sight_range(false);
				}
				if (detector->is_target_within_min_range(target, detection_range))
				{
					//update visionFlags for current target eval
					param->visionFlags |=
						detector->parasite_flags					// detector's parasites player owner flags (8 bits)
					|	player_vision->flags[detector->player]		// default visibility (32 bits)
					|	visibility_flags;							// detector's player flag (8 bits)
				}
			}
		}
	}
}