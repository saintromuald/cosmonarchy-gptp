#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{

	Unit* find_best_stasis_field_target(Unit* caster, bool isUnderAttack)
	{
		s32 bounds;
		if (isUnderAttack)
			bounds = 32 * 9;
		else if (is_campaign_ai(caster->player))
			bounds = 32 * 64;
		else
			bounds = 32 * 32;

		return bw::UnitFinder::get_nearest_target(
			caster->position.x - bounds,
			caster->position.y - bounds,
			caster->position.x + bounds,
			caster->position.y + bounds,
			caster,
			[&](Unit* target)
		{

			if (!is_target_worth_hitting(target, caster))
				return false;

			if (!bw::can_weapon_target_unit(WeaponId::StasisField, target, caster))
				return false;

			if (units_dat::base_property[target->id] & UnitProperty::Building)
				return false;

			Unit* targetOfTarget = target->order_target.unit;
			if (!targetOfTarget)
				return false;

			if (targetOfTarget->player >= 8)
				return false;

			if (caster->is_target_enemy(targetOfTarget))
				return false;

			const s32 totalEnemyLife = get_total_enemy_life_in_area(target->get_x(), target->get_y(), 96, caster, WeaponId::StasisField);
			if (!isUnderAttack && totalEnemyLife < 250)
				return false;

			const s32 totalAllyLife = get_total_ally_life_in_area(target->get_x(), target->get_y(), 96, caster, WeaponId::StasisField);
			if (totalAllyLife * 2 >= totalEnemyLife)
				return false;

			return true;

		});
	}

} //AI
