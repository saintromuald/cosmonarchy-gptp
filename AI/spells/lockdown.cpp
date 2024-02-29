#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* find_best_lockdown_target(Unit* caster, bool isUnderAttack)
	{
		if (caster->mine_carrier.mine_count == 0)
		{
			return 0;
		}

		s32 bounds;
		if (isUnderAttack)
		{
			bounds = 32 * 9;
		}
		else if (is_campaign_ai(caster->player))
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

		return bw::UnitFinder::get_nearest_target(
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

			if (!bw::can_weapon_target_unit(WeaponId::Lockdown, target, caster))
			{
				return false;
			}

			if (target->is_frozen())
			{
				return false;
			}

			if (is_target_attacking_ally(target, caster))
			{
				return true;
			}

			return false;
		});
	}
}
