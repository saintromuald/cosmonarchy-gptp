#include "spells.h"
#include <AI/ai_common.h>

// V241 for VS2008

namespace AI
{

	Unit* findBestDefensiveMatrixTarget(Unit* caster, bool isUnderAttack)
	{

		s32 bounds;

		if (isUnderAttack)
			bounds = 32 * 9;
		else
			bounds = 32 * 64;

		return scbw::UnitFinder::get_nearest_target(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
			caster,
			[&](Unit* target)
		{
			if (target->defensiveMatrixHp)
				return false;
			if (caster->isTargetEnemy(target))
				return false;
			if (!target->orderTarget.unit)  //Ignore idle units?
				return false;
			if (target->status & UnitStatus::RequiresDetection)
				return false;
			if (target->get_max_hp_ingame() < 100 || target->get_current_hp_ingame() > 75)
				return false;
			if (target->status & UnitStatus::Invincible)
				return false;
			if (scbw::get_tech_user_error_message((Unit*)target, caster->player, TechId::DefensiveMatrix) != 0)
				return false;
			return true;
		});
	}

} //AI
