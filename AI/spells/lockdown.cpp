#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* findBestLockdownTarget(Unit* caster, bool isUnderAttack)
	{
		if (caster->mine_carrier.mine_count == 0)
		{
			return false;
		}

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

		if (caster->mine_carrier.mine_count == 0)
		{
			return nullptr;
		}

		return scbw::UnitFinder::get_nearest_target(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
			caster,
			[&](Unit* target)
		{

			if (!is_target_worth_hitting(target, caster))
			{
				return false;
			}

			if (!scbw::can_weapon_target_unit(WeaponId::Lockdown, target, caster))
			{
				return false;
			}

			if (target->is_frozen())
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
