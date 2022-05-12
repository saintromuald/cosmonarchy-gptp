#include "player_color.h"
#include "trigger_system.h"

static bool loaded_colors;
static bool can_randomize_player_colors;
static u8 tminimap_colors[MinimapColorCount];
static u8 tunit_colors[UnitColorCount];
static u8 player_color_list[PLAYABLE_PLAYER_COUNT];

static std::vector<u32> get_player_tints_from_color(u32 color);
static bool can_use_player_color(u32 color, u32 last_player);
static bool can_use_player_tint(u32 color_tint, u32 last_player);
static u32 get_random_tint();
static u32 get_random_color_from_tint(u32 tint);
static u32 get_color_for_personality(u32 player);

static bool is_player_randomizable(u32 player) {
	if (can_randomize_player_colors) {
		bool randomize = playerForceFlags[playerTable[player].force - 1].randomize_start_locations;
		if (deathTable[UnitId::ResourceVespeneGeyser].player[player] & VespeneDeaths_ToggleRandomizeColor) {
			randomize = !randomize;
		}
		return randomize;
	}
	return false;
}

static std::vector<u32> get_player_tints_from_color(u32 color) {
	std::vector<u32> tints = {};
	for (u32 tint = 0; tint < player_tints.size(); ++tint) {
		for (u32 color_index = 0; color_index < player_tints[tint].size(); color_index++) {
			if (player_tints[tint][color_index] == color) {
				tints.push_back(tint);
				break;
			}
		}
	}
	return tints;
}

static bool can_use_player_color(u32 color, u32 player) {
	for (u32 player_tint : get_player_tints_from_color(color)) {
		if (!can_use_player_tint(player_tint, player)) {
			return false;
		}
	}
	return true;
}

static bool can_use_player_tint(u32 color_tint, u32 player) {
	if (playerTable[player].race < banned_race_tints.size()) {
		for (u32 banned_tint : banned_race_tints[playerTable[player].race]) {
			if (color_tint == banned_tint) {
				return false;
			}
		}
	}
	if (*CURRENT_TILESET < banned_tileset_tints.size()) {
		for (u32 banned_tint : banned_tileset_tints[*CURRENT_TILESET]) {
			if (color_tint == banned_tint) {
				return false;
			}
		}
	}
	for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
		if (other_player != player && (other_player < player || !is_player_randomizable(other_player))) {
			for (u32 other_tint : get_player_tints_from_color(player_color_list[other_player])) {
				if (color_tint == other_tint) {
					return false;
				}
			}
		}
	}
	return true;
}

static u32 get_random_tint() {
	return scbw::random() % PlayerTintRollCount;
}

static u32 get_random_color_from_tint(u32 tint) {
	return player_tints[tint][scbw::random() % player_tints[tint].size()];
}

static u32 get_color_for_personality(u32 player) {
	const u32 race = playerTable[player].race;
	const u32 build = deathTable[UnitId::Spell_DisruptionWeb].player[player] - 1;
	if (build >= 0
		&& personality_colors.size() > race
		&& personality_colors[race].size() > build) {
		u32 size = personality_colors[race][build].size();
		u32 start_index = scbw::random() % size;
		u32 index = start_index;
		do {
			if (can_use_player_color(personality_colors[race][build][index], player)) {
				return personality_colors[race][build][index];
			}
			index = (index + 1) % size;
		} while (index != start_index);
	}
	return PlayerColor::Invalid;
}

static bool has_enough_colors() {
	u32 banned_race_tints_count = 0;
	for (const std::vector<u32>& race : banned_race_tints) {
		if (race.size() > banned_race_tints_count) {
			banned_race_tints_count = race.size();
		}
	}
	if (player_tints.size() - banned_tileset_tints[*CURRENT_TILESET].size() - banned_race_tints_count < PLAYABLE_PLAYER_COUNT) {
		return false;
	}
	return true;
}

static void randomize_player_color(u32 player) {
	if (playerTable[player].type == PlayerType::Computer) {
		u32 color = get_color_for_personality(player);
		if (color != PlayerColor::Invalid) {
			set_player_color(player, color);
			return;
		}
	}
	u32 tint = get_random_tint();
	while (!can_use_player_tint(tint, player)) {
		tint = get_random_tint();
	}
	set_player_color(player, get_random_color_from_tint(tint));
}

static void match_color_of(u32 player) {
	for (u32 color = 0; color < ColorSetCount; ++color) {
		bool match = true;
		for (u32 i = 0; i < PLAYABLE_PLAYER_COUNT; ++i) {
			if (playerInGameColors[player * PLAYABLE_PLAYER_COUNT + i] != tunit_colors[color * PLAYABLE_PLAYER_COUNT + i]) {
				match = false;
				break;
			}
		}
		if (match) {
			set_player_color(player, color);
			break;
		}
	}
}

void set_player_color(u32 player, u32 color) {
	player_color_list[player] = color;
	playerMinimapColors[player] = tminimap_colors[color + ColorSetCount * *CURRENT_TILESET];
	for (u32 i = 0; i < PLAYABLE_PLAYER_COUNT; i++) {
		playerInGameColors[player * PLAYABLE_PLAYER_COUNT + i] = tunit_colors[color * PLAYABLE_PLAYER_COUNT + i];
	}
}

static void load_color_tables() {
	if (!loaded_colors) {
		loaded_colors = true;
		helper::SBmpLoadImage("game\\tminimap.pcx", 0, tminimap_colors, MinimapColorCount, 0, 0, 0);
		helper::SBmpLoadImage("game\\tunit.pcx", 0, tunit_colors, UnitColorCount, 0, 0, 0);
	}
}

void load_player_colors() {
	load_color_tables();
	can_randomize_player_colors = has_enough_colors();
	for (u32 player = 0; player < PLAYABLE_PLAYER_COUNT; ++player) {
		if (is_player_randomizable(player)) {
			randomize_player_color(player);
		} else {
			match_color_of(player);
		}
	}
}

static bool can_get_player_name(u32 player) {
	if (!has_custom_name[player]
	&&  playerTable[player].type == PlayerType::Computer) {
		return true;
	}
	return false;
}

static bool is_valid_name(u32 player, char* name) {
	if (!name) {
		return false;
	}
	for (u32 i = 0; i < 8; ++i) {
		if (i != player && can_get_player_name(i)) {
			if (strcmp(playerNameList->player[i].player_name, name) == 0) {
				return false;
			}
		}
	}
	return true;
}

static u32 get_name_count(u32 race, u32 personality) {
	u32 count = 0;
	if (personality < AiPersonality_AnyCount) {
		for (u32 i = 0; i < PlayerPesonalityMaxNames; ++i) {
			if (player_personality_names[race][personality][i]) {
				count = i;
			}
		}
	}
	return count;
}

static void update_player_name(u32 player) {
	if (!can_get_player_name(player)) {
		return;
	}
	u32 race = playerTable[player].race;
	u32 personality = unit_deaths[UnitId::Spell_DisruptionWeb].player[player];
	u32 count = get_name_count(race, personality);
	if (count) {
		for (u32 tries = 0; tries < 20; ++tries) {
			u32 random = scbw::randBetween(0, PlayerPesonalityMaxNames - 1);
			char* name = player_personality_names[race][personality][random];
			if (is_valid_name(player, name)) {
				strcpy_s(playerNameList->player[player].player_name, name);
				return;
			}
		}
	}
	strcpy_s(playerNameList->player[player].player_name, "Computer"); // this seems to crash sometimes for unknown reasons
}

void load_player_names() {
	for (u32 i = 0; i < 8; ++i) {
		update_player_name(i);
	}
}