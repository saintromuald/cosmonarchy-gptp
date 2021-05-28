#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* findBestLockdownTarget(Unit* caster, bool isUnderAttack)
	{
		if (caster->vulture.spiderMineCount == 0)
		{
			return false;
		}

		s32 bounds;
		if (isUnderAttack)
		{
			bounds = 32 * 9;
		}
		else if (isUmsMode(caster->playerId))
		{
			bounds = 32 * 64;
		}
		else
		{
			bounds = 32 * 32;
		}

		if (caster->vulture.spiderMineCount == 0)
		{
			return nullptr;
		}

		return scbw::UnitFinder::getNearestTarget(
			caster->getX() - bounds,
			caster->getY() - bounds,
			caster->getX() + bounds,
			caster->getY() + bounds,
			caster,
			[&](Unit* target)
		{

			if (!isTargetWorthHitting(target, caster))
			{
				return false;
			}

			if (!scbw::canWeaponTargetUnit(WeaponId::Lockdown, target, caster))
			{
				return false;
			}

			if (target->isFrozen())
			{
				return false;
			}

			if (isTargetAttackingAlly(target, caster))
			{
				return true;
			}

			return false;
		});
	}
}
