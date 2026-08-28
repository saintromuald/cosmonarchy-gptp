#include "trigger_parser.h"
#include "trigger_vars.h"
#include "BW/stat_txt_spec.h"

#include <magic_enum/magic_enum_all.hpp>
using namespace magic_enum::bitwise_operators;

namespace trigger {


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

std::string enum_name_to_snake_case(std::string_view name) {
	std::string result;
	result.reserve(name.size());
	for (size_t i = 0; i < name.size(); ++i) {
		const auto current = name[i];
		const auto upper = isupper(current);
		const auto previous_lower = i > 0 && islower(name[i - 1]);
		const auto next_lower = i + 1 < name.size() && islower(name[i + 1]);
		if (upper && (previous_lower || (i > 0 && next_lower))) {
			result += '_';
		}
		result += char(tolower(current));
	}
	return result;
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
	m_token = {m_at, 0};
}

void Parser::expand_token() {
	if (m_token.data()) {
		m_token = {m_token.data(), m_token.size() + 1};
	} else {
		assert(false);
	}
	m_at++;
}

std::optional<std::string_view> Parser::eat_word() {
	start_token();
	while (is_word_char(*m_at)) {
		expand_token();
	}
	if (m_token.empty()) {
		return {};
	}
	return m_token;
}

std::optional<std::string_view> Parser::get_word(bool is_command) {
	if (!skip_non_word_chars()) {
		return {};
	}

	if (is_command) {
		m_command_start_pos = m_at - m_start;
	}
	return eat_word();
}


bool Parser::has_next_token() {
	const auto saved = marker();
	const auto token = get_word();
	unwind(saved);
	return token.has_value();
}

void Parser::load_command_pos(u32 position) {
	if (position) {
		m_should_load_pos = true;
		m_at = m_start + position;
	}
}

std::optional<std::string_view> Parser::get_command() {
	if (m_should_load_pos) {
		m_should_load_pos = false;
		m_parsed_first_command = true;
		m_command_start_pos = m_at - m_start;
		return eat_word();
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

	return get_word(true);
}

bool Parser::get_keyword(const char* keyword) {
	const auto marker = this->marker();
	const auto word = get_word();
	if (word == keyword) {
		return true;
	}
	unwind(marker);
	return false;
}

bool Parser::eat_prefix(const char* prefix) {
	const char* c = m_at;
	const char* p = prefix;
	while (*c && *p) {
		if (*c != *p) {
			return false;
		}
		c++;
		p++;
	}
	if (*p) {
		return false;
	}
	m_at = c;
	return true;
}

std::optional<std::string_view> Parser::get_string(bool allow_quotes) {
	if (!skip_whitespace()) {
		return {};
	}

	if (allow_quotes && get_char('"')) {
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

std::optional<std::string_view> Parser::get_identifier() {
	if (!skip_whitespace()) {
		return {};
	}

	start_token();
	while (*m_at && !is_any_space(*m_at) && !is_separator(*m_at) && *m_at != '"') {
		expand_token();
	}

	return m_token;
}

std::optional<s32> Parser::maybe_get_var_value() {
	if (!get_char('$')) {
		return {};
	}
	const auto name = get_word();
	if (!name) {
		return {};
	}
	const auto var = g_trigger_vars.find(TriggerVarType_Var, *name);
	if (!var) {
		return {};
	}
	return var->value();
}

std::optional<bool> Parser::get_bool() {
	const auto value = get_integer();
	if (!value) {
		return {};
	}
	return bool(*value);
}

std::optional<fp64> Parser::get_time_seconds_raw() {
	if (!skip_whitespace() || !*m_at) {
		return {};
	}

	const auto add_time = [](fp64& total_seconds, fp64 time, char suffix) {
		switch (suffix) {
			case 'h': total_seconds += time * 60 * 60; break;
			case 'm': total_seconds += time * 60;      break;
			case 's': total_seconds += time;           break;
			case 't': total_seconds += time / 24;      break;
		}
	};

	fp64 total_seconds{};
	start_token();
	while (*m_at && !is_any_space(*m_at) && !is_separator(*m_at)) {
		if (isdigit(*m_at) || *m_at == '.' || (m_token.empty() && (*m_at == '+' || *m_at == '-'))) {
			expand_token();
			continue;
		}

		const auto time = sv_convert_to<fp64>(m_token);
		if (!time) {
			return {};
		}

		if (*m_at == 'm' && m_at[1] == 's') {
			total_seconds += *time / 1000;
			expand_token();
			expand_token();
		} else if (*m_at == 'h' || *m_at == 'm' || *m_at == 's' || *m_at == 't') {
			const auto suffix = *m_at;
			add_time(total_seconds, *time, suffix);
			expand_token();
		} else {
			return {};
		}
		start_token();
	}

	if (!m_token.empty()) {
		const auto time = sv_convert_to<fp64>(m_token);
		if (!time) {
			return {};
		}
		total_seconds += fpm::ceil(*time);
	}
	return total_seconds;
}

std::optional<Seconds> Parser::get_time_seconds() {
	const auto seconds = get_time_seconds_raw();
	if (!seconds) {
		return {};
	}
	return Seconds{*seconds};
}

std::optional<Ticks> Parser::get_time_ticks() {
	if (!skip_whitespace() || !*m_at) {
		return {};
	}

	const auto add_time = [](s64& total_ticks, fp32 time, char suffix) {
		const s64 raw_time = time.raw_value();
		switch (suffix) {
			case 'h': total_ticks += raw_time * 60 * 60 * 24; break;
			case 'm': total_ticks += raw_time * 60 * 24;      break;
			case 's': total_ticks += raw_time * 24;           break;
			case 't': total_ticks += raw_time;                break;
		}
	};

	s64 total_ticks{};
	start_token();
	while (*m_at && !is_any_space(*m_at) && !is_separator(*m_at)) {
		if (isdigit(*m_at) || *m_at == '.' || (m_token.empty() && (*m_at == '+' || *m_at == '-'))) {
			expand_token();
			continue;
		}

		const auto time = sv_convert_to<fp32>(m_token);
		if (!time) {
			return {};
		}

		if (*m_at == 'm' && m_at[1] == 's') {
			total_ticks += s64(time->raw_value()) * 24 / 1000;
			expand_token();
			expand_token();
		} else if (*m_at == 'h' || *m_at == 'm' || *m_at == 's' || *m_at == 't') {
			const auto suffix = *m_at;
			add_time(total_ticks, *time, suffix);
			expand_token();
		} else {
			return {};
		}
		start_token();
	}

	if (!m_token.empty()) {
		const auto time = sv_convert_to<fp32>(m_token);
		if (!time) {
			return {};
		}
		total_ticks += s64(fpm::ceil(*time).raw_value()) * 24;
	}

	constexpr s64 fraction_scale = fp32{1}.raw_value();
	if (total_ticks > 0) {
		total_ticks += fraction_scale - 1;
	}
	return Ticks{u32(total_ticks / fraction_scale)};
}

std::optional<s32> Parser::get_var_value(bool is_timer) {
	if (get_keyword("random")) {
		std::optional<s32> min_value;
		std::optional<s32> max_value;
		if (is_timer) {
			const auto min_ticks = get_time_ticks();
			const auto max_ticks = get_time_ticks();
			if (min_ticks) {
				min_value = s32(min_ticks->value);
			}
			if (max_ticks) {
				max_value = s32(max_ticks->value);
			}
		} else {
			min_value = get_integer();
			max_value = get_integer();
		}
		if (!min_value || !max_value) {
			return {};
		}
		return bw::rand_between(*min_value, *max_value);
	}
	if (is_timer) {
		const auto ticks = get_time_ticks();
		if (!ticks) {
			return {};
		}
		return s32(ticks->value);
	}
	return get_integer();
}

std::optional<VarQuota> Parser::get_var_quota() {
	const auto type = get_quota_type(QuotaType_Equal);
	const auto create = get_char('=');
	if (!type) {
		return {};
	}
	if (*type == QuotaType_Anything) {
		get_word(); // Consume the wildcard rewound by get_quota_type.
		return VarQuota{Quota{.type = *type, .value = 0}, create};
	}
	const auto value = get_integer();
	if (!value) {
		return {};
	}
	return VarQuota{Quota{.type = *type, .value = *value}, create};
}

std::optional<TimerQuota> Parser::get_timer_quota() {
	const auto type = get_quota_type(QuotaType_Equal);
	const auto create = get_char('=');
	const auto value = get_time_ticks();
	if (!type || !value) {
		return {};
	}
	return TimerQuota{Quota{.type = *type, .value = s32(value->value)}, create};
}

std::optional<SpriteSelector> Parser::get_sprite_selector() {
	const auto selector = get_word();
	if (selector == "sprite_id") {
		const auto sprite_id = get_integer();
		if (!sprite_id) {
			return {};
		}
		return SpriteSelector{.value = SpriteId::Enum(*sprite_id), .valid = true};
	}
	if (selector == "unit_id") {
		const auto unit_id = get_unit_id();
		if (!unit_id) {
			return {};
		}
		return SpriteSelector{.value = SpriteId::Enum(flingy_dat::sprite_id[units_dat::graphic[*unit_id]]), .valid = true};
	}
	return {};
}

Location* Parser::get_location() {
	const auto marker = this->marker();
	const auto string = get_string();
	if (!string) {
		return nullptr;
	}
	if (*string == "*") {
        return &g_location_table[63];
	}
	for (u32 i = 0; i < 256; ++i) {
		char* name = helper::get_chk_string(g_location_table[i].string_id);
		if (name && name[0] && *string == name) {
			return &g_location_table[i];
		}
	}
	unwind(marker); // string wasn't valid, let's unwind and try an int

	const auto index = get_integer();
	if (index && *index >= 0 && *index < 256) {
		return &g_location_table[*index];
	}
	return nullptr;
}

std::optional<Area> Parser::get_area() {
	Area area = {};
	if (!skip_whitespace()) {
		return {};
	}

	if (get_char('(')) {
		area.flags = ~0;
		area.box.left = get_integer().value_or(0);
		area.box.top  = get_integer().value_or(0);
		{
			u32 relative_op = get_relative_op();
			const auto w = u32(get_integer().value_or(0));
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
			const auto h = u32(get_integer().value_or(0));
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

	if (const auto location = get_location()) {
        area.box = location->dimensions;
        area.flags = ~location->flags;
        return area;
	}
	return {};
}

std::optional<v2u16> Parser::get_position() {
	if (!skip_whitespace()) {
		return {};
	}

	if (get_char('(')) {
		v2u16 pos = {};
		pos.x = get_integer().value_or(0);
		pos.y = get_integer().value_or(0);
		get_char(')');
		return pos;
	}

	if (const auto location = get_location()) {
		return location->dimensions.center();
	}
	return {};
}


template <bool AllowNeutral>
std::optional<PlayerId<AllowNeutral>> Parser::get_player() {
	const auto value = get_integer();
	if (!value || *value == 0) {
		return {};
	}
	const auto player = *value;
	if (player >= 1 && player <= PlayerLimit) {
		return PlayerId<AllowNeutral>{u32(player - 1)};
	}
	if (player == 13) {
		return PlayerId<AllowNeutral>{u32(*g_current_trigger_player_id)};
	}
	if constexpr (AllowNeutral) {
		if (player == 12) {
			return PlayerId<AllowNeutral>{u32(player - 1)};
		}
	}
	error("Invalid player", player);
	return {};
}
template std::optional<PlayerId<false>> Parser::get_player<false>();
template std::optional<PlayerId<true>>  Parser::get_player<true>();

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

std::optional<PlayerBitset> Parser::get_player_bitset() {
	bool mask = false;
	PlayerBitset bitset = {};
	PlayerBitset current_bitset = {};
	while (true) {
		const auto negate = get_negation();
		const auto marker = this->marker();
		const auto token = get_word();
		if (!token) {
			return {};
		}
		unwind(marker);
		const auto value = get_integer();
		if (token == "*") {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, true, mask, negate);
			}
		} else if (value && *value == 13) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, i == *g_current_trigger_player_id, mask, negate);
			}
		} else if (value && *value == 14) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, bw::is_allied_to(*g_current_trigger_player_id, i), mask, negate);
			}
		} else if (value && *value == 15) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, !bw::is_allied_to(*g_current_trigger_player_id, i), mask, negate);
			}
		} else if (value && *value == 16) {
			// wasn't used in aise, but we can do something here later?
		} else if (value && *value == 17) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, true, mask, negate);
			}
		} else if (value && *value == 18) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, g_player_table[i].force == 1, mask, negate);
			}
		} else if (value && *value == 19) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, g_player_table[i].force == 2, mask, negate);
			}
		} else if (value && *value == 20) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, g_player_table[i].force == 3, mask, negate);
			}
		} else if (value && *value == 21) {
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, g_player_table[i].force == 4, mask, negate);
			}
		} else {
			unwind(marker);
			const auto player = get_player<true>();
			if (!player) {
				return {};
			}
			for (u32 i = 0; i < FullPlayerLimit; ++i) {
				player_bitset_set(current_bitset, i, i == *player, mask, negate);
			}
		}

		const auto separator = get_separator();
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

std::string_view get_short_flingy_name(u32 flingy_id) {
	return FlingyId::get_name(flingy_id::Enum(flingy_id));
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

std::optional<UnitId::Enum> Parser::get_unit_id() {
	const auto marker = this->marker();
	const auto token = get_string();
	for (u32 i = 0; token && i < UnitId::Count; ++i) {
		if (sv_lazy_equals(*token, get_short_unit_name(i))) {
			return UnitId::Enum(i);
		}
	}
	unwind(marker);
	const auto value = get_integer();
	if (!value) {
		return {};
	}
	return UnitId::Enum(*value);
}

template <typename T>
struct BitsetSize_t;

template <size_t N>
struct BitsetSize_t<std::bitset<N>> : std::integral_constant<size_t, N> {};

template <std::integral T>
struct BitsetSize_t<T> : std::integral_constant<size_t, 8 * sizeof(T)> {};

template <typename T> requires std::is_enum_v<T>
struct BitsetSize_t<T> : std::integral_constant<size_t, 8 * sizeof(T)> {};

template <typename T>
static constexpr size_t BitsetSize = BitsetSize_t<T>::value;

template <typename Bitset>
class BitsetParser {
public:
	BitsetParser(Parser& parser) : m_parser{parser} {}

	template <typename Func>
	std::optional<Bitset> parse(Func&& callback) {
		Bitset result{};
		m_bitset = {};
		m_mask = false;
		while (true) {
			m_negate = m_parser.get_negation();
			m_marker = m_parser.marker();
			const auto token = m_parser.get_string(false);
			if (!token || token->empty()) {
				return {};
			}

			if (token == "*") {
				set([](u32 i) { return true; });
			} else {
				callback(*this, *token);
			}

			const auto separator = m_parser.get_separator();
			if (separator == '&') {
				m_mask = true;
				continue;
			}
			if (separator == '|') {
				m_mask = false;
				result |= m_bitset;
				m_bitset = {};
				continue;
			}
			result |= m_bitset;
			break;
		}
		return result;
	}

	template <typename Func>
	void set(Func&& func) {
		for (u32 i = 0; i < BitsetSize<Bitset>; i++) {
			bool value = func(i);
            if (m_negate) {
                value = !value;
            }
            if (value) {
                if (!m_mask) {
                    if constexpr (std::is_integral_v<Bitset> || std::is_enum_v<Bitset>) {
                        m_bitset = Bitset(u32(m_bitset) | (1 << i));
                    } else {
                        m_bitset[i] = true;
                    }
                }
            } else if (m_mask) {
                if constexpr (std::is_integral_v<Bitset> || std::is_enum_v<Bitset>) {
                    m_bitset = Bitset(u32(m_bitset) & ~(1 << i));
                } else {
                    m_bitset[i] = false;
                }
            }
		}
	}

	void unwind_token() {
		m_parser.unwind(m_marker);
	}

private:
	Parser& m_parser;
	Bitset m_bitset;
	bool m_mask;
	bool m_negate;
	ParserMarker m_marker;
};

template <typename Bitset, typename Func>
std::optional<Bitset> Parser::parse_bitset(Func&& callback) {
	BitsetParser<Bitset> parser{*this};
	return parser.parse(callback);
}

std::optional<UnitIdBitset> Parser::get_unit_id_bitset() {
	return parse_bitset<UnitIdBitset>([&](BitsetParser<UnitIdBitset>& parser, std::string_view token) {
		if (sv_iequals(token, "men")) {
            parser.set([](u32 i) { return units_dat::group_flags[i].is_men; });
		} else if (sv_iequals(token, "buildings")) {
            parser.set([](u32 i) { return units_dat::group_flags[i].is_building; });
		} else if (sv_iequals(token, "factories")) {
            parser.set([](u32 i) { return units_dat::group_flags[i].is_factory; });
		} else if (sv_iequals(token, "tech")) {
            parser.set([](u32 i) { return unit_id_is_tech(i); });
		} else if (sv_iequals(token, "tech_tier")) {
            parser.set([](u32 i) { return unit_id_is_tech_tier_structure(i); });
		} else if (sv_iequals(token, "terran")) {
            parser.set([](u32 i) { return unit_id_is_race(i, RaceId::Terran); });
		} else if (sv_iequals(token, "protoss")) {
            parser.set([](u32 i) { return unit_id_is_race(i, RaceId::Protoss); });
		} else if (sv_iequals(token, "zerg")) {
            parser.set([](u32 i) { return unit_id_is_race(i, RaceId::Zerg); });
		} else if (sv_iequals(token, "askosi")) {
            parser.set([](u32 i) { return unit_id_is_race(i, RaceId::Askosi); });
		} else if (sv_iequals(token, "combatants")) {
            parser.set([](u32 i) { return unit_id_is_combatant(i); });
		} else if (sv_iequals(token, "military")) {
            parser.set([](u32 i) { return unit_id_is_military(i); });
		} else if (sv_iequals(token, "transports")) {
            parser.set([](u32 i) { return unit_id_is_transport(i); });
		} else if (sv_iequals(token, "townhalls")) {
            parser.set([](u32 i) { return unit_id_is_town_center(i); });
		} else if (sv_iequals(token, "depots")) {
            parser.set([](u32 i) { return units_dat::base_property[i] & UnitProperty::ResourceDepot; });
		} else if (sv_iequals(token, "nodes")) {
            parser.set([](u32 i) { return units_dat::base_property[i] & UnitProperty::ResourceContainer; });
		} else if (sv_iequals(token, "factional")) {
            parser.set([](u32 i) { return unit_id_is_factional(i); });
		} else if (sv_iequals(token, "workers")) {
            parser.set([](u32 i) { return units_dat::base_property[i] & UnitProperty::Worker; });
		} else {
            parser.unwind_token();
            if (const auto unit_id = get_unit_id()) {
                parser.set([unit_id = *unit_id](u32 i) { return i == unit_id; });
            }
		}
	});
}

std::optional<WeaponId::Enum> Parser::get_weapon_id() {
	const auto marker = this->marker();
	const auto token = get_string();
	for (u32 i = 0; token && i < WeaponId::Count; ++i) {
		if (sv_lazy_equals(*token, g_stat_txt_tbl_new->get_string(weapons_dat::label[i]))) { // TODO: Use new method
			return WeaponId::Enum(i);
		}
	}
	unwind(marker);
	const auto value = get_integer();
	if (!value) {
		return {};
	}
	return WeaponId::Enum(*value);
}

std::optional<WeaponIdBitset> Parser::get_weapon_id_bitset() {
	return parse_bitset<WeaponIdBitset>([&](BitsetParser<WeaponIdBitset>& parser, std::string_view) {
        parser.unwind_token();
        if (const auto weapon_id = get_weapon_id()) {
            parser.set([weapon_id = *weapon_id](u32 i) { return i == weapon_id; });
        }
	});
}

std::optional<flingy_id::Enum> Parser::get_flingy_id() {
	const auto marker = this->marker();
	const auto token = get_string();
	for (u32 i = 0; token && i < flingy_id::Count; ++i) {
		if (sv_lazy_equals(*token, get_short_flingy_name(i))) {
			return flingy_id::Enum(i);
		}
	}
	unwind(marker);
	const auto value = get_integer();
	if (!value) {
		return {};
	}
	return flingy_id::Enum(*value);
}

std::optional<FlingyIdBitset> Parser::get_flingy_id_bitset() {
	return parse_bitset<FlingyIdBitset>([&](BitsetParser<FlingyIdBitset>& parser, std::string_view) {
        parser.unwind_token();
        if (const auto flingy_id = get_flingy_id()) {
            parser.set([flingy_id = *flingy_id](u32 i) { return i == flingy_id; });
        }
	});
}


std::optional<StatOperator> Parser::get_stat_operator() {
	if (!skip_whitespace() || !*m_at) {
		return {};
	}
	if (eat_prefix("+=")) return StatOperator::Add;
	if (eat_prefix("-=")) return StatOperator::Subtract;
	if (eat_prefix("*=")) return StatOperator::Multiply;
	if (eat_prefix("/=")) return StatOperator::Divide;
	if (eat_prefix("|=")) return StatOperator::Or;
	if (eat_prefix("&=")) return StatOperator::InverseAnd;
	if (eat_prefix("^=")) return StatOperator::Xor;
	if (eat_prefix("!"))  return StatOperator::Negate;
	if (eat_prefix("="))  return StatOperator::Set;
	return StatOperator::Set;
}

std::optional<Designator> Parser::get_designator() {
	const auto area = get_area();
	if (!area) {
		return {};
	}
	const auto players = get_player_bitset();
	if (!players) {
		return {};
	}
	const auto unit_ids = get_unit_id_bitset();
	if (!unit_ids) {
		return {};
	}
	return Designator{
		.area = *area,
		.players = *players,
		.unit_ids = *unit_ids,
	};
}

std::optional<QuotaType> Parser::get_quota_type(QuotaType default_type) {
	if (!skip_whitespace() || !*m_at) {
		return {};
	}

	switch (*m_at) {
		case '>':
		case '<':
		case '!':
		case '=':
		case '*': {
            const auto marker = this->marker();
            char op = *m_at++;

            bool equals = get_char('=');
            switch (op) {
                case '>': return equals ? QuotaType_GreaterOrEqual : QuotaType_Greater;
                case '<': return equals ? QuotaType_LessOrEqual : QuotaType_Less;
                case '=': return QuotaType_Equal;
                case '!': return QuotaType_NotEqual;
                case '*': {
                    // We pop since we don't want next get to consume next argument
                    unwind(marker);
                    return QuotaType_Anything;
                } break;
            }
		} break;
	}
	return default_type;
}

std::optional<Quota> Parser::get_quota(QuotaType default_type) {
	const auto type = get_quota_type(default_type);
	const auto value = get_integer();
	if (!type || !value) {
		return {};
	}
	return Quota{
		.type = *type,
		.value = *value,
	};
}

std::optional<TimeQuota> Parser::get_time_quota() {
	const auto type = get_quota_type(QuotaType_LessOrEqual);
	const auto value = get_time_ticks();
	if (!type || !value) {
		return {};
	}
	return TimeQuota{{
		.type = *type,
		.value = s32(value->value),
	}};
}

bool Parser::has_recolor() {
	if (!skip_whitespace()) {
		return false;
	}
	return *m_at == '(';
}

std::optional<Recolor> Parser::get_recolor() {
	if (!skip_whitespace() || !get_char('(')) {
		return {};
	}

	const auto r = get_float<f32>();
	const auto g = get_float<f32>();
	const auto b = get_float<f32>();
	const auto gamma = get_float<f32>();
	const auto saturation = get_float<f32>();
	if (!r || !g || !b || !gamma || !saturation) {
		return {};
	}
	get_char(')');
	return Recolor{
		.r = *r,
		.g = *g,
		.b = *b,
		.gamma = *gamma,
		.saturation = *saturation,
	};
}

std::optional<RecolorSet> Parser::get_recolor_set() {
	RecolorSet set = {};
	while (set.count < RecolorCap) {
		if (!has_recolor()) {
			break;
		}
		const auto recolor = get_recolor();
		if (!recolor) {
			return {};
		}
		set.recolors[set.count++] = *recolor;
	}
	if (!set.count) {
		return {};
	}
	return set;
}

} // namespace trigger

std::string_view get_short_unit_name(Unit* unit) {
	std::string_view name = unit->get_name();
	trigger::remove_prefix(name, "terran");
	trigger::remove_prefix(name, "protoss");
	trigger::remove_prefix(name, "zerg");
	trigger::remove_prefix(name, "askosi");
	trigger::remove_prefix(name, "bloodsummon");
	trigger::remove_prefix(name, "kogri");
	return name;
}

std::string_view get_short_unit_name(u32 unit_id) {
	std::string_view name = Unit::get_name(unit_id, true);
	trigger::remove_prefix(name, "terran");
	trigger::remove_prefix(name, "protoss");
	trigger::remove_prefix(name, "zerg");
	trigger::remove_prefix(name, "askosi");
	trigger::remove_prefix(name, "bloodsummon");
	trigger::remove_prefix(name, "kogri");
	return name;
}

bool sv_iequals(std::string_view a, std::string_view b) {
	return std::ranges::equal(a, b, [](char a, char b) { return tolower(a) == tolower(b); });
}

bool sv_lazy_equals(std::string_view a, std::string_view b) {
	return std::ranges::equal(a, b, [](char a, char b) { return trigger::tolazy(a) == trigger::tolazy(b); });
}
