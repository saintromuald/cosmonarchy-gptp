#include "spells.h"
#include <AI/ai_common.h>

// V241 for VS2008

namespace AI
{

	Unit* find_best_emp_shockwave_target(Unit* caster, bool isUnderAttack)
	{

		s32 bounds;

		if (isUnderAttack)
			bounds = 32 * 9;
		else
			bounds = 32 * 64;

		Unit* result = scbw::UnitFinder::get_nearest_target(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
			caster,
			[&](Unit* target)
		{
			if (!is_target_worth_hitting(target, caster))
				return false;

			if (!scbw::unit_has_shields(target))
				return false;

			if (!scbw::can_weapon_target_unit(WeaponId::EMP_Shockwave, target, caster))
				return false;

			const s32 totalEnemyShields = get_total_enemy_shields_in_area(target->get_x(), target->get_y(), 160, caster);
			if (totalEnemyShields >= 200)
				return true;

			return false;
		});

		if (result || isUnderAttack)
			return result;

		return scbw::UnitFinder::get_nearest_target(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
			caster,
			[&](Unit* target)
		{
			if (!is_target_worth_hitting(target, caster))
				return false;

			if (!target->is_valid_caster())
				return false;

			if (!scbw::can_weapon_target_unit(WeaponId::EMP_Shockwave, target, caster))
				return false;

			const s32 totalEnemyEnergy = get_total_enemy_energy_in_area(target->get_x(), target->get_y(), 160, caster);
			if (totalEnemyEnergy >= 200)
				return true;

			return false;
		});
	}

} //AI