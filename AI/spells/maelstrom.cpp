#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI {

	CUnit* findBestMaelstromTarget(CUnit* caster, bool isUnderAttack) {
		int bounds;
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
			[&](CUnit* target) {

			if (!isTargetWorthHitting(target, caster))
				return false;

			if (!scbw::canWeaponTargetUnit(WeaponId::Maelstrom, target, caster))
				return false;

			if (!(units_dat::BaseProperty[target->id] & UnitProperty::Organic))
				return false;

			if (target->maelstromTimer)
				return false;

			if (target->id == UnitId::overlord
				&& target->movementFlags & MovementFlags::Accelerating
				&& target->hasLoadedUnit())
				return true;

			if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
				return false;

			if (isTargetAttackingAlly(target, caster)
				&& !scbw::isZergEggNew(target, true, true)) {

				const int totalEnemyLife = getTotalEnemyLifeInArea(target->getX(), target->getY(), 96, caster, WeaponId::Maelstrom);
				if (!isUnderAttack && totalEnemyLife < 250)
					return false;

				const int totalAllyLife = getTotalAllyLifeInArea(target->getX(), target->getY(), 96, caster, WeaponId::Maelstrom);
				if (totalAllyLife * 2 >= totalEnemyLife)
					return false;

				return true;
			} else
				return false;
		});
	}

} //AI
