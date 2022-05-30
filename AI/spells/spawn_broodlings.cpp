#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* findBestSpawnBroodlingsTarget(Unit* caster, bool isUnderAttack)
	{
		s32 bounds;
		if (isUnderAttack)
		{
			bounds = 32 * 9;
		}

		else
		{
			bounds = 32 * 64;
		}

		return scbw::UnitFinder::getNearestTarget(
			caster->getX() - bounds, caster->getY() - bounds,
			caster->getX() + bounds, caster->getY() + bounds,
			caster, [&](Unit* target)
		{
			if (!isTargetWorthHitting(target, caster))
			{
				return false;
			}

			if (!scbw::can_weapon_target_unit(WeaponId::SpawnBroodlings, target, caster))
			{
				return false;
			}

			if (units_dat::BaseProperty[target->id] & UnitProperty::Hero)
			{
				return false;
			}

			if (!isUnderAttack && target->get_current_life_ingame() < 100)
			{
				return false;
			}

			if (units_dat::BaseProperty[target->id] & UnitProperty::Worker
				|| target->id == UnitId::TerranPhalanxTankMode
				|| target->id == UnitId::TerranPhalanxSiegeMode
				|| target->id == UnitId::TerranCleric
				|| (target->mainOrderId == OrderId::NukeWait
					|| target->mainOrderId == OrderId::NukeTrack))
			{
				return true;
			}

			return false;

		});
	}
}