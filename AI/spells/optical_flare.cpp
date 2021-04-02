#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI {

	CUnit* findBestOpticalFlareTarget(CUnit* caster, bool isUnderAttack) {

		int bounds;

		if (isUnderAttack)
			bounds = 32 * 9;
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

				if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
					return false;

				if (target->isBlind)
					return false;

				if (target->canDetect())
					return true;

				if (target->getCurrentLifeInGame() > 80)
					return true;

				return false;

			});
	}


} //AI
