#include "trigger_parser.h"
#include "trigger_vars.h"

static bool is_normal_space(char c) {
	return c == ' ' || c == '\t';
}

static bool is_any_space(char c) {
	return isspace(c);
}

static bool is_separator(char c) {
	return c == '|' || c == '&';
}

static u32 is_newline(char c) {
	if (c == '\r' || c == '\n') {
		return 2;
	}
	return 0;
}

static bool is_word_char(char c) {
	return isalnum(c) || c == '.' || c == '_' || c == '*';
}

static void parser_clear(Parser* parser) {
	memset(&parser->token, 0, sizeof(parser->token));
}

static bool parser_skip_whitespace(Parser* parser) {
	parser_clear(parser);
	while (*parser->at && is_any_space(*parser->at)) {
		if (is_newline(*parser->at)) {
			return false;
		}
		++parser->at;
	}
	return true;
}

static bool parser_skip_non_word_chars(Parser* parser) {
	parser_clear(parser);
	while (*parser->at && !is_word_char(*parser->at)) {
		if (is_newline(*parser->at)) {
			return false;
		}
		++parser->at;
	}
	return true;
}

static bool parser_get_negation(Parser* parser) {
	if (!parser_skip_whitespace(parser)) {
		return false;
	}
	return parser_get_char(parser, '!');
}

static char parser_get_separator(Parser* parser) {
	if (!parser_skip_whitespace(parser)) {
		return 0;
	}

	if (is_separator(*parser->at)) {
		return *parser->at++;
	}
	return 0;
}

static char parser_get_relative_op(Parser* parser) {
	if (!parser_skip_whitespace(parser)) {
		return 0;
	}

	switch (*parser->at) {
		case '-':
		case '+':
		case '~':
			return *parser->at++;
	}
	return 0;
}

char* parser_marker(Parser* parser) {
	return parser->at;
}

void parser_unwind(Parser* parser, char* marker) {
	parser_clear(parser);
	parser->at = marker;
}

bool parser_get_char(Parser* parser, char c) {
	if (*parser->at == c) {
		++parser->at;
		return true;
	}
	return false;
}

Token parser_get_word(Parser* parser) {
	if (!parser_skip_non_word_chars(parser)) {
		return {};
	}

	while (is_word_char(*parser->at)) {
		if (parser->token.size < MaxToken) {
			parser->token.data[parser->token.size++] = *parser->at;
		}
		++parser->at;
	}

	return parser->token;
}

bool parser_get_keyword(Parser* parser, char* keyword) {
	char* marker = parser_marker(parser);
	parser_get_word(parser);
	if (parser_eq(parser, keyword)) {
		return true;
	}
	parser_unwind(parser, marker);
	return false;
}

bool parser_get_command(Parser* parser) {
	if (!parser->parsed_first_command) {
		parser->parsed_first_command = true;
	} else {
		while (*parser->at && !is_newline(*parser->at)) {
			++parser->at;
		}
	}

	while (*parser->at && is_newline(*parser->at)) {
		++parser->at;
	}

	return parser_get_word(parser).size;
}

Token parser_get_string(Parser* parser) {
	if (!parser_skip_whitespace(parser)) {
		return parser->token;
	}

	bool is_string = false;
	if (parser_get_char(parser, '"')) {
		while (*parser->at && *parser->at != '"' && !is_newline(*parser->at)) {
			if (parser->token.size < MaxToken) {
				parser->token.data[parser->token.size++] = *parser->at;
			}
			++parser->at;
		}
		parser_get_char(parser, '"');
	} else {
		while (*parser->at && !is_any_space(*parser->at) && !is_separator(*parser->at)) {
			if (parser->token.size < MaxToken) {
				parser->token.data[parser->token.size++] = *parser->at;
			}
			++parser->at;
		}
	}

	return parser->token;
}

TriggerVar* parser_get_var(Parser* parser) {
	if (parser_get_char(parser, '$')) {
		Token name = parser_get_word(parser);
		TriggerVar* var = find_trigger_var(TriggerVarType_Var, name.data);
		return var;
	}
	return 0;
}

s32 parser_get_s32(Parser* parser, s32 default_value) {
	TriggerVar* var = parser_get_var(parser);
	if (var) {
		return var->value;
	}

	parser_get_word(parser);
	if (parser_eqi(parser, "true")) {
		return 1;
	}
	if (parser_eqi(parser, "false")) {
		return 0;
	}

	char* end = 0;
	s32 value = strtol(parser->token.data, &end, 10);
	if (end != parser->token.data + parser->token.size) {
		return default_value;
	}
	return value;
}

f32 parser_get_f32(Parser* parser, f32 default_value) {
	parser_get_word(parser);

	char* end = 0;
	f32 value = strtof(parser->token.data, &end);
	if (end != parser->token.data + parser->token.size) {
		return default_value;
	}
	return value;
}

enum TimeType {
	TimeType_Hours,
	TimeType_Minutes,
	TimeType_Seconds,
	TimeType_Milliseconds,
	TimeType_Ticks,
	TimeType_Count,
};

f32 parser_get_time_seconds(Parser* parser) {
	if (!parser_skip_whitespace(parser)) {
		return false;
	}

	bool add_to_total = false;
	f32 total_time = 0;
	f32 time = 0;
	u32 time_type = TimeType_Seconds;

	u32 buffer_index = 0;
	char buffer[MaxToken + 1] = {};
	while (*parser->at && !is_any_space(*parser->at) && !is_separator(*parser->at)) {
		if (isdigit(*parser->at) || *parser->at == '.') {
			if (buffer_index < MaxToken) {
				buffer[buffer_index++] = *parser->at;
			}
		} else if (*parser->at == 'h') {
			time_type = TimeType_Hours;
			add_to_total = true;
		} else if (*parser->at == 'm') {
			if (parser->at[1] == 's') {
				time_type = TimeType_Milliseconds;
				add_to_total = true;

				if (parser->token.size < MaxToken) {
					parser->token.data[parser->token.size++] = *parser->at;
				}
				++parser->at;
			} else {
				time_type = TimeType_Minutes;
				add_to_total = true;
			}
		} else if (*parser->at == 's') {
			time_type = TimeType_Seconds;
			add_to_total = true;
		} else if (*parser->at == 't') {
			time_type = TimeType_Ticks;
			add_to_total = true;
		}

		if (add_to_total) {
			add_to_total = false;
			time = strtof(buffer, 0);
			switch (time_type) {
				case TimeType_Hours: {
					total_time += time * 60 * 60 * 24;
				} break;
				case TimeType_Minutes: {
					total_time += time * 60 * 24;
				} break;
				case TimeType_Seconds: {
					total_time += time;
				} break;
				case TimeType_Milliseconds: {
					total_time += time / 1000;
				} break;
				case TimeType_Ticks: {
					total_time += time / 24;
				} break;
			}
			buffer_index = 0;
			memset(buffer, 0, sizeof(buffer));
			time = 0;
		}

		if (parser->token.size < MaxToken) {
			parser->token.data[parser->token.size++] = *parser->at;
		}
		++parser->at;
	}

	// add seconds by default if anything without type was left
	if (buffer_index) {
		time = strtof(buffer, 0);
		total_time += ceil(time);
	}
	return total_time;
}

u32 parser_get_time_ticks(Parser* parser) {
	return ceil(parser_get_time_seconds(parser) * 24);
}

s32 parser_get_var_value(Parser* parser, bool is_timer) {
	char* marker = parser_marker(parser);
	if (parser_get_keyword(parser, "random")) {
		s32 min_value = is_timer ? parser_get_time_ticks(parser) : parser_get_s32(parser);
		s32 max_value = is_timer ? parser_get_time_ticks(parser) : parser_get_s32(parser);
		return scbw::rand_between(min_value, max_value);
	}
	return is_timer ? parser_get_time_ticks(parser) : parser_get_s32(parser);
}

Area parser_get_area(Parser* parser, Area default_value) {
	Area area = {};
	if (!parser_skip_whitespace(parser)) {
		return area;
	}

	if (parser_get_char(parser, '(')) {
		area.flags = ~0;
		area.box.left = parser_get_s32(parser);
		area.box.top  = parser_get_s32(parser);
		{
			u32 relative_op = parser_get_relative_op(parser);
			u32 w = parser_get_s32(parser);
			if (relative_op == '+') {
				area.box.right = area.box.left + w;
			} else if (relative_op == '-') {
				area.box.right = area.box.left;
				area.box.left -= w;
			} else if (relative_op == '~') {
				area.box.left  = area.box.left - w / 2;
				area.box.right = area.box.left + w;
			} else {
				area.box.right = w;
			}
		}
		{
			u32 relative_op = parser_get_relative_op(parser);
			u32 h = parser_get_s32(parser);
			if (relative_op == '+') {
				area.box.bottom = area.box.top + h;
			} else if (relative_op == '-') {
				area.box.bottom = area.box.top;
				area.box.top   -= h;
			} else if (relative_op == '~') {
				area.box.top    = area.box.top - h / 2;
				area.box.bottom = area.box.top + h;
			} else {
				area.box.bottom = h;
			}
		}

		parser_get_char(parser, ')');
		return area;
	}

	char* marker = parser_marker(parser);
	Token string = parser_get_string(parser);
	if (parser_eq(parser, "*")) {
		area.box = g_location_table[63].dimensions;
		area.flags = ~g_location_table[63].flags;
		return area;
	}
	for (u32 i = 0; i < 256; ++i) {
		char* name = helper::get_chk_string(g_location_table[i].string_id);
		if (name && name[0] && strcmp(name, string.data) == 0) {
			area.box = g_location_table[i].dimensions;
			area.flags = ~g_location_table[i].flags;
			return area;
		}
	}
	parser_unwind(parser, marker); // string wasn't valid, let's unwind and try an int

	s32 i = parser_get_s32(parser, -1);
	if (i >= 0 && i < 256) {
		area.box = g_location_table[i].dimensions;
		area.flags = ~g_location_table[i].flags;
		return area;
	}

	return default_value;
}

bool check_area(Area area, Point16 position, bool air) {
	if (position.x >= area.box.left && position.x <= area.box.right &&
		position.y >= area.box.top  && position.y <= area.box.bottom) {
		ActiveTile* tile = scbw::get_active_tile_at_coords(position.x, position.y);
		u32 height = tile->ground_height / 2;
		if (height == 0) {
			// Low
			return air ? area.flags & LocationFlags_LowAir : area.flags & LocationFlags_LowGround;
		} else if (height == 1) {
			// Mid
			return air ? area.flags & LocationFlags_MidAir : area.flags & LocationFlags_MidGround;
		} else if (height == 2) {
			// High
			return air ? area.flags & LocationFlags_HighAir : area.flags & LocationFlags_HighGround;
		}
		return false;
	}
	return false;
}

Point16 parser_get_position(Parser* parser, Point16 default_value) {
	if (!parser_skip_whitespace(parser)) {
		return {};
	}

	if (parser_get_char(parser, '(')) {
		Point16 pos = {};
		pos.x = parser_get_s32(parser);
		pos.y = parser_get_s32(parser);
		parser_get_char(parser, ')');
		return pos;
	}

	char* marker = parser_marker(parser);
	Token string = parser_get_string(parser);
	if (parser_eq(parser, "*")) {
		return box_center(g_location_table[63].dimensions);
	}
	for (u32 i = 0; i < 256; ++i) {
		char* name = helper::get_chk_string(g_location_table[i].string_id);
		if (name && name[0] && strcmp(name, string.data) == 0) {
			return box_center(g_location_table[i].dimensions);
		}
	}
	parser_unwind(parser, marker); // string wasn't valid, let's unwind and try an int

	s32 i = parser_get_s32(parser);
	if (i >= 0 && i < 256) {
		return box_center(g_location_table[i].dimensions);
	}

	return default_value;
}

s32 parser_get_enum(Parser* parser, const char** enum_names, u32 count, s32 default_value) {
	Token token = parser_get_string(parser);

	for (u32 i = 0; i < count; ++i) {
		if (stricmp(token.data, enum_names[i]) == 0) {
			return i;
		}
	}

	return default_value;
}

u32 parser_get_player(Parser* parser, bool allow_neutral) {
	s32 player = parser_get_s32(parser, -1);
	if (player >= 0) {
		if (player <= 8 || (allow_neutral && player == 12)) {
			return player;
		}
		if (player == 13) {
			return *g_current_trigger_player_id;
		}
	}
	return 0;
}

static PlayerBitset player_bitset_combine(PlayerBitset a, PlayerBitset b) {
	for (u32 i = 0; i < PlayerLimit; ++i) {
		a.set[i] = a.set[i] || b.set[i];
	}
	return a;
}

static void player_bitset_set(PlayerBitset* bitset, u32 i, bool value, bool mask, bool negate) {
	if (negate) {
		value = !value;
	}
	if (value) {
		if (!mask) {
			bitset->set[i] = true;
		}
	} else if (mask) {
		bitset->set[i] = false;
	}
}

PlayerBitset parser_get_player_bitset(Parser* parser) {
	bool mask = false;
	PlayerBitset bitset = {};
	PlayerBitset current_bitset = {};
	while (true) {
		bool negate = parser_get_negation(parser);
		char* marker = parser_marker(parser);
		s32 value = parser_get_s32(parser);
		if (parser_eq(parser, "*")) {
			for (u32 i = 0; i < PlayerLimit; ++i) {
				player_bitset_set(&current_bitset, i, true, mask, negate);
			}
		} else if (value == 14) {
			for (u32 i = 0; i < PlayerLimit; ++i) {
				player_bitset_set(&current_bitset, i, scbw::is_allied_to(*g_current_trigger_player_id, i), mask, negate);
			}
		} else if (value == 15) {
			for (u32 i = 0; i < PlayerLimit; ++i) {
				player_bitset_set(&current_bitset, i, !scbw::is_allied_to(*g_current_trigger_player_id, i), mask, negate);
			}
		} else if (value == 16) {
			// wasn't used in aise, but we can do something here later?
		} else if (value == 17) {
			for (u32 i = 0; i < PlayerLimit; ++i) {
				player_bitset_set(&current_bitset, i, true, mask, negate);
			}
		} else if (value == 18) {
			for (u32 i = 0; i < PlayerLimit; ++i) {
				player_bitset_set(&current_bitset, i, g_player_table[i].force == 1, mask, negate);
			}
		} else if (value == 19) {
			for (u32 i = 0; i < PlayerLimit; ++i) {
				player_bitset_set(&current_bitset, i, g_player_table[i].force == 2, mask, negate);
			}
		} else if (value == 20) {
			for (u32 i = 0; i < PlayerLimit; ++i) {
				player_bitset_set(&current_bitset, i, g_player_table[i].force == 3, mask, negate);
			}
		} else if (value == 21) {
			for (u32 i = 0; i < PlayerLimit; ++i) {
				player_bitset_set(&current_bitset, i, g_player_table[i].force == 4, mask, negate);
			}
		} else {
			parser_unwind(parser, marker);
			s32 player = parser_get_player(parser);
			if (player) {
				player -= 1;
				for (u32 i = 0; i < PlayerLimit; ++i) {
					player_bitset_set(&current_bitset, i, i == player, mask, negate);
				}
			}
		}

		char separator = parser_get_separator(parser);
		if (separator == '&') {
			mask = true;
		} else if (separator == '|') {
			mask = false;
			bitset = player_bitset_combine(bitset, current_bitset);
			current_bitset = {};
		} else {
			bitset = player_bitset_combine(bitset, current_bitset);
			break;
		}
	}
	return bitset;
}

static const char* remove_prefix(const char* string, char* prefix) {
	u32 len = strlen(prefix);
	if (strnicmp(string, prefix, len) == 0) {
		string += len;
		for (const char* c = string; *c == ' '; ++c) {
			++string;
		}
	}
	return string;
}

Token token_from_str(const char* str) {
	Token token = {};
	for (const char* c = str; *c; ++c) {
		if (token.size < MaxToken) {
			token.data[token.size++] = *c;
		} else {
			break;
		}
	}
	return token;
}

Token get_short_unit_name(u32 unit_id) {
	const char* name = Unit::get_name(unit_id);
	name = remove_prefix(name, "terran");
	name = remove_prefix(name, "protoss");
	name = remove_prefix(name, "zerg");
	return token_from_str(name);
}

static char tolazy(char c) {
	if (c == ' ') {
		return '_';
	}
	return tolower(c);
}

static bool str_equals_lazy(const char* a, const char* b) {
	while (*a++ && *b++) {
		if (tolazy(*a) != tolazy(*b)) {
			return false;
		}
	}
	return *a == *b;
}

u32 parser_get_unit_id(Parser* parser) {
	char* marker = parser_marker(parser);
	Token token = parser_get_string(parser);
	for (u32 i = 0; i < UnitId::Count; ++i) {
		if (str_equals_lazy(token.data, get_short_unit_name(i).data)) {
			return i;
		}
	}
	parser_unwind(parser, marker);
	return parser_get_s32(parser, UnitId::None);
}

static UnitIdBitset unit_id_bitset_combine(UnitIdBitset a, UnitIdBitset b) {
	for (u32 i = 0; i < UnitId::Count; ++i) {
		a.set[i] = a.set[i] || b.set[i];
	}
	return a;
}

static void unit_id_bitset_set(UnitIdBitset* bitset, u32 i, bool value, bool mask, bool negate) {
	if (negate) {
		value = !value;
	}
	if (value) {
		if (!mask) {
			bitset->set[i] = true;
		}
	} else if (mask) {
		bitset->set[i] = false;
	}
}

UnitIdBitset parser_get_unit_id_bitset(Parser* parser) {
	bool mask = false;
	UnitIdBitset bitset = {};
	UnitIdBitset current_bitset = {};
	while (true) {
		bool negate = parser_get_negation(parser);
		char* marker = parser_marker(parser);
		parser_get_string(parser);
		if (parser_eq(parser, "*")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, true, mask, negate);
			}
		} else if (parser_eqi(parser, "men")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, units_dat::group_flags[i].is_men, mask, negate);
			}
		} else if (parser_eqi(parser, "buildings")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, units_dat::group_flags[i].is_building, mask, negate);
			}
		} else if (parser_eqi(parser, "factories")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, units_dat::group_flags[i].is_factory, mask, negate);
			}
		} else if (parser_eqi(parser, "tech")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, scbw::unit_id_is_tech(i), mask, negate);
			}
		} else if (parser_eqi(parser, "tech_tier")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, scbw::unit_id_is_tech_tier_structure(i), mask, negate);
			}
		} else if (parser_eqi(parser, "terran")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, scbw::unit_id_race_check(i, RaceId::Terran), mask, negate);
			}
		} else if (parser_eqi(parser, "protoss")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, scbw::unit_id_race_check(i, RaceId::Protoss), mask, negate);
			}
		} else if (parser_eqi(parser, "zerg")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, scbw::unit_id_race_check(i, RaceId::Zerg), mask, negate);
			}
		} else if (parser_eqi(parser, "combatants")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				bool is_combatant = false;
				u32 subunit_id = units_dat::subunit[i];
				if (units_dat::ground_weapon[i] != WeaponId::None || units_dat::ground_weapon[subunit_id] != WeaponId::None) {
					is_combatant = true;
				}
				if (units_dat::air_weapon[i] != WeaponId::None || units_dat::air_weapon[subunit_id] != WeaponId::None) {
					is_combatant = true;
				}
				if (units_dat::base_property[i] & UnitProperty::Spellcaster && i != UnitId::TerranComsatStation) {
					is_combatant = true;
				}
				unit_id_bitset_set(&current_bitset, i, is_combatant, mask, negate);
			}
		} else if (parser_eqi(parser, "transports")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, scbw::unit_id_is_transport(i), mask, negate);
			}
		} else if (parser_eqi(parser, "townhalls")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, scbw::unit_id_is_town_center(i), mask, negate);
			}
		} else if (parser_eqi(parser, "depots")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, units_dat::base_property[i] & UnitProperty::ResourceDepot, mask, negate);
			}
		} else if (parser_eqi(parser, "nodes")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, units_dat::base_property[i] & UnitProperty::ResourceContainer, mask, negate);
			}
		} else if (parser_eqi(parser, "factional")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, scbw::unit_id_is_factional(i), mask, negate);
			}
		} else if (parser_eqi(parser, "workers")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(&current_bitset, i, units_dat::base_property[i] & UnitProperty::Worker, mask, negate);
			}
		} else {
			parser_unwind(parser, marker);
			s32 unit_id = parser_get_unit_id(parser);
			if (unit_id != UnitId::None) {
				for (u32 i = 0; i < UnitId::Count; ++i) {
					unit_id_bitset_set(&current_bitset, i, i == unit_id, mask, negate);
				}
			}
		}

		char separator = parser_get_separator(parser);
		if (separator == '&') {
			mask = true;
		} else if (separator == '|') {
			mask = false;
			bitset = unit_id_bitset_combine(bitset, current_bitset);
			current_bitset = {};
		} else {
			bitset = unit_id_bitset_combine(bitset, current_bitset);
			break;
		}
	}
	return bitset;
}

QuotaType parser_get_quota_type(Parser* parser, QuotaType default_type) {
	if (!parser_skip_whitespace(parser)) {
		return default_type;
	}

	if (*parser->at == '>' || *parser->at == '<' || *parser->at == '!' || *parser->at == '=' || *parser->at == '*') {
		char* marker = parser_marker(parser);
		char op = *parser->at++;

		bool equals = parser_get_char(parser, '=');
		switch (op) {
			case '>': return equals ? QuotaType_GreaterOrEqual : QuotaType_Greater; break;
			case '<': return equals ? QuotaType_LessOrEqual : QuotaType_Less; break;
			case '=': return QuotaType_Equal; break;
			case '!': return QuotaType_NotEqual; break;
			case '*': {
				// We pop since we don't want next parser_get to consume next argument
				parser_unwind(parser, marker);
				return QuotaType_Anything;
			} break;
		}
	}
	return default_type;
}

bool check_quota(QuotaType type, s32 quota, s32 value) {
	switch (type) {
		case QuotaType_Greater:			return value >  quota;
		case QuotaType_Less:			return value <  quota;
		case QuotaType_GreaterOrEqual:	return value >= quota;
		case QuotaType_LessOrEqual:		return value <= quota;
		case QuotaType_Equal:			return value == quota; 
		case QuotaType_NotEqual:		return value != quota; 
		case QuotaType_Anything:		return true; 
	}
	return false;
}

bool parser_has_recolor(Parser* parser) {
	if (!parser_skip_whitespace(parser)) {
		return false;
	}
	return *parser->at == '(';
}

Recolor parser_get_recolor(Parser* parser) {
	Recolor recolor = default_recolor();
	if (!parser_skip_whitespace(parser)) {
		return recolor;
	}

	if (parser_get_char(parser, '(')) {
		recolor.r          = parser_get_f32(parser, recolor.r);
		recolor.g          = parser_get_f32(parser, recolor.g);
		recolor.b          = parser_get_f32(parser, recolor.b);
		recolor.gamma      = parser_get_f32(parser, recolor.gamma);
		recolor.saturation = parser_get_f32(parser, recolor.saturation);
		parser_get_char(parser, ')');
	}
	return recolor;
}

RecolorSet parser_get_recolor_set(Parser* parser) {
	RecolorSet set = {};
	while (set.count < RecolorCap) {
		if (!parser_has_recolor(parser)) {
			break;
		}
		set.recolors[set.count++] = parser_get_recolor(parser);
	}
	if (!set.count) {
		set = default_recolor_set();
	}
	return set;
}

Point16 box_center(Box32 box) {
	Point16 point = {};
	point.x = box.left + (box.right  - box.left) / 2;
	point.y = box.top  + (box.bottom - box.top)  / 2;
	return point;
}

bool token_eq(Token* token, char* string) {
	if (!string) return false;
	return strcmp(token->data, string) == 0;
}

bool token_eqi(Token* token, char* string) {
	if (!string) return false;
	return stricmp(token->data, string) == 0;
}

bool token_eql(Token* token, char* string) {
	if (!string) return false;
	return str_equals_lazy(token->data, string);
}

bool parser_eq(Parser* parser, char* string) {
	return token_eq(&parser->token, string);
}

bool parser_eqi(Parser* parser, char* string) {
	return token_eqi(&parser->token, string);
}