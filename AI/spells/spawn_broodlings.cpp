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

		return scbw::UnitFinder::get_nearest_target(
			caster->get_x() - bounds, caster->get_y() - bounds,
			caster->get_x() + bounds, caster->get_y() + bounds,
			caster, [&](Unit* target)
		{
			if (!is_target_worth_hitting(target, caster))
			{
				return false;
			}

			if (!scbw::can_weapon_target_unit(WeaponId::SpawnBroodlings, target, caster))
			{
				return false;
			}

			if (units_dat::base_property[target->id] & UnitProperty::Hero)
			{
				return false;
			}

			if (!isUnderAttack && target->get_current_life_ingame() < 100)
			{
				return false;
			}

			if (units_dat::base_property[target->id] & UnitProperty::Worker
				|| target->id == UnitId::TerranPhalanxTankMode
				|| target->id == UnitId::TerranPhalanxSiegeMode
				|| target->id == UnitId::TerranCleric
				|| (target->main_order_id == OrderId::NukeWait
					|| target->main_order_id == OrderId::NukeTrack))
			{
				return true;
			}

			return false;

		});
	}
}