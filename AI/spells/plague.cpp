#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* findBestPlagueTarget(Unit* caster, bool isUnderAttack)
	{
		s32 bounds;
		if (isUnderAttack)
		{
			bounds = 32 * 9;
		}
		else
		{
			bounds = 32 * 128;
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
			if (!scbw::can_weapon_target_unit(WeaponId::Plague, target, caster))
			{
				return false;
			}
			if (target->plague_timer)
			{
				return false;
			}
			if (units_dat::base_property[target->id] & UnitProperty::Hero)
			{
				return false;
			}
			const s32 totalEnemyLife = get_total_enemy_life_in_area(target->get_x(), target->get_y(), 96, caster, WeaponId::Plague);
			if (!isUnderAttack && totalEnemyLife < 250)
			{
				return false;
			}
			const s32 totalAllyLife = get_total_ally_life_in_area(target->get_x(), target->get_y(), 96, caster, WeaponId::Plague);
			if (totalAllyLife * 2 >= totalEnemyLife)
			{
				return false;
			}
			return true;
		});
	}
}