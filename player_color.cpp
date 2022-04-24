#include "player_color.h"

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
	for (u32 tint = 0; tint < playerTints.size(); ++tint) {
		for (u32 colorIndex = 0; colorIndex < playerTints[tint].size(); colorIndex++) {
			if (playerTints[tint][colorIndex] == color) {
				tints.push_back(tint);
			}
		}
	}
	return tints;
}

static bool can_use_player_color(u32 color, u32 last_player) {
	for (u32 playerTint : get_player_tints_from_color(color)) {
		if (!can_use_player_tint(playerTint, last_player)) {
			return false;
		}
	}
	return true;
}

static bool can_use_player_tint(u32 color_tint, u32 last_player) {
	if (playerTable[last_player].race < bannedRaceTints.size()) {
		for (u32 banned_tint : bannedRaceTints[playerTable[last_player].race]) {
			if (color_tint == banned_tint) {
				return false;
			}
		}
	}
	if (*CURRENT_TILESET < bannedTilesetTints.size()) {
		for (u32 banned_tint : bannedTilesetTints[*CURRENT_TILESET]) {
			if (color_tint == banned_tint) {
				return false;
			}
		}
	}
	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (player != last_player && (player < last_player || !is_player_randomizable(player))) {
			for (u32 other_tint : get_player_tints_from_color(player_color_list[player])) {
				if (color_tint == other_tint) {
					return false;
				}
			}
		}
	}
	return true;
}

static u32 get_random_tint() {
	return helper::randFunc() % PlayerTintRollCount;
}

static u32 get_random_color_from_tint(u32 tint) {
	return playerTints[tint][helper::randFunc() % playerTints[tint].size()];
}

static u32 get_color_for_personality(u32 player) {
	const u32 race = playerTable[player].race;
	const u32 build = deathTable[UnitId::Spell_DisruptionWeb].player[player] - 1;

	if (build >= 0
		&& personalityColors.size() > race
		&& personalityColors[race].size() > build) {
		u32 size = personalityColors[race][build].size();
		u32 startIndex = helper::randFunc() % size;
		u32 index = startIndex;

		do {
			if (can_use_player_color(personalityColors[race][build][index], player)) {
				return personalityColors[race][build][index];
			}

			index = (index + 1) % size;
		} while (index != startIndex);
	}

	return PlayerColor::Invalid;
}

static bool has_enough_colors() {
	u32 bannedRaceTintsCount = 0;
	for (const std::vector<u32>& race : bannedRaceTints) {
		if (race.size() > bannedRaceTintsCount) {
			bannedRaceTintsCount = race.size();
		}
	}

	if (playerTints.size() - bannedTilesetTints[*CURRENT_TILESET].size() - bannedRaceTintsCount < PLAYABLE_PLAYER_COUNT) {
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