#include "custom_ai.h"

CustomAiPlayer custom_ai_player[PlayerLimit];
CustomAiTown custom_ai_towns[AiTownLimit];

void init_custom_ai() {
	memset(custom_ai_player, 0, sizeof(custom_ai_player));
	memset(custom_ai_towns, 0, sizeof(custom_ai_towns));
}

static void update_custom_player(CustomAiPlayer* custom_ai_player, u32 player) {
	
}

static void update_custom_town(CustomAiTown* custom_ai_town, u32 town) {
	AiTown* ai_town = &g_ai_town_array[town];
	u32 mineral_threshold = custom_ai_town->gas_requested ? 100 : 500;
	if (!ai_town->main_building) {
		// don't do anything, let's wait for the main building
	} else if (custom_ai_town->gas_cooldown) {
		custom_ai_town->gas_cooldown--;
	} else if (g_game_minerals[ai_town->player] >= mineral_threshold) {
		for (u32 i = 0; i < 3; ++i) {
			if (!custom_ai_town->gas_requested[i] && ai_town->gas_buildings[i] && ai_town->gas_buildings[i]->id == UnitId::ResourceVespeneGeyser) {
				u32 capped_id = UnitId::None;
				u32 race = g_player_table[ai_town->player].race;
				switch (race) {
					case RaceId::Terran:	capped_id = UnitId::TerranReservoir; break;
					case RaceId::Protoss:	capped_id = UnitId::ProtossAquifer; break;
					case RaceId::Zerg:		capped_id = UnitId::ZergExcisant; break;
				}
				if (capped_id != UnitId::None) {
					custom_ai_town->gas_requested[i] = true;
					helper::ai_build(capped_id, false, custom_ai_town->gas_requested_count + 1, ai_town, 105 - custom_ai_town->gas_requested_count);
					custom_ai_town->gas_requested_count++;
					custom_ai_town->gas_cooldown = 30 * 24;
					break;
				}
			}
		}
	}
}

void update_custom_ai() {
	for (u32 i = 0; i < PlayerLimit; ++i) {
		update_custom_player(&custom_ai_player[i], i);
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