#include "UprisingGameMode.h"
#include "BW/api.h"
#include "trigger_system/trigger_system.h"
#include "hooks/limits/sfx_extender.h"
#include "ai_layouts.h"

// I copied and edited Threshold code for this. -Pr0nogo

// TODO: 
//  * FIXES
//	  * Randomness for spawner always returns 1. Why?
//	 --	Tried moving it away from game_init_first_frame and into next_frame with checks for frame count being 3 or 12, no dice.
//	  * Upstarts can't see anyone because they're an extended player, just like Turned (zombies).
//	  * AI expands into Upstart camps.
//  * NEW FEATURES
//	  * When 10 upstarts are near the spawner, they should go on an attack
//	  * Terran spawners should add addons
//	  * Zerg spawners should mutate into higher tiers
//	  * Protoss spawners should warp into successor options
//    * All camps should set up defensive structures over time

template <typename Seq>
static auto& get_sync_random_from(Seq&& sequence) {
	return *std::next(std::begin(sequence), bw::rand_between(0, std::size(sequence) - 1));
}

UprisingGameMode::UprisingGameMode() {
	for (auto& unit : g_chk_cache) {
		if (unit.player != NeutralPlayer) {
			continue;
		}
		u32 flags = unit.valid_state_flags & unit.state_flags;
		if (flags & ChkUnitFlags_Cloaked) {
			continue;
		}

		Point16 position = { unit.x, unit.y };
		switch (unit.unit_id) {
			case UnitId::TownCenterMarker: {
				UpstartSpawnLocation spawn{
					.position = position,
					.resarea = bw::get_closest_resarea(position),
				};

				// Ignore active start locations
				for (u32 player = 0; player < PlayerLimit; player++) {
					if (bw::player_is_active(player) && spawn.resarea == helper::get_closest_base_from_startloc(player, spawn.position.x, spawn.position.y)) {
						break;
					}
				}

				const auto& [unit_id, spawns] = get_sync_random_from(s_spawner_types);
				Unit* unit = bw::create_unit_in_game((u16)unit_id, NeutralHostilePlayerId, spawn.position.x, spawn.position.y);
				if (!unit) {
					break;
				}

				UnitExt* unit_ext = get_unit_ext(unit);
				unit_ext->values[ValueUprisingSpawner] = 1;
				unit_ext->timers[TimerUprisingSpawn] = 3;
				if (unit->get_race() == RaceId::Zerg) {
					bw::spawn_custom_creep(unit);
				}
			} break;
		}
	}
}

void UprisingGameMode::tick_unit(Unit* unit) {
	UnitExt* unit_ext = get_unit_ext(unit);

	if (unit_ext->values[ValueUprisingSpawner] && !unit_ext->timers[TimerUprisingSpawn]) {
		unit_ext->timers[TimerUprisingSpawn] = 3;
	}

	if (unit_ext->timers[TimerUprisingSpawn] == 1) {
		const auto& units = s_spawner_types[(UnitId::Enum)unit->id];

		EntryArray upstarts = {};
		unit_search_radial(&upstarts, unit->position, unit->get_sight_range() * 32);
		unit_search_filter(&upstarts, unit, FilterOwn | FilterNoBuilding);
		if (upstarts.count < 10) {
			UnitId::Enum spawn_id = get_sync_random_from(units);
			bool two_per_egg = units_dat::base_property[spawn_id] & UnitProperty::TwoUnitsIn1Egg;
			u32 spawn_count = two_per_egg ? 2 : 1;
			for (u32 i = 0; i < spawn_count; i++) {
				bw::create_unit_in_game(spawn_id, NeutralHostilePlayerId, unit->position.x, unit->position.y, unit);
			}
			unit_ext->timers[TimerUprisingSpawn] = units_dat::time_cost[spawn_id];
		} else {
			unit_ext->timers[TimerUprisingSpawn] = 3;
		}
	}
}