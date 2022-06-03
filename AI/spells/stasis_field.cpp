#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{

	Unit* findBestStasisFieldTarget(Unit* caster, bool isUnderAttack)
	{
		s32 bounds;
		if (isUnderAttack)
			bounds = 32 * 9;
		else if (isUmsMode(caster->player))
			bounds = 32 * 64;
		else
			bounds = 32 * 32;

		return scbw::UnitFinder::get_nearest_target(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
			caster,
			[&](Unit* target)
		{

			if (!isTargetWorthHitting(target, caster))
				return false;

			if (!scbw::can_weapon_target_unit(WeaponId::StasisField, target, caster))
				return false;

			if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
				return false;

			Unit* targetOfTarget = target->orderTarget.unit;
			if (!targetOfTarget)
				return false;

			if (targetOfTarget->player >= 8)
				return false;

			if (caster->isTargetEnemy(targetOfTarget))
				return false;

			const s32 totalEnemyLife = getTotalEnemyLifeInArea(target->get_x(), target->get_y(), 96, caster, WeaponId::StasisField);
			if (!isUnderAttack && totalEnemyLife < 250)
				return false;

			const s32 totalAllyLife = getTotalAllyLifeInArea(target->get_x(), target->get_y(), 96, caster, WeaponId::StasisField);
			if (totalAllyLife * 2 >= totalEnemyLife)
				return false;

			return true;

		});
	}

} //AI
