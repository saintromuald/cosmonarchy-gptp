#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{

	CUnit* findBestNukeLaunchTarget(CUnit* caster, bool isUnderAttack)
	{
		return scbw::UnitFinder::getNearestTarget(caster,
			[&](CUnit* target)
		{

			if ((target->status & (UnitStatus::Cloaked | UnitStatus::RequiresDetection))
				&& !target->isVisibleTo(caster->playerId))
				return false;

			if (!caster->isTargetEnemy(target))
				return false;

			const s32 totalEnemyClumpValue = getTotalEnemyNukeValueInArea(target->getX(), target->getY(), 192, caster);
			if (totalEnemyClumpValue >= 800)
				return true;

			return false;

		});
	}

} //AI
