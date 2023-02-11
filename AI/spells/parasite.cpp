#include "spells.h"
#include <AI/ai_common.h>
#include "globals.h"

namespace AI {

	Unit* find_best_parasite_target(Unit* caster, bool is_under_attack) {
		s32 bounds = 32 * 64;
		if (is_under_attack) {
			bounds = 32 * 9;
		}

		return scbw::UnitFinder::get_nearest_target(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
			caster, [&](Unit* target) {
			if (!is_target_worth_hitting(target, caster)) {
				return false;
			}

			UnitExt* target_ext = get_unit_ext(target);
			if (target_ext->timers[TimerParasiteArm]) {
				return false;
			}

			if (!scbw::can_weapon_target_unit(WeaponId::MatralethParasite, target, caster)) {
				return false;
			}

			if (target->can_detect()) {
				return true;
			}

			if (!(target->status & UnitStatus::IsHallucination)
				&& (target->id != UnitId::ZergOvileth || scbw::get_upgrade_level(target->player, UpgradeId::VentralSacs))
				&& units_dat::space_provided[target->id] > 0) {
				return true;
			}

			if (target->is_valid_caster()) {
				return true;
			}

			if (units_dat::base_property[target->id] & UnitProperty::Worker) {
				return true;
			}

			if (target->get_current_life_ingame() >= 300) {
				return true;
			}

			return false;
		});
	}
}
