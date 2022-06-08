#include "detector.h"
#include "../SCBW/scbwdata.h"
#include "../SCBW/enumerations.h"

namespace hooks
{

	/// Determines whether this unit can detect cloaked/burrowed units.
	/// This affects Unit::canDetect().
	/// This overrides the EXE edit settings for Detectors in FireGraft.
	bool unitCanDetectHook(Unit* unit)
	{
		return (
			units_dat::BaseProperty[unit->id] & UnitProperty::Detector &&
			unit->status & UnitStatus::Completed &&
			!(unit->status & UnitStatus::Frozen) &&
			unit->lockdown_timer == 0 &&
			unit->stasis_timer == 0 &&
			unit->maelstrom_timer == 0 &&
			unit->is_blind == 0
			);
	}

	//Check related to the question of the @p detector being able to see the @p param->target 
	//(assuming target is cloaked).
	//Used in a IterateUnitsAtLocation function (thus the special structure)
	//Part of a bigger logic, probably should not be touched
	void getCloakedTargetVisibility(Unit* detector, DetectorCheckParam* param)
	{
		Unit* target = param->target;
		if (detector->canDetect() &&								// detector must be in a state to detect
			detector != target										// detector shouldn't try to detect itself
			)
		{

			u8 visibilityFlags = (1 << detector->player);
			//only continue if the target sprite is visible (else it's
			//not a cloaking issue that hide the unit)
			if (target->sprite->visibilityFlags & visibilityFlags)
			{
				u32 detectionRange;
				if (detector->status & UnitStatus::GroundedBuilding)
				{
					detectionRange = 224; //0xE0, 32 * 7
				}
				else
				{
					detectionRange = 32 * detector->get_sight_range(false);
				}
				if (detector->is_target_within_min_range(target, detectionRange))
				{
					//update visionFlags for current target eval
					param->visionFlags |=
						detector->parasite_flags |					// detector's parasites player owner flags (8 bits)
						player_vision->flags[detector->player] |	// default visibility (32 bits)
						visibilityFlags								// detector's player flag (8 bits)
						;
				}
			}
		}
	}

} //hooks
