#include "building_modifier.h"
#include <globals.h>
#include "trigger_system\trigger_system.h"

namespace cm {

BuildingModifier get_building_modifier(Unit* unit, const ProgressBuildOptions& options) {
	BuildingModifier modifier{.factor = 1, .integer_step = 1};
	if (unit && !options.use_stock_speed) {
		UnitExt* unit_ext = get_unit_ext(unit);

		Unit* builder = options.builder;
		if (options.is_constructing) {
			/*
			if ((builder && builder->id == UnitId::TerranTalos)
				|| unit->create_origin == UnitId::ProtossArtisan
				|| unit->create_origin == UnitId::ZergGosvileth) {
				modifier.factor *= 1.5;
			}
			*/
		}

		bool crucible_active = !unit_ext->unit_vectors[UnitVectorCrucibles].empty();
		if (!crucible_active && builder) {
			UnitExt* builder_ext = get_unit_ext(builder);
			if (!builder_ext->unit_vectors[UnitVectorCrucibles].empty()) {
				crucible_active = true;
			}
		}
		if (crucible_active) {
			modifier.factor *= 1.5;
		}

		if (unit->player < PlayerLimit) {
			const auto& cm_player = g_globals->players[unit->player];
			modifier.factor *= cm_player.build_multiplier;
		}
		if (bw::get_game_kind() == GameKind_Debug) {
			modifier.factor *= 32;
		}

		if (unit_ext->build_progress_last_update_tick != g_elapsed_time_frames) {
			unit_ext->build_progress_last_update_tick = g_elapsed_time_frames;

			unit_ext->build_progress_float += modifier.factor;
			unit_ext->build_progress_integer = floor(unit_ext->build_progress_float);
			unit_ext->build_progress_float -= unit_ext->build_progress_integer;
		}
		modifier.integer_step = unit_ext->build_progress_integer;
	}

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

	const auto modifier = get_building_modifier(unit, options);
	if (unit->remaining_build_time >= modifier.integer_step) {
		unit->remaining_build_time -= modifier.integer_step;
	} else {
		unit->remaining_build_time = 0;
	}

	if (modifier.factor) {
		if (options.shield_gain) {
			s32 max_shields = units_dat::max_shield_points[unit->id] << 8;
			unit->shields = std::min<s32>(unit->shields + options.shield_gain * modifier.factor, max_shields);
		}
		// Need to call set_hitpoints for shield case, regardless of hp_gain, to update unit force value and etc
		if (options.hp_gain || options.shield_gain) {
			unit->set_hitpoints(unit->hitpoints + options.hp_gain * modifier.factor);
		}
	}

	return false;
}

}