#include "building_modifier.h"
#include <globals.h>
#include "trigger_system\trigger_system.h"

namespace cm {
BuildingModifier advance_building_modifier(Unit* unit, const ProgressBuildOptions& options) {
	BuildingModifier modifier{.factor = 1_fp64, .integer_step = 1};

	assert(unit);
    auto& unit_ext = unit->ext();
	if (!options.use_stock_speed) {
		Unit* builder = options.builder;

		bool crucible_active = !unit_ext.unit_vectors[UnitVectorCrucibles].empty();
		if (!crucible_active && builder) {
			UnitExt* builder_ext = get_unit_ext(builder);
			if (!builder_ext->unit_vectors[UnitVectorCrucibles].empty()) {
				crucible_active = true;
			}
		}
		if (crucible_active) {
			modifier.factor *= 1.5_fp64;
		}

		if (unit->player < PlayerLimit) {
			const auto& cm_player = g_globals->players[unit->player];
			modifier.factor *= cm_player.build_multiplier;
		}
		if (bw::get_game_kind() == GameKind::Debug) {
			modifier.factor *= 32;
		}
	}

    unit_ext.build_progress_accumulator += modifier.factor - 1;
    const auto step = (u32)unit_ext.build_progress_accumulator; // truncate
    unit_ext.build_progress_accumulator -= step;
    modifier.integer_step += step;

	if (options.already_ticked) {
		modifier.factor -= 1;
		modifier.integer_step -= 1;
	}
	return modifier;
}

bool progress_build(Unit* unit, const ProgressBuildOptions& options) {
	if (!unit) {
		return false;
	}
	if (!unit->remaining_build_time) {
		return true;
	}

	const auto modifier = advance_building_modifier(unit, options);
	if (unit->remaining_build_time >= modifier.integer_step) {
		unit->remaining_build_time -= modifier.integer_step;
	} else {
		unit->remaining_build_time = 0;
	}

	if (modifier.factor) {
		if (options.shield_gain) {
			const s32 max_shields = units_dat::max_shield_points[unit->id] << 8;
			const auto gain = s32(options.shield_gain * modifier.factor);
			unit->shields = std::min(unit->shields + gain, max_shields);
		}
		// Need to call set_hitpoints for shield case, regardless of hp_gain, to update unit force value and etc
		if (options.hp_gain || options.shield_gain) {
			const auto gain = s32(options.hp_gain * modifier.factor);
			unit->set_hitpoints(unit->hitpoints + gain);
		}
	}
	return false;
}
} // namespace cm