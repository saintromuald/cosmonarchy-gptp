#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{

	CUnit* findBestStasisFieldTarget(CUnit* caster, bool isUnderAttack)
	{
		s32 bounds;
		if (isUnderAttack)
			bounds = 32 * 9;
		else if (isUmsMode(caster->playerId))
			bounds = 32 * 64;
		else
			bounds = 32 * 32;

		return scbw::UnitFinder::getNearestTarget(
			caster->getX() - bounds,
			caster->getY() - bounds,
			caster->getX() + bounds,
			caster->getY() + bounds,
			caster,
			[&](CUnit* target)
		{

			if (!isTargetWorthHitting(target, caster))
				return false;

			if (!scbw::canWeaponTargetUnit(WeaponId::StasisField, target, caster))
				return false;

			if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
				return false;

			CUnit* targetOfTarget = target->orderTarget.unit;
			if (!targetOfTarget)
				return false;

			if (targetOfTarget->playerId >= 8)
				return false;

			if (caster->isTargetEnemy(targetOfTarget))
				return false;

			const s32 totalEnemyLife = getTotalEnemyLifeInArea(target->getX(), target->getY(), 96, caster, WeaponId::StasisField);
			if (!isUnderAttack && totalEnemyLife < 250)
				return false;

			const s32 totalAllyLife = getTotalAllyLifeInArea(target->getX(), target->getY(), 96, caster, WeaponId::StasisField);
			if (totalAllyLife * 2 >= totalEnemyLife)
				return false;

			return true;

		});
	}

} //AI
