#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{

	Unit* findBestNukeLaunchTarget(Unit* caster, bool isUnderAttack)
	{
		return scbw::UnitFinder::get_nearest_target(caster,
			[&](Unit* target)
		{

			if ((target->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection))
				&& !target->is_visible_to(caster->player))
				return false;

			if (!caster->is_target_enemy(target))
				return false;

			const s32 totalEnemyClumpValue = getTotalEnemyNukeValueInArea(target->get_x(), target->get_y(), 192, caster);
			if (totalEnemyClumpValue >= 800)
				return true;

			return false;

		});
	}

} //AI
