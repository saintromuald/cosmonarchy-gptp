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

void Parser::clear_token() {
	m_token = {};
}

bool Parser::skip_whitespace() {
	clear_token();
	while (*m_at && is_any_space(*m_at)) {
		if (is_newline(*m_at)) {
			return false;
		}
		++m_at;
	}
	return true;
}

bool Parser::skip_non_word_chars() {
	clear_token();
	while (*m_at && !is_word_char(*m_at)) {
		if (is_newline(*m_at)) {
			return false;
		}
		++m_at;
	}
	return true;
}

bool Parser::get_negation() {
	if (!skip_whitespace()) {
		return false;
	}
	return get_char('!');
}

char Parser::get_separator() {
	if (!skip_whitespace()) {
		return 0;
	}

	if (is_separator(*m_at)) {
		return *m_at++;
	}
	return 0;
}

char Parser::get_relative_op() {
	if (!skip_whitespace()) {
		return 0;
	}

	switch (*m_at) {
		case '-':
		case '+':
		case '~':
			return *m_at++;
	}
	return 0;
}

Parser::Parser(const char* at) : m_start(at), m_at(at) {
}

ParserMarker Parser::marker() {
	return {m_at, m_token};
}

void Parser::unwind(const ParserMarker& marker) {
	m_at = marker.at;
	m_token = marker.token;
}

bool Parser::get_char(char c) {
	if (*m_at == c) {
		++m_at;
		return true;
	}
	return false;
}

void Parser::start_token() {
	m_token.sv = {m_at, 0};
}

void Parser::expand_token() {
	if (m_token.sv.data()) {
		m_token.sv = {m_token.sv.data(), m_token.sv.size() + 1};
	} else {
		assert(false);
	}
	m_at++;
}

Token Parser::eat_word() {
	start_token();
	while (is_word_char(*m_at)) {
		expand_token();
	}
	return m_token;
}

Token Parser::get_word(bool is_command) {
	if (!skip_non_word_chars()) {
		return {};
	}

	if (is_command) {
		m_command_start_pos = m_at - m_start;
	}
	return eat_word();
}

void Parser::load_command_pos(u32 position) {
	if (position) {
		m_should_load_pos = true;
		m_at = m_start + position;
	}
}

bool Parser::get_command() {
	if (m_should_load_pos) {
		m_should_load_pos = false;
		m_parsed_first_command = true;
		return !eat_word().sv.empty();
	}

	// Skip to end of line
	if (!m_parsed_first_command) {
		m_parsed_first_command = true;
	} else {
		while (*m_at && !is_newline(*m_at)) {
			++m_at;
		}
	}

	// Skip newlines
	while (*m_at && is_newline(*m_at)) {
		++m_at;
	}

	return !get_word(true).sv.empty();
}

bool Parser::get_keyword(const char* keyword) {
	const auto marker = this->marker();
	get_word();
	if (equals(keyword)) {
		return true;
	}
	unwind(marker);
	return false;
}

Token Parser::get_string() {
	if (!skip_whitespace()) {
		return m_token;
	}

	if (get_char('"')) {
		start_token();
		while (*m_at && *m_at != '"' && !is_newline(*m_at)) {
			expand_token();
		}
		get_char('"');
	} else {
		start_token();
		while (*m_at && !is_any_space(*m_at) && !is_separator(*m_at) && *m_at != '"') {
			expand_token();
		}
	}

	return m_token;
}

Token Parser::get_identifier() {
	if (!skip_whitespace()) {
		return m_token;
	}

	start_token();
	while (*m_at && !is_any_space(*m_at) && !is_separator(*m_at) && *m_at != '"') {
		expand_token();
	}

	return m_token;
}

TriggerVar* Parser::maybe_get_var() {
	if (get_char('$')) {
		Token name = get_word();
		TriggerVar* var = g_trigger_vars.find(TriggerVarType_Var, name.sv);
		return var;
	}
	return 0;
}

s32 Parser::get_s32(s32 default_value) {
	TriggerVar* var = maybe_get_var();
	if (var) {
		return var->value();
	}

	get_word();
	if (iequals("true")) {
		return 1;
	}
	if (iequals("false")) {
		return 0;
	}

	const auto value = sv_convert_to<s32>(m_token.sv);
	return value ? *value : default_value;
}

f32 Parser::get_f32(f32 default_value) {
	get_word();

	const auto value = sv_convert_to<f32>(m_token.sv);
	return value ? *value : default_value;
}

enum TimeType {
	TimeType_Hours,
	TimeType_Minutes,
	TimeType_Seconds,
	TimeType_Milliseconds,
	TimeType_Ticks,
	TimeType_Count,
};

f32 Parser::get_time_seconds() {
	if (!skip_whitespace()) {
		return false;
	}

	bool add_to_total = false;
	f32 total_time = 0;
	f32 time = 0;
	u32 time_type = TimeType_Seconds;

	start_token();
	while (*m_at && !is_any_space(*m_at) && !is_separator(*m_at)) {
		if (isdigit(*m_at) || *m_at == '.') {
			expand_token();
			continue;
		} else if (*m_at == 'h') {
			time_type = TimeType_Hours;
			add_to_total = true;
		} else if (*m_at == 'm') {
			if (m_at[1] == 's') {
				time_type = TimeType_Milliseconds;
				add_to_total = true;

				expand_token();
			} else {
				time_type = TimeType_Minutes;
				add_to_total = true;
			}
		} else if (*m_at == 's') {
			time_type = TimeType_Seconds;
			add_to_total = true;
		} else if (*m_at == 't') {
			time_type = TimeType_Ticks;
			add_to_total = true;
		}

		if (add_to_total) {
			add_to_total = false;
			time = sv_convert_to<f32>(m_token.sv).value_or(0);
			switch (time_type) {
				case TimeType_Hours: {
					total_time += time * 60 * 60;
				} break;
				case TimeType_Minutes: {
					total_time += time * 60;
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
			start_token();
			time = 0;
		}

		expand_token();
	}

	// add seconds by default if anything without type was left
	if (!m_token.sv.empty()) {
		time = sv_convert_to<f32>(m_token.sv).value_or(0);
		total_time += ceil(time);
	}
	return total_time;
}

u32 Parser::get_time_ticks() {
	return ceil(get_time_seconds() * 24);
}

s32 Parser::get_var_value(bool is_timer) {
	if (get_keyword("random")) {
		s32 min_value = is_timer ? get_time_ticks() : get_s32();
		s32 max_value = is_timer ? get_time_ticks() : get_s32();
		return bw::rand_between(min_value, max_value);
	}
	return is_timer ? get_time_ticks() : get_s32();
}

Area Parser::get_area(Area default_value) {
	Area area = {};
	if (!skip_whitespace()) {
		return area;
	}

	if (get_char('(')) {
		area.flags = ~0;
		area.box.left = get_s32();
		area.box.top  = get_s32();
		{
			u32 relative_op = get_relative_op();
			u32 w = get_s32();
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
			u32 relative_op = get_relative_op();
			u32 h = get_s32();
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

		get_char(')');
		return area;
	}

	const auto marker = this->marker();
	Token string = get_string();
	if (equals("*")) {
		area.box = g_location_table[63].dimensions;
		area.flags = ~g_location_table[63].flags;
		return area;
	}
	for (u32 i = 0; i < 256; ++i) {
		char* name = helper::get_chk_string(g_location_table[i].string_id);
		if (name && name[0] && sv_equals(name, string.sv)) {
			area.box = g_location_table[i].dimensions;
			area.flags = ~g_location_table[i].flags;
			return area;
		}
	}
	unwind(marker); // string wasn't valid, let's unwind and try an int

	s32 i = get_s32(-1);
	if (i >= 0 && i < 256) {
		area.box = g_location_table[i].dimensions;
		area.flags = ~g_location_table[i].flags;
		return area;
	}

	return default_value;
}

bool Area::check(Point16 position, bool air) {
	if (position.x >= this->box.left && position.x <= this->box.right &&
		position.y >= this->box.top  && position.y <= this->box.bottom) {
		ActiveTile* tile = bw::get_active_tile_at_coords(position.x, position.y);
		u32 height = tile->ground_height / 2;
		if (height == 0) {
			// Low
			return air ? this->flags & LocationFlags_LowAir : this->flags & LocationFlags_LowGround;
		} else if (height == 1) {
			// Mid
			return air ? this->flags & LocationFlags_MidAir : this->flags & LocationFlags_MidGround;
		} else if (height == 2) {
			// High
			return air ? this->flags & LocationFlags_HighAir : this->flags & LocationFlags_HighGround;
		}
		return false;
	}
	return false;
}

Point16 Parser::get_position(Point16 default_value) {
	if (!skip_whitespace()) {
		return {};
	}

	if (get_char('(')) {
		Point16 pos = {};
		pos.x = get_s32();
		pos.y = get_s32();
		get_char(')');
		return pos;
	}

	const auto marker = this->marker();
	Token string = get_string();
	if (equals("*")) {
		return box_center(g_location_table[63].dimensions);
	}
	for (u32 i = 0; i < 256; ++i) {
		char* name = helper::get_chk_string(g_location_table[i].string_id);
		if (name && name[0] && sv_equals(name, string.sv)) {
			return box_center(g_location_table[i].dimensions);
		}
	}
	unwind(marker); // string wasn't valid, let's unwind and try an int

	s32 i = get_s32();
	if (i >= 0 && i < 256) {
		return box_center(g_location_table[i].dimensions);
	}

	return default_value;
}

s32 Parser::get_enum(const char** enum_names, u32 count, s32 default_value) {
	Token token = get_string();

	for (u32 i = 0; i < count; ++i) {
		if (sv_iequals(token.sv, enum_names[i]) == 0) {
			return i;
		}
	}

	return default_value;
}

u32 Parser::get_player(bool allow_neutral) {
	s32 player = get_s32(-1);
	if (player >= 0) {
		if (player <= 8 || (allow_neutral && player == 12)) {
			return player;
		}
		if (player == 13) {
			return *g_current_trigger_player_id + 1;
		}
	}
	return 0;
}

static PlayerBitset player_bitset_combine(PlayerBitset a, PlayerBitset b) {
	a |= b;
	return a;
}

static void player_bitset_set(PlayerBitset& bitset, u32 i, bool value, bool mask, bool negate) {
	if (negate) {
		value = !value;
	}
	if (value != mask) {
		bitset[i] = value;
	}
}

PlayerBitset Parser::get_player_bitset() {
	bool mask = false;
	PlayerBitset bitset = {};
	PlayerBitset current_bitset = {};
	while (true) {
		bool negate = get_negation();
		const auto marker = this->marker();
		s32 value = get_s32();
		if (equals("*")) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, true, mask, negate);
			}
		} else if (value == 14) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, bw::is_allied_to(*g_current_trigger_player_id, i), mask, negate);
			}
		} else if (value == 15) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, !bw::is_allied_to(*g_current_trigger_player_id, i), mask, negate);
			}
		} else if (value == 16) {
			// wasn't used in aise, but we can do something here later?
		} else if (value == 17) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, true, mask, negate);
			}
		} else if (value == 18) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, g_player_table[i].force == 1, mask, negate);
			}
		} else if (value == 19) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, g_player_table[i].force == 2, mask, negate);
			}
		} else if (value == 20) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, g_player_table[i].force == 3, mask, negate);
			}
		} else if (value == 21) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, g_player_table[i].force == 4, mask, negate);
			}
		} else {
			unwind(marker);
			s32 player = get_player(true);
			if (player) {
				player -= 1;
				for (u32 i = 0; i < FullPlayerLimit; ++i) {
					player_bitset_set(current_bitset, i, i == player, mask, negate);
				}
			}
		}

		char separator = get_separator();
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

static void remove_prefix(std::string_view& string, std::string_view prefix) {
	const auto string_prefix = string.substr(0, prefix.size());
	if (sv_iequals(string_prefix, prefix)) {
		string.remove_prefix(prefix.size());
	
		for (const char c : string) {
			if (!isspace(c)) break;

			string.remove_prefix(1);
		}
	}
}

Token get_short_unit_name(u32 unit_id) {
	std::string_view name = Unit::get_name(unit_id);
	remove_prefix(name, "terran");
	remove_prefix(name, "protoss");
	remove_prefix(name, "zerg");
	return {name};
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

u32 Parser::get_unit_id() {
	const auto marker = this->marker();
	Token token = get_string();
	for (u32 i = 0; i < UnitId::Count; ++i) {
		if (sv_lazy_equals(token.sv, get_short_unit_name(i).sv)) {
			return i;
		}
	}
	unwind(marker);
	return get_s32(UnitId::None);
}

static void unit_id_bitset_set(UnitIdBitset& bitset, u32 i, bool value, bool mask, bool negate) {
	if (negate) {
		value = !value;
	}
	if (value) {
		if (!mask) {
			bitset[i] = true;
		}
	} else if (mask) {
		bitset[i] = false;
	}
}

UnitIdBitset Parser::get_unit_id_bitset() {
	bool mask = false;
	UnitIdBitset bitset = {};
	UnitIdBitset current_bitset = {};
	while (true) {
		bool negate = get_negation();
		const auto marker = this->marker();
		get_string();
		if (equals("*")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, true, mask, negate);
			}
		} else if (iequals("men")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, units_dat::group_flags[i].is_men, mask, negate);
			}
		} else if (iequals("buildings")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, units_dat::group_flags[i].is_building, mask, negate);
			}
		} else if (iequals("factories")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, units_dat::group_flags[i].is_factory, mask, negate);
			}
		} else if (iequals("tech")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, unit_id_is_tech(i), mask, negate);
			}
		} else if (iequals("tech_tier")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, unit_id_is_tech_tier_structure(i), mask, negate);
			}
		} else if (iequals("terran")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, unit_id_race_check(i, RaceId::Terran), mask, negate);
			}
		} else if (iequals("protoss")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, unit_id_race_check(i, RaceId::Protoss), mask, negate);
			}
		} else if (iequals("zerg")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, unit_id_race_check(i, RaceId::Zerg), mask, negate);
			}
		} else if (iequals("combatants")) {
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
				unit_id_bitset_set(current_bitset, i, is_combatant, mask, negate);
			}
		} else if (iequals("transports")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, unit_id_is_transport(i), mask, negate);
			}
		} else if (iequals("townhalls")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, unit_id_is_town_center(i), mask, negate);
			}
		} else if (iequals("depots")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, units_dat::base_property[i] & UnitProperty::ResourceDepot, mask, negate);
			}
		} else if (iequals("nodes")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, units_dat::base_property[i] & UnitProperty::ResourceContainer, mask, negate);
			}
		} else if (iequals("factional")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, unit_id_is_factional(i), mask, negate);
			}
		} else if (iequals("workers")) {
			for (u32 i = 0; i < UnitId::Count; ++i) {
				unit_id_bitset_set(current_bitset, i, units_dat::base_property[i] & UnitProperty::Worker, mask, negate);
			}
		} else {
			unwind(marker);
			s32 unit_id = get_unit_id();
			if (unit_id != UnitId::None) {
				for (u32 i = 0; i < UnitId::Count; ++i) {
					unit_id_bitset_set(current_bitset, i, i == unit_id, mask, negate);
				}
			}
		}

		char separator = get_separator();
		if (separator == '&') {
			mask = true;
		} else if (separator == '|') {
			mask = false;
			bitset |= current_bitset;
			current_bitset = {};
		} else {
			bitset |= current_bitset;
			break;
		}
	}
	return bitset;
}

QuotaType Parser::get_quota_type(QuotaType default_type) {
	if (!skip_whitespace()) {
		return default_type;
	}

	if (*m_at == '>' || *m_at == '<' || *m_at == '!' || *m_at == '=' || *m_at == '*') {
		const auto marker = this->marker();
		char op = *m_at++;

		bool equals = get_char('=');
		switch (op) {
			case '>': return equals ? QuotaType_GreaterOrEqual : QuotaType_Greater; break;
			case '<': return equals ? QuotaType_LessOrEqual : QuotaType_Less; break;
			case '=': return QuotaType_Equal; break;
			case '!': return QuotaType_NotEqual; break;
			case '*': {
				// We pop since we don't want next get to consume next argument
				unwind(marker);
				return QuotaType_Anything;
			} break;
		}
	}
	return default_type;
}

Quota Parser::get_quota(QuotaType default_type, s32 default_value) {
	Quota quota{};
	quota.type = get_quota_type(default_type);
	quota.value = get_s32(default_value);
	return quota;
}

Quota Parser::get_time_quota(QuotaType default_type) {
	Quota quota{};
	quota.type = get_quota_type(default_type);
	quota.value = get_time_ticks();
	return quota;
}

bool Quota::check(s32 value) {
	switch (this->type) {
		case QuotaType_Greater:			return value >  this->value;
		case QuotaType_Less:			return value <  this->value;
		case QuotaType_GreaterOrEqual:	return value >= this->value;
		case QuotaType_LessOrEqual:		return value <= this->value;
		case QuotaType_Equal:			return value == this->value; 
		case QuotaType_NotEqual:		return value != this->value; 
		case QuotaType_Anything:		return true; 
	}
	return false;
}

bool Parser::has_recolor() {
	if (!skip_whitespace()) {
		return false;
	}
	return *m_at == '(';
}

Recolor Parser::get_recolor() {
	Recolor recolor = default_recolor();
	if (!skip_whitespace()) {
		return recolor;
	}

	if (get_char('(')) {
		recolor.r          = get_f32(recolor.r);
		recolor.g          = get_f32(recolor.g);
		recolor.b          = get_f32(recolor.b);
		recolor.gamma      = get_f32(recolor.gamma);
		recolor.saturation = get_f32(recolor.saturation);
		get_char(')');
	}
	return recolor;
}

RecolorSet Parser::get_recolor_set() {
	RecolorSet set = {};
	while (set.count < RecolorCap) {
		if (!has_recolor()) {
			break;
		}
		set.recolors[set.count++] = get_recolor();
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

bool sv_equals(std::string_view a, std::string_view b) {
	return std::ranges::equal(a, b);
}

bool sv_iequals(std::string_view a, std::string_view b) {
	return std::ranges::equal(a, b, [](char a, char b) { return tolower(a) == tolower(b); });
}

bool sv_lazy_equals(std::string_view a, std::string_view b) {
	return std::ranges::equal(a, b, [](char a, char b) { return tolazy(a) == tolazy(b); });
}
