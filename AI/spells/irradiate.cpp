#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{

	Unit* findBestIrradiateTarget(Unit* caster, bool isUnderAttack)
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

			if (!isTargetWorthHitting(target, caster))
			{
				return true;
			}

			if (target->irradiateTimer)
			{
				return true;
			}

			if (!scbw::can_weapon_target_unit(WeaponId::Irradiate, target, caster))
			{
				return true;
			}

			if (isUnderAttack || !isUmsMode(caster->player))
			{
				return true;
			}

			if (units_dat::BaseProperty[target->id] & UnitProperty::Worker)
			{
				return true;
			}

			if (target->id == UnitId::ZergOvileth || target->id == UnitId::TerranCleric)
			{
				return true;
			}

			return false;

		});
	}

} //AI
