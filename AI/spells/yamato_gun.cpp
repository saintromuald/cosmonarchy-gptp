#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* findBestYamatoGunTarget(Unit* caster, bool isUnderAttack)
	{
		auto match = [&](Unit* target)
		{
			if (!isTargetWorthHitting(target, caster))
			{
				return false;
			}

			if ((target->status & UnitStatus::GroundedBuilding) && target->hasWeapon())
			{
				return true;
			}

			if (target->id == UnitId::TerranBunker && target->hasLoadedUnit())
			{
				return true;
			}

			if (scbw::is_zerg_egg_new(target))
			{
				return false;
			}

			const s32 targetLife = target->getCurrentLifeInGame();
			if (200 <= targetLife && targetLife <= 450
				&& !(units_dat::BaseProperty[target->id] & UnitProperty::Hero))
			{
				return true;
			}

			return false;
		};

		if (isUnderAttack)
		{
			s32 bounds = 32 * 16;
			return scbw::UnitFinder::getNearestTarget(
				caster->getX() - bounds,
				caster->getY() - bounds,
				caster->getX() + bounds,
				caster->getY() + bounds,
				caster,
				match
			);
		}

		else
		{
			return scbw::UnitFinder::getNearestTarget(caster, match);
		}
	}
}
