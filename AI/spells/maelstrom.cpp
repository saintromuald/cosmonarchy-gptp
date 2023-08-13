#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* find_best_maelstrom_target(Unit* caster, bool isUnderAttack)
	{
		s32 bounds;
		if (isUnderAttack)
		{
			bounds = 32 * 9;
		} else if (is_campaign_ai(caster->player)) {
			bounds = 32 * 64;
		} else {
			bounds = 32 * 32;
		}

		return scbw::UnitFinder::get_nearest_target(
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
			if (!scbw::can_weapon_target_unit(WeaponId::MindTyrantMaelstrom, target, caster))
			{
				return false;
			}
			if (target->maelstrom_timer)
			{
				return false;
			}
			if (target->id == UnitId::ZergOvileth && target->movement_flags & MovementFlags_Accelerating && target->has_loaded_unit())
			{
				return true;
			}
			if (units_dat::base_property[target->id] & UnitProperty::Building)
			{
				return false;
			}
			if (is_target_attacking_ally(target, caster)
				&& !scbw::is_zerg_egg_new(target, true, true))
			{
				const s32 totalEnemyLife = get_total_enemy_life_in_area(target->get_x(), target->get_y(), 96, caster, WeaponId::MindTyrantMaelstrom);
				if (!isUnderAttack && totalEnemyLife < 250)
				{
					return false;
				}
				const s32 totalAllyLife = get_total_ally_life_in_area(target->get_x(), target->get_y(), 96, caster, WeaponId::MindTyrantMaelstrom);
				if (totalAllyLife * 2 >= totalEnemyLife)
				{
					return false;
				}
				return true;
			} else {
				return false;
			}
		});
	}
}
