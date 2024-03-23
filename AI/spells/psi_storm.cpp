#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* find_best_psi_storm_target(Unit* caster, bool isUnderAttack)
	{
		s32 bounds = 32 * 64;
		if (isUnderAttack) {
			bounds = 32 * 16;
		}

		return bw::UnitFinder::get_nearest_target(
			caster->position.x - bounds,
			caster->position.y - bounds,
			caster->position.x + bounds,
			caster->position.y + bounds,
			caster,
			[&](Unit* target)
		{
			if (!is_target_worth_hitting(target, caster))
			{
				return false;
			}

			if (!bw::can_weapon_target_unit(WeaponId::PsiStorm, target, caster))
			{
				return false;
			}

			const s32 total_enemy_life = get_total_enemy_life_in_area(target->get_x(), target->get_y(), 96, caster, WeaponId::PsiStorm);
			if (!isUnderAttack && total_enemy_life < 150)
			{
				return false;
			}

			const s32 total_ally_life = get_total_ally_life_in_area(target->get_x(), target->get_y(), 96, caster, WeaponId::PsiStorm);
			if (total_ally_life * 2 >= total_enemy_life)
			{
				return false;
			}
			return true;
		});
	}
}