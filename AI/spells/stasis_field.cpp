#include "spells.h"
#include <AI/ai_common.h>

namespace AI { 
	Unit* find_best_stasis_field_target(Unit* caster, bool is_under_attack) {
		s32 bounds = is_under_attack ? 9 * 32 : 32 * 32;
		return bw::UnitFinder::get_nearest_target(caster->position.x - bounds, caster->position.y - bounds, caster->position.x + bounds, caster->position.y + bounds, caster, [&](Unit* target) {
			if (!is_target_worth_hitting(target, caster)) {
				return false;
			}
			if (!bw::can_weapon_target_unit(WeaponId::StasisField, target, caster)) {
				return false;
			}
			if (units_dat::base_property[target->id] & UnitProperty::Building) {
				return false;
			}
			Unit* target_of_target = target->order_target.unit;
			if (!target_of_target) {
				return false;
			}
			if (target_of_target->player >= PlayerLimit) {
				return false;
			}
			if (caster->is_target_enemy(target_of_target)) {
				return false;
			}
			const s32 total_enemy_life = get_total_enemy_life_in_area(target->get_x(), target->get_y(), 96, caster, WeaponId::StasisField);
			if (!is_under_attack && total_enemy_life < 250) {
				return false;
			}
			const s32 total_ally_life = get_total_ally_life_in_area(target->get_x(), target->get_y(), 96, caster, WeaponId::StasisField);
			if (total_ally_life * 2 >= total_enemy_life) {
				return false;
			}
			return true;
		});
	}
} //AI
