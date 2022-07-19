#include "player_color.h"
#include "trigger_system/trigger_system.h"
#include "color_palette.h"

static bool loaded_colors;
static bool can_randomize_player_colors;
static bool has_custom_color[PlayerLimit];
static u8 tminimap_colors[MinimapColorCount];
static u8 tunit_colors[UnitColorCount];
static u8 player_color_list[PLAYABLE_PLAYER_COUNT];

static std::vector<u32> get_player_tints_from_color(u32 color);
static bool can_use_player_color(u32 color, u32 last_player);
static bool can_use_player_tint(u32 color_tint, u32 last_player);
static u32 get_random_tint();
static u32 get_random_color_from_tint(u32 tint);
static u32 get_color_for_personality(u32 player);

static bool player_wants_to_randomize(u32 player) {
	if (has_custom_color[player]) {
		return false;
	}
	if (can_randomize_player_colors) {
		bool randomize = player_force_flags[g_player_table[player].force - 1].randomize_start_locations;
		if (g_death_table[UnitId::ResourceVespeneGeyser].player[player] & VespeneDeaths_ToggleRandomizeColor) {
			randomize = !randomize;
		}
		return randomize;
	}
	return false;
}

static bool player_can_randomize(u32 player) {
	u32 personality = g_unit_deaths[UnitId::Spell_DisruptionWeb].player[player];
	if (g_player_table[player].type == PlayerType::Computer && !personality) {
		return false; // Don't randomize AI players that don't have personality yet
	}
	return true;

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

static bool can_use_player_tint(u32 color_tint, u32 player) {
	if (g_player_table[player].race < banned_race_tints.size()) {
		for (u32 banned_tint : banned_race_tints[g_player_table[player].race]) {
			if (color_tint == banned_tint) {
				return false;
			}
		}
	}
	if (*g_current_tileset < banned_tileset_tints.size()) {
		for (u32 banned_tint : banned_tileset_tints[*g_current_tileset]) {
			if (color_tint == banned_tint) {
				return false;
			}
		}
	}
	for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
		if (other_player != player && (other_player < player || !player_wants_to_randomize(other_player))) {
			if (player_wants_to_randomize(other_player) && !player_can_randomize(other_player)) {
				continue;
			}
			for (u32 other_tint : get_player_tints_from_color(player_color_list[other_player])) {
				if (color_tint == other_tint) {
					return false;
				}
			}
		}
	}
	return true;
}

static u32 get_color_delta(u32 a, u32 b) {
	RGBA a_rgba = init_palette->colors[tminimap_colors[a + ColorSetCount * *g_current_tileset]];
	RGBA b_rgba = init_palette->colors[tminimap_colors[b + ColorSetCount * *g_current_tileset]];

	u32 delta = 0;
	delta += abs(a_rgba.r - b_rgba.r);
	delta += abs(a_rgba.g - b_rgba.g);
	delta += abs(a_rgba.b - b_rgba.b);
	return delta;
}

static bool can_use_player_color(u32 color, u32 player) {
	for (u32 player_tint : get_player_tints_from_color(color)) {
		if (!can_use_player_tint(player_tint, player)) {
			return false;
		}
	}
	for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
		if (other_player != player && (other_player < player || !player_wants_to_randomize(other_player))) {
			if (player_wants_to_randomize(other_player) && !player_can_randomize(other_player)) {
				continue;
			}
			u32 delta = get_color_delta(color, player_color_list[other_player]);
			if (delta < 128) {
				return false;
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
	const u32 race = g_player_table[player].race;
	const u32 build = g_death_table[UnitId::Spell_DisruptionWeb].player[player] - 1;
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
	if (player_tints.size() - banned_tileset_tints[*g_current_tileset].size() - banned_race_tints_count < PLAYABLE_PLAYER_COUNT) {
		return false;
	}
	return true;
}

static void randomize_player_color(u32 player) {
	if (g_player_table[player].type == PlayerType::Computer) {
		u32 color = get_color_for_personality(player);
		if (color != PlayerColor::Invalid) {
			set_player_color(player, color);
			return;
		}
	}
	u32 color = 0;
	do {
		u32 tint = 0;
		do {
			tint = get_random_tint();
		} while (!can_use_player_tint(tint, player));
		color = get_random_color_from_tint(tint);
	} while (!can_use_player_color(color, player));
	set_player_color(player, color);
}

static void match_color_of(u32 player) {
	for (u32 color = 0; color < ColorSetCount; ++color) {
		bool match = true;
		for (u32 i = 0; i < PLAYABLE_PLAYER_COUNT; ++i) {
			if (g_player_in_game_colors[player * PLAYABLE_PLAYER_COUNT + i] != tunit_colors[color * PLAYABLE_PLAYER_COUNT + i]) {
				match = false;
				break;
			}
		}
		if (match) {
			set_player_color(player, color);
			has_custom_color[player] = false;
			break;
		}
	}
}

void set_player_color(u32 player, u32 color) {
	has_custom_color[player] = true;
	player_color_list[player] = color;
	g_player_minimap_colors[player] = tminimap_colors[color + ColorSetCount * *g_current_tileset];
	for (u32 i = 0; i < PLAYABLE_PLAYER_COUNT; i++) {
		g_player_in_game_colors[player * PLAYABLE_PLAYER_COUNT + i] = tunit_colors[color * ColorSetSize + i];
	}
}

static void init_player_colors() {
	if (!loaded_colors) {
		loaded_colors = true;
		helper::SBmpLoadImage("game\\tminimap.pcx", 0, tminimap_colors, MinimapColorCount, 0, 0, 0);
		helper::SBmpLoadImage("game\\tunit.pcx", 0, tunit_colors, UnitColorCount, 0, 0, 0);
	}
	memset(has_custom_color, 0, sizeof(has_custom_color));
}

void load_player_colors() {
	can_randomize_player_colors = has_enough_colors();
	for (u32 player = 0; player < PLAYABLE_PLAYER_COUNT; ++player) {
		if (player_wants_to_randomize(player) && player_can_randomize(player)) {
			randomize_player_color(player);
		} else {
			match_color_of(player);
		}
	}
}

static bool can_get_player_name(u32 player) {
	if (!has_custom_player_name[player]
	&&   g_player_table[player].type == PlayerType::Computer)
	{
		return true;
	}
	return false;
}

static bool is_valid_name(u32 player, char* name) {
	if (!name) {
		return false;
	}
	for (u32 i = 0; i < PlayerLimit; ++i) {
		if (i != player && can_get_player_name(i)) {
			if (strcmp(g_player_name_list->player[i].player_name, name) == 0) {
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

void set_player_name(u32 player, char* name) {
	has_custom_player_name[player] = true;
	strncpy(g_player_name_list->player[player].player_name, name, 24);
	g_player_name_list->player[player].player_name[24] = 0;
}

void set_player_force_name(u32 player, char* name) {
	strncpy(g_player_name_list->player[player].force_name, name, 29);
	g_player_name_list->player[player].force_name[29] = 0;
}

static void update_player_name(u32 player) {
	if (!can_get_player_name(player)) {
		return;
	}
	u32 race = g_player_table[player].race;
	u32 personality = g_unit_deaths[UnitId::Spell_DisruptionWeb].player[player];
	u32 count = get_name_count(race, personality);
	if (count) {
		for (u32 tries = 0; tries < 20; ++tries) {
			u32 random = scbw::rand_between(0, PlayerPesonalityMaxNames - 1);
			char* name = player_personality_names[race][personality][random];
			if (is_valid_name(player, name)) {
				set_player_name(player, name);
				return;
			}
		}
	}
	set_player_name(player, "Computer");
	has_custom_player_name[player] = false; // Here is where we want to remove this
}

void load_player_names() {
	for (u32 i = 0; i < PlayerLimit; ++i) {
		update_player_name(i);
	}
}