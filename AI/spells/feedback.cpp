#include "spells.h"
#include <AI/ai_common.h>

// V241 for VS2008

namespace AI
{
	Unit* findBestFeedbackTarget(Unit* caster, bool isUnderAttack)
	{

		s32 bounds;

		if (isUnderAttack)
			bounds = 32 * 9;
		else if (isUmsMode(caster->player))
			bounds = 32 * 64;
		else
			bounds = 32 * 32;

		return scbw::UnitFinder::get_nearest_target(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
			caster,
			[&](Unit* target)
		{
			if (!is_target_worth_hitting(target, caster))
				return false;

			if (!target->is_valid_caster())
				return false;

			if (target->status & UnitStatus::GroundedBuilding)
				return false;

			if (units_dat::BaseProperty[target->id] & UnitProperty::Hero)
				return false;

			if (target->energy / 256u >= target->get_current_life_ingame())
				return true;

			return false;
		});
	}

} //AI
