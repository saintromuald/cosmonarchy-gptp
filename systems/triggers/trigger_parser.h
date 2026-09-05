#pragma once
#include "BW/api.h"
#include "color_palette.h"

#include <bitset>
#include <charconv>
#include <concepts>
#include <optional>
#include <string>

bool sv_iequals(std::string_view a, std::string_view b);
bool sv_lazy_equals(std::string_view a, std::string_view b);

std::string_view get_short_unit_name(Unit* unit);
std::string_view get_short_unit_name(u32 unit_id);

namespace trigger {

enum LocationFlags {
	LocationFlags_LowGround  = 0x01,
	LocationFlags_MidGround  = 0x02,
	LocationFlags_HighGround = 0x04,
	LocationFlags_LowAir     = 0x08,
	LocationFlags_MidAir     = 0x10,
	LocationFlags_HighAir    = 0x20,

	LocationFlags_All        = 0x3F,
};

struct Area {
	Box32 box;
	u16 flags;

	bool check(v2u16 position, bool air) const {
		if (position.x >= this->box.left && position.x <= this->box.right &&
			position.y >= this->box.top && position.y <= this->box.bottom) {
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

	bool is_entire_map() const {
		return box.x == 0
			&& box.y == 0
			&& box.w == g_map_width_pixels
			&& box.h == g_map_height_pixels
			&& (flags & LocationFlags_All) == LocationFlags_All;
	}

	v2u16 center() const {
		return box.center();
	}
};

using PlayerBitset = std::bitset<FullPlayerLimit>;
using UnitIdBitset = std::bitset<UnitId::Count>;
using WeaponIdBitset = std::bitset<WeaponId::Count>;
using FlingyIdBitset = std::bitset<flingy_id::Count>;

template <typename I, size_t N, size_t MAX = N>
struct IterateBitset {
    const std::bitset<N>& bitset;

    struct Iterator {
        const std::bitset<N>& bitset;
		size_t index{};

		Iterator(const std::bitset<N>& bitset, size_t index) : bitset{bitset}, index{index} { if (!bitset[index]) { ++*this; } }
		I operator*() const { return I(index); }
		Iterator& operator++() {
			while (++index < MAX) {
				if (bitset[index]) {
					break;
				}
			}
			return *this;
		}
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        friend bool operator==(const Iterator& a, const Iterator& b) { return a.index == b.index; };
        friend bool operator!=(const Iterator& a, const Iterator& b) { return !(a == b); };
    };

	Iterator begin() { return Iterator{bitset, 0}; }
	Iterator end() { return Iterator{bitset, MAX}; }
};

using IteratePlayerBitsetPlayable = IterateBitset<int, FullPlayerLimit, PlayerLimit>;
using IteratePlayerBitsetAll      = IterateBitset<int, FullPlayerLimit>;
using IterateUnitIdBitset         = IterateBitset<UnitId::Enum, UnitId::Count>;
using IterateWeaponIdBitset       = IterateBitset<WeaponId::Enum, WeaponId::Count>;

enum QuotaType {
	QuotaType_Greater,
	QuotaType_Less,
	QuotaType_GreaterOrEqual,
	QuotaType_LessOrEqual,
	QuotaType_Equal,
	QuotaType_NotEqual,
	QuotaType_Anything,

	QuotaType_Count
};

struct Quota {
	QuotaType type;
	s32 value;

	bool check(s32 value) const {
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
};

enum class StatOperator {
	Set,
	Add,
	Subtract,
	Multiply,
	Divide,
	Negate,
	Or,
	InverseAnd,
	Xor,
};

struct ParserMarker {
	const char* at;
	std::string_view token;
};

// Parses and handles <area> <players> <unit_ids>
struct Designator {
	Area area;
	PlayerBitset players;
	UnitIdBitset unit_ids;

	struct SearchSettings {
		u32 limit = 0; // NOTE: limit is unsorted, biased towards top left chunks in search area (internal sorting not guaranteed either)
		bool sort = false;
	};

	bool check(Unit* unit) const {
        if (!area.check(unit->position, unit->is_in_air())) {
            return false;
        }
        if (unit->player >= FullPlayerLimit || !players[unit->player]) {
            return false;
        }
        if (unit->id >= UnitId::Count || !unit_ids[unit->id]) {
            return false;
        }
        return true;
	}

	EntryArray<Unit> search(const SearchSettings& settings = {}) const {
		return (area.is_entire_map() ? g_unit_search.rect(area.box, settings.limit) : g_unit_search.all(settings.limit))
			.apply_if(settings.sort, [this](auto&& units) { return units.sort_by_distance(area.box.center()); })
            .filter_by([this](Unit* unit) { return check(unit); });
	}

	EntryArray<Unit> search_seen(u32 player, const SearchSettings& settings = {}) const {
		return (area.is_entire_map() ? g_seen_unit_search[player].rect(area.box, settings.limit) : g_seen_unit_search[player].all(settings.limit))
			.apply_if(settings.sort, [this](auto&& units) { return units.sort_by_distance(area.box.center()); })
            .filter_by([this](Unit* unit) { return check(unit); });
	}
};

template <std::integral T>
std::optional<T> sv_convert_to(std::string_view sv, int base = 10) {
	T value{};
	const auto result = std::from_chars(sv.data(), sv.data() + sv.size(), value, base);
	if (result.ec != std::errc{}) {
		return {};
	}
	return value;
}

template <std::floating_point T>
std::optional<T> sv_convert_to(std::string_view sv, std::chars_format fmt = std::chars_format::general) {
	T value{};
	const auto result = std::from_chars(sv.data(), sv.data() + sv.size(), value, fmt);
	if (result.ec != std::errc{}) {
		return {};
	}
	return value;
}

template <typename T> requires std::same_as<T, fp32> || std::same_as<T, fp64>
std::optional<T> sv_convert_to(std::string_view sv) {
	using Raw = decltype(T{}.raw_value());
	using Wide = boost::int128::int128_t;

	if (sv.empty()) {
		return {};
	}

	bool negative = false;
	if (sv.front() == '+' || sv.front() == '-') {
		negative = sv.front() == '-';
		sv.remove_prefix(1);
	}
	if (sv.empty()) {
		return {};
	}

	const auto decimal_point = sv.find('.');
	const auto whole_sv = sv.substr(0, decimal_point);
	const auto fraction_sv = decimal_point == std::string_view::npos ? std::string_view{} : sv.substr(decimal_point + 1);

	u64 whole = 0;
	if (!whole_sv.empty()) {
		const auto result = std::from_chars(whole_sv.data(), whole_sv.data() + whole_sv.size(), whole);
		if (result.ec != std::errc{} || result.ptr != whole_sv.data() + whole_sv.size()) {
			return {};
		}
	}

	u64 fraction = 0;
	u64 denominator = 1;
	for (const auto character : fraction_sv) {
		if (!isdigit(character) || denominator > std::numeric_limits<u64>::max() / 10) {
			return {};
		}
		fraction = fraction * 10 + u64(character - '0');
		denominator *= 10;
	}

	constexpr auto fraction_bits = std::same_as<T, fp32> ? 12 : 32;
	Wide raw = Wide(whole) << fraction_bits;
	if (denominator > 1) {
		const auto denominator_wide = Wide(denominator);
		raw += ((Wide(fraction) << fraction_bits) + denominator_wide / 2) / denominator_wide;
	}
	if (negative) {
		raw = -raw;
	}
	if (raw < std::numeric_limits<Raw>::min() || raw > std::numeric_limits<Raw>::max()) {
		return {};
	}
	return T::from_raw_value(Raw(raw));
}

// Helper parameter types for parser getters that don't map to a unique C++ type.
// Internal player index (0 - 7), or none. Users write 1 - 8 in crigs.
template <bool AllowNeutral>
struct PlayerId { u32 value; operator u32() const { return value; } };
using PlayablePlayer  = PlayerId<false>;
using PlayerOrNeutral = PlayerId<true>;

struct Identifier { std::string_view value; operator std::string_view() const { return value; } };
struct Ticks      { u32 value; operator u32() const { return value; } };
struct Seconds    { fp64 value; operator fp64() const { return value; } };

struct TimeQuota     : Quota {};
struct EqualityQuota : Quota {};
struct VarQuota      : Quota { bool create; };
struct TimerQuota    : Quota { bool create; };
struct VarValue   { s32 value; operator s32() const { return value; } };
struct TimerValue { s32 value; operator s32() const { return value; } };

struct SpriteSelector { SpriteId::Enum value; bool valid; };

template <typename T>
struct Any {
    T value;
    bool any;
};

enum class StringType {
	Quotable,
	QuotesDeprecated,
	Identifier,
};

std::string enum_name_to_snake_case(std::string_view name);

struct LuaScript {
    std::string_view name;
    std::string_view body;
};

class Parser {
public:
	Parser(const char* at);
	ParserMarker marker();
	void unwind(const ParserMarker& marker);

	bool get_char(char c);
	std::optional<std::string_view> get_word(bool is_command = false, bool allow_hash = false);
	bool has_next_token();
	void load_command_pos(u32 position);
	std::optional<std::string_view> get_command();
	std::optional<LuaScript> get_lua();
	bool get_keyword(const char* keyword, bool allow_hash = false);
	bool eat_prefix(const char* chars);
	std::optional<std::string_view> get_string(StringType type = StringType::Quotable);

	std::optional<s32> maybe_get_var_value();
	std::optional<bool> get_bool();
	std::optional<Seconds> get_time_seconds();
	std::optional<Ticks> get_time_ticks();
	std::optional<s32> get_var_value(bool is_timer);

	std::optional<VarQuota> get_var_quota();
	std::optional<TimerQuota> get_timer_quota();

	std::optional<SpriteSelector> get_sprite_selector();

	Location* get_location();
	std::optional<Area> get_area();
	std::optional<v2u16> get_position();

	template <typename T> requires std::is_enum_v<T>
	std::optional<T> get_magic_enum() {
		const auto marker = this->marker();
		const auto token = get_string(StringType::QuotesDeprecated);
		if (!token) {
			return {};
		}
		for (const auto [value, name] : magic_enum::enum_entries<T>()) {
			if (sv_iequals(*token, name) || sv_iequals(*token, enum_name_to_snake_case(name))) {
				return value;
			}
		}
		unwind(marker);
		const auto value = get_integer();
		if (!value) {
			return {};
		}
		return T(*value);
	}

	template <typename T> requires std::is_enum_v<T>
	std::optional<T> get_magic_flags() {
		return parse_bitset<T>([&](auto& parser, std::string_view token) {
			if (const auto flag = magic_enum::enum_flags_cast<T>(token)) {
                parser.set([flag = u32(*flag)](u32 i) { return (1 << i) & flag; });
			} else {
                parser.unwind_token();
				if (const auto value = get_integer<u32>()) {
                    parser.set([value = *value](u32 i) { return (i & value) != 0; });
                }
			}
		});
	}

	template <bool AllowNeutral = false>
	std::optional<PlayerId<AllowNeutral>> get_player();
	std::optional<PlayerBitset> get_player_bitset();

	std::optional<UnitId::Enum> get_unit_id();
	std::optional<UnitIdBitset> get_unit_id_bitset();

	std::optional<WeaponId::Enum> get_weapon_id();
	std::optional<WeaponIdBitset> get_weapon_id_bitset();

	std::optional<flingy_id::Enum> get_flingy_id();
	std::optional<FlingyIdBitset> get_flingy_id_bitset();

    std::optional<StatOperator> get_stat_operator();

	std::optional<Designator> get_designator();

	std::optional<QuotaType> get_quota_type(QuotaType default_type);
	std::optional<Quota> get_quota(QuotaType default_type = QuotaType_GreaterOrEqual);
	std::optional<TimeQuota> get_time_quota();

	bool has_recolor();
	std::optional<Recolor> get_recolor();
	std::optional<RecolorSet> get_recolor_set();

	bool iequals(std::string_view sv) const { return sv_iequals(m_token, sv); }

	u32 command_start_pos() const { return m_command_start_pos; }

	std::string_view current_command_line() const {
		std::string_view sv{m_start + m_command_start_pos};
		if (const auto newline_pos = sv.find_first_of("\r\n"); newline_pos != std::string_view::npos) {
			return sv.substr(0, newline_pos);
		}
		return sv;
	}

	template <typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args) {
		bw::play_sound(SoundId::Misc_Buzz_wav_1);
        bw::println();
        bw::println(TextColor::Yellow3, "Crig Warning: {}{}", TextColor::White, std::format(fmt, std::forward<Args>(args)...));
        bw::println(TextColor::Grey, "{}", current_command_line());
    }

	template <typename... Args>
	void error(std::format_string<Args...> fmt, Args&&... args) {
		m_had_errors = true;
		bw::play_sound(SoundId::Misc_Buzz_wav_1);
		bw::println();
		bw::println(TextColor::Red, "Crig Error: {}{}", TextColor::White, std::format(fmt, std::forward<Args>(args)...));
		bw::println(TextColor::Grey, "{}", current_command_line());
	}

    // Reports an error only if no previous errors have been reported in this context
    template <typename... Args>
    void error_once(std::format_string<Args...> fmt, Args&&... args) {
        if (!m_had_errors) {
            error(fmt, std::forward<Args>(args)...);
        }
    }

	template <std::integral T = s32>
	std::optional<T> get_integer(u32 base = 10) {
		if (const auto var_value = maybe_get_var_value()) {
			return T(*var_value);
		}

        if (eat_prefix("0x")) {
            const auto token = get_word();
            if (!token) {
                return {};
            }
            return sv_convert_to<T>(*token, 16);
        }

        const auto token = get_word();
        if (!token) {
            return {};
        }
        if (sv_iequals(*token, "true")) {
            return T{1};
        }
        if (sv_iequals(*token, "false")) {
            return T{0};
        }

        return sv_convert_to<T>(*token, base);
    }

	template <utils::FloatLike T = fp32>
    std::optional<T> get_float() {
        const auto token = get_word();
        if (!token) {
            return {};
        }
        return sv_convert_to<T>(*token);
	}

    bool had_errors() const { return m_had_errors; }
    void clear_errors() { m_had_errors = false; }

private:
	template <typename Bitset, typename Func>
	std::optional<Bitset> parse_bitset(Func&& callback);

	void start_token();
	void expand_token();

	std::optional<fp64> get_time_seconds_raw();

	std::optional<std::string_view> eat_word(bool allow_hash = false);
	bool skip_spaces();
	void skip_all_whitespace();
	void skip_to_next_line();
	bool skip_non_word_chars(bool allow_hash = false);
	bool get_negation();
	char get_separator();
	char get_relative_op();

private:
	template <typename Bitset>
	friend class BitsetParser;

	bool m_had_errors = false;
	bool m_should_load_pos{};
	const char* m_start{};
	const char* m_at{};
	u32 m_command_start_pos{};
	std::string_view m_token{};
};

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
			const auto token = m_parser.get_string(StringType::Identifier);
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

} // namespace trigger
