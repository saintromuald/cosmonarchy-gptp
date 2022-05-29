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

		return scbw::UnitFinder::getNearestTarget(
			caster->getX() - bounds,
			caster->getY() - bounds,
			caster->getX() + bounds,
			caster->getY() + bounds,
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
			if (target->getMaxHpInGame() < 100 || target->getCurrentHpInGame() > 75)
				return false;
			if (target->status & UnitStatus::Invincible)
				return false;
			if (scbw::getTechUseErrorMessage((Unit*)target, caster->player, TechId::DefensiveMatrix) != 0)
				return false;
			return true;
		});
	}

} //AI
