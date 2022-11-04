#include "custom_ai.h"
#include "ai_layouts.h"

CustomAiPlayer custom_ai_players[PlayerLimit];
CustomAiTown custom_ai_towns[AiTownLimit];

void init_custom_ai() {
	memset(custom_ai_players, 0, sizeof(custom_ai_players));
	memset(custom_ai_towns, 0, sizeof(custom_ai_towns));
}

static void update_custom_player(CustomAiPlayer* custom_ai_player, u32 player) {
	for (u32 i = 0; i < PlayerLimit; ++i) {

	}
}

static void update_custom_town(CustomAiTown* custom_ai_town, u32 town) {
	AiTown* ai_town = &g_ai_town_array[town];
	if (ai_town->main_building && ai_town->main_building->status & UnitStatus::Completed) {
		if (custom_ai_town->gas_cap < 3) {
			if (custom_ai_town->gas_timer < 30 * 24) {
				custom_ai_town->gas_timer++;
			} else {
				custom_ai_town->gas_timer = 0;
				custom_ai_town->gas_cap += 1;
			}
		}

		u32 capped_id = UnitId::None;
		switch (g_player_table[ai_town->player].race) {
			case RaceId::Terran:  capped_id = UnitId::TerranReservoir; break;
			case RaceId::Protoss: capped_id = UnitId::ProtossAquifer;  break;
			case RaceId::Zerg:	  capped_id = UnitId::ZergExcisant;    break;
		}

		if (capped_id != UnitId::None) {
			u32 total_gas = 0;
			u32 harvestable_gas = 0;
			u32 unupgradeable_gas = 0;
			for (u32 i = 0; i < 3; ++i) {
				Unit* unit = ai_town->gas_buildings[i];
				if (unit) {
					total_gas++;
					if (unit->status & UnitStatus::Completed) {
						harvestable_gas++;
						if (unit->id == UnitId::ResourceVespeneRidge) {
							unupgradeable_gas++;
						}
					}
				}
			}

			u32 requested_count = town_count_units(ai_town, capped_id, false);
			u32 mineral_threshold = requested_count ? 100 : 500;
			u32 needed_threshold = 100 * (1 + requested_count);
			if (total_gas == 1) {
				needed_threshold *= 2;
			}

			if ((harvestable_gas || total_gas == 1)
				&& g_game_minerals[ai_town->player] >= mineral_threshold
				&& g_aiscript_controller[ai_town->player].vespene_needed >= needed_threshold) {
				s32 request_count = std::min(total_gas - unupgradeable_gas, custom_ai_town->gas_cap) - requested_count;
				if (request_count > 0) {
					u32 priority = 125 - requested_count;
					for (u32 i = 0; i < request_count; ++i) {
						scbw::town_add_spending_request(ai_town, 1, capped_id, priority);
						priority -= 1;
					}
				}
			}
		}
	}
}

void update_custom_ai() {
	for (u32 i = 0; i < PlayerLimit; ++i) {
		update_custom_player(&custom_ai_players[i], i);
	}

	for (u32 i = 0; i < AiTownLimit; ++i) {
		if (g_ai_town_array[i].inited) {
			custom_ai_towns[i].active = true;
			update_custom_town(&custom_ai_towns[i], i);
		} else if (custom_ai_towns[i].active) {
			custom_ai_towns[i] = {};
		}
	}
}