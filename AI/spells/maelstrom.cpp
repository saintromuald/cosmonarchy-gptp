#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{

	Unit* findBestMaelstromTarget(Unit* caster, bool isUnderAttack)
	{
		s32 bounds;
		if (isUnderAttack)
		{
			bounds = 32 * 9;
		}
		else if (isUmsMode(caster->player))
		{
			bounds = 32 * 64;
		}
		else
		{
			bounds = 32 * 32;
		}

		return scbw::UnitFinder::getNearestTarget(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
			caster,
			[&](Unit* target)
		{
			if (!isTargetWorthHitting(target, caster))
			{
				return false;
			}
			if (!scbw::can_weapon_target_unit(WeaponId::Maelstrom, target, caster))
			{
				return false;
			}
			if (!(units_dat::BaseProperty[target->id] & UnitProperty::Organic))
			{
				return false;
			}
			if (target->maelstromTimer)
			{
				return false;
			}
			if (target->id == UnitId::ZergOvileth
				&& target->movementFlags & MovementFlags::Accelerating
				&& target->hasLoadedUnit())
			{
				return true;
			}
			if (units_dat::BaseProperty[target->id] & UnitProperty::Building)
			{
				return false;
			}
			if (isTargetAttackingAlly(target, caster)
				&& !scbw::is_zerg_egg_new(target, true, true))
			{
				const s32 totalEnemyLife = getTotalEnemyLifeInArea(target->get_x(), target->get_y(), 96, caster, WeaponId::Maelstrom);
				if (!isUnderAttack && totalEnemyLife < 250)
				{
					return false;
				}
				const s32 totalAllyLife = getTotalAllyLifeInArea(target->get_x(), target->get_y(), 96, caster, WeaponId::Maelstrom);
				if (totalAllyLife * 2 >= totalEnemyLife)
				{
					return false;
				}
				return true;
			}
			else
			{
				return false;
			}
		});
	}

}
