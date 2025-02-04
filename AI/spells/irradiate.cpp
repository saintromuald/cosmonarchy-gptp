#include "spells.h"
#include <AI/ai_common.h>

namespace AI {
	Unit* find_best_irradiate_target(Unit* caster, bool is_under_attack) {
		u32 bounds = is_under_attack ? 9 * 32 : 64 * 32;
		return bw::UnitFinder::get_nearest_target(caster->get_x() - bounds, caster->get_y() - bounds, caster->get_x() + bounds, caster->get_y() + bounds, caster, [&](Unit* target) {
			if (!is_target_worth_hitting(target, caster)) {
				return true;
			}
			if (target->irradiate_timer) {
				return true;
			}
			if (!bw::can_weapon_target_unit(WeaponId::Irradiate, target, caster)) {
				return true;
			}
			if (is_under_attack || !is_campaign_ai(caster->player)) {
				return true;
			}
			if (units_dat::base_property[target->id] & UnitProperty::Worker) {
				return true;
			}
			if (target->id == UnitId::ZergOvileth || target->id == UnitId::TerranCleric) {
				return true;
			}
			return false;
		});
	}
} //AI