#include "player_color.h"
#include "trigger_system/trigger_system.h"
#include "color_palette.h"

static bool loaded_colors;
u8 tminimap_colors[MinimapColorCount];
u8 tunit_colors[UnitColorCount];

PlayerData player_data;

static std::vector<u32> get_player_tints_from_color(u32 color);
static bool can_use_player_color(u32 color, u32 last_player);
static bool can_use_player_tint(u32 color_tint, u32 last_player);
static u32 get_color_for_personality(u32 player);

static bool player_wants_to_randomize(u32 player) {
	if (player_data.has_custom_color[player]) {
		return false;
	}
	if (player_data.can_randomize_player_colors) {
		bool randomize = player_force_flags[g_player_table[player].force - 1].randomize_start_locations;
		if (g_unit_deaths_table[UnitId::ResourceVespeneGeyser].player[player] & VespeneDeaths_ToggleRandomizeColor) {
			randomize = !randomize;
		}
		return randomize;
	}
	return false;
}

static bool player_can_randomize(u32 player) {
	u32 personality = g_unit_deaths_table[UnitId::Spell_DisruptionWeb].player[player];
	if (g_player_table[player].type == PlayerType::Computer && !personality) {
		return false; // Don't randomize AI players that don't have personality yet
	}
	if (globals.players[player].witness_mode) {
		return false;
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
	return true;
}

static bool should_check_color_overlap(u32 player, u32 other_player) {
	if (player_wants_to_randomize(other_player) && !player_can_randomize(other_player)) {
		return false;
	}
	return other_player != player && !globals.players[other_player].witness_mode && (other_player < player || !player_wants_to_randomize(other_player));
}

static bool can_use_player_color(u32 color, u32 player) {
	for (u32 player_tint : get_player_tints_from_color(color)) {
		if (!can_use_player_tint(player_tint, player)) {
			return false;
		}
	}
	for (u32 banned_color : banned_colors) {
		if (color == banned_color) {
			return false;
		}
	}
	for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
		if (should_check_color_overlap(player, other_player)) {
			if (color == player_data.player_color_list[other_player]) {
				return false;
			}
		}
	}
	return true;
}

#define Square(X) ((X) * (X))

static u32 get_color_delta_srgb(u32 a, u32 b) {
	RGBA a_rgba = init_palette->colors[tminimap_colors[a + ColorSetCount * *g_current_tileset]];
	RGBA b_rgba = init_palette->colors[tminimap_colors[b + ColorSetCount * *g_current_tileset]];

	RGBA delta_rgba = {};
	delta_rgba.a = abs(a_rgba.r - b_rgba.r);
	delta_rgba.g = abs(a_rgba.g - b_rgba.g);
	delta_rgba.b = abs(a_rgba.b - b_rgba.b);

	u32 delta = 0;
	u32 avg_r = (a_rgba.r + b_rgba.r) / 2;
	if (avg_r < 128) {
		delta = 2 * Square(delta_rgba.r) + 4 * Square(delta_rgba.g) + 3 * Square(delta_rgba.b);
	} else {
		delta = 3 * Square(delta_rgba.r) + 4 * Square(delta_rgba.g) + 2 * Square(delta_rgba.b);
	}
	return delta;
}

static u32 get_next_best_color(u32 player) {
	struct ColorMatch {
		u32 color;
		u32 min_distance;
	};

	bool any_distance = false;
	std::vector<ColorMatch> matches; // Auch, use of std is gross. Can we do better?
	for (u32 color = 0; color < PlayerColor::Count; ++color) {
		if (!can_use_player_color(color, player)) continue;
		ColorMatch match = {color, UINT_MAX};
		for (u32 other_player = 0; other_player < PlayerLimit; ++other_player) {
			if (!should_check_color_overlap(player, other_player)) {
				continue;
			}
			u32 other_color = player_data.player_color_list[other_player];
			u32 distance = get_color_delta_srgb(color, other_color);
			if (distance < match.min_distance) {
				match.min_distance = distance;
				any_distance = true;
			}
		}
		matches.push_back(match);
	}
	std::sort(matches.begin(), matches.end(), [](ColorMatch& a, ColorMatch& b) { return a.min_distance > b.min_distance; });


	if (matches.size()) {
		// We are only interested in the better portion of the colors, unless we have no distance data (e.g. first melee randomization)
		u32 cap = any_distance ? matches.size() / 8 : matches.size();
		if (!cap) {
			cap = 1;
		}
		return matches[scbw::random() % cap].color;
	}
	
	scbw::printf("Something went wrong when randomizing colors...");
	return PlayerColor::Red;
}

static u32 get_color_for_personality(u32 player) {
	const u32 race = g_player_table[player].race;
	const u32 build = g_unit_deaths_table[UnitId::Spell_DisruptionWeb].player[player] - 1;
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
	if (player_tints.size() - banned_tileset_tints[*g_current_tileset].size() - banned_race_tints_count < PlayerLimit) {
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
	set_player_color(player, get_next_best_color(player));
}

static void match_color_of(u32 player) {
	for (u32 color = 0; color < ColorSetCount; ++color) {
		bool match = true;
		for (u32 i = 0; i < PlayerLimit; ++i) {
			if (g_player_in_game_colors[player * PlayerLimit + i] != tunit_colors[color * PlayerLimit + i]) {
				match = false;
				break;
			}
		}
		if (match) {
			set_player_color(player, color);
			player_data.has_custom_color[player] = false;
			break;
		}
	}
}

u8 get_player_color_index(u32 player, u32 index) {
	index %= ColorSetSize;
	return g_player_in_game_colors[player * ColorSetSize + index];
}

void set_player_color(u32 player, u32 color) {
	player_data.has_custom_color[player] = true;
	player_data.player_color_list[player] = color;
	g_player_minimap_colors[player] = tminimap_colors[color + ColorSetCount * *g_current_tileset];
	g_player_color_ids[player] = color;
	for (u32 i = 0; i < ColorSetSize; i++) {
		g_player_in_game_colors[player * ColorSetSize + i] = tunit_colors[color * ColorSetSize + i];
	}
}

static void init_player_colors() {
	if (!loaded_colors) {
		loaded_colors = true;
		helper::SBmpLoadImage("game\\tminimap.pcx", 0, tminimap_colors, MinimapColorCount, 0, 0, 0);
		helper::SBmpLoadImage("game\\tunit.pcx", 0, tunit_colors, UnitColorCount, 0, 0, 0);
	}
	memset(&player_data, 0, sizeof(player_data));
}

void load_player_colors() {
	player_data.can_randomize_player_colors = has_enough_colors();
	for (u32 player = 0; player < PlayerLimit; ++player) {
		if (player_wants_to_randomize(player) && player_can_randomize(player)) {
			randomize_player_color(player);
		} else {
			match_color_of(player);
		}
	}
}

static bool can_get_player_name(u32 player) {
	if (!player_data.has_custom_player_name[player]
	&&   g_player_table[player].type == PlayerType::Computer) {
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
			if (strcmp(g_player_name_list->player[i].player_name, name) == 0) { // Will use the player name list, since full names constain player unique color code chars
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

void safe_strcpy(char* to, const char* from, size_t capacity) {
	strncpy(to, from, capacity - 1);
	to[capacity - 1] = 0;
}

void set_player_name(u32 player, char* name, bool prevent_overwrites) {
	if (prevent_overwrites) {
		player_data.has_custom_player_name[player] = true;
	}
	safe_strcpy(g_player_table[player].name, name, 25);
	safe_strcpy(g_player_name_list->player[player].player_name, name, 25);
	safe_strcpy(player_data.full_player_names[player].data, name, FullNameCap);
}

void set_player_force_name(u32 player, char* name) {
	safe_strcpy(g_player_name_list->player[player].force_name, name, 30);
	safe_strcpy(player_data.full_force_names[player].data, name, FullNameCap);
}

static void update_player_name(u32 player) {
	set_player_name(player, g_player_name_list->player[player].player_name, false);
	set_player_force_name(player, g_player_name_list->player[player].force_name);
	if (!can_get_player_name(player)) {
		return;
	}
	u32 race = g_player_table[player].race;
	u32 personality = g_unit_deaths_table[UnitId::Spell_DisruptionWeb].player[player];
	u32 count = get_name_count(race, personality);
	if (count) {
		for (u32 tries = 0; tries < 100; ++tries) {
			u32 random = scbw::rand_between(0, PlayerPesonalityMaxNames - 1);
			char* name = player_personality_names[race][personality][random];
			if (is_valid_name(player, name)) {
				set_player_name(player, name, true);
				return;
			}
		}
	}
	set_player_name(player, "Computer", false);
}

void load_player_names() {
	for (u32 i = 0; i < PlayerLimit; ++i) {
		update_player_name(i);
	}
}