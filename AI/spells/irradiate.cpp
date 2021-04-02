#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI {

	CUnit* findBestIrradiateTarget(CUnit* caster, bool isUnderAttack) {

		int bounds;

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
			[&](CUnit* target) {

				if (!isTargetWorthHitting(target, caster))
					return false;

				if (target->irradiateTimer)
					return false;

				if (!scbw::canWeaponTargetUnit(WeaponId::Irradiate, target, caster))
					return false;

				if (!(units_dat::BaseProperty[target->id] & UnitProperty::Organic))
					return false;

				if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
					return false;

				if (scbw::isZergEggNew(target))
					return false;

				if (isUnderAttack || !isUmsMode(caster->playerId))
					return true;

				if (units_dat::BaseProperty[target->id] & UnitProperty::Worker)
					return true;

				if (target->id == UnitId::overlord || target->id == UnitId::medic)
					return true;

				return false;

			});
	}

} //AI
