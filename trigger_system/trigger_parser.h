#pragma once
#include "BW/api.h"
#include "color_palette.h"

#include <bitset>
#include <charconv>

bool sv_iequals(std::string_view a, std::string_view b);
bool sv_lazy_equals(std::string_view a, std::string_view b);

struct Token {
	std::string_view sv{};

	Token() = default;
	Token(const char* cstr) : sv{cstr} {}
	Token(std::string_view sv) : sv{sv} {}

	std::string string() const { return std::string{sv}; }
	operator bool() const { return !sv.empty(); }
	bool operator==(const Token&) const = default;

	bool iequals(std::string_view other) const { return sv_iequals(sv, other); }
	bool lazy_equals(std::string_view other) const { return sv_lazy_equals(sv, other); }
};
Token get_short_unit_name(Unit* unit);
Token get_short_unit_name(u32 unit_id);

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

	bool check(Point16 position, bool air) const {
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

	Point16 center() const {
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
        I operator*() const { return static_cast<I>(index); }
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
	Count,
};

class TriggerVar;

struct ParserMarker {
	const char* at;
	Token token;
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
        if (unit->player >= PlayerLimit || !players[unit->player]) {
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

class Parser {
public:
	Parser(const char* at);
	ParserMarker marker();
	void unwind(const ParserMarker& marker);

	bool get_char(char c);
	Token get_word(bool is_command = false);
	void load_command_pos(u32 position);
	bool get_command();
	bool get_keyword(const char* keyword);
	bool eat_prefix(const char* chars);
	Token get_string(bool allow_quotes = true);
	Token get_identifier();

	TriggerVar* maybe_get_var();
	s32 get_s32(s32 default_value = 0);
	u32 get_u32(u32 default_value = 0);
	u32 get_count(u32 default_value = UINT_MAX);
	f32 get_f32(f32 default_value = 0);
	bool get_bool(bool default_value = false);
	f32 get_time_seconds();
	u32 get_time_ticks();
	s32 get_var_value(bool is_timer);

	Location* get_location();
	Area get_area(Area default_value = {});
	Point16 get_position(Point16 default_value = {});

	s32 get_enum(const char** enum_names, size_t count, s32 default_value = 0);

	template <typename T>
	T get_typed_enum(const char* enum_names[(size_t)T::Count], T default_value = T::None) {
		return (T)get_enum(enum_names, (size_t)T::Count, (s32)default_value);
	}

	template <typename T> requires std::is_enum_v<T>
	T get_magic_enum(T default_value = T::None) {
		const auto marker = this->marker();
		const auto token = get_string(false);
		if (auto result = magic_enum::enum_cast<T>(token.sv)) {
			return *result;
		}
		return T(get_s32(s32(default_value)));
	}

	template <typename T> requires std::is_enum_v<T>
	T get_magic_flags() {
		return parse_bitset<T>([&](auto& parser) {
			if (auto flag = magic_enum::enum_flags_cast<T>(m_token.sv)) {
                parser.set([flag = u32(*flag)](u32 i) { return (1 << i) & flag; });
			} else {
                parser.unwind_token();
				const auto value = get_u32();
                parser.set([value](u32 i) { return (i & value) != 0; });
			}
		});
	}

	u32 get_player(bool allow_neutral = false); // 1 - 8
	PlayerBitset get_player_bitset();

	u32 get_unit_id();
	UnitIdBitset get_unit_id_bitset();

	u32 get_weapon_id();
	WeaponIdBitset get_weapon_id_bitset();

	u32 get_flingy_id();
	FlingyIdBitset get_flingy_id_bitset();

	u32 get_flags_bitset();
	u32 get_flags_bitset(const char** enum_names, size_t count);

    StatOperator get_stat_operator();

	Designator get_designator();

	QuotaType get_quota_type(QuotaType default_type);
	Quota get_quota(QuotaType default_type = QuotaType_GreaterOrEqual, s32 default_value = 0);
	Quota get_time_quota(QuotaType default_type = QuotaType_LessOrEqual);

	bool has_recolor();
	Recolor get_recolor();
	RecolorSet get_recolor_set();

	bool equals(std::string_view sv) const { return m_token == sv; }
	bool iequals(std::string_view sv) const { return m_token.iequals(sv); }
	bool lazy_equals(std::string_view sv) const { return m_token.lazy_equals(sv); }

	Token token() const { return m_token; }
	u32 command_start_pos() const { return m_command_start_pos; }

	std::string_view current_command_line() const {
		std::string_view sv{m_start + m_command_start_pos};
		if (const auto newline_pos = sv.find_first_of("\r\n"); newline_pos != std::string_view::npos) {
			return sv.substr(0, newline_pos);
		}
		return sv;
	}

	template <typename... Ts>
	void error(std::string_view title, Ts&&... args) {
		std::ostringstream ss;
		if (sizeof...(args) == 0) {
			ss << "!";
		} else {
			ss << ": ";
            size_t i = 0;
            (ss << ... << std::format("{}{}{}{}", i++ > 0 ? ", " : "", char(TextColor::Yellow3), args, char(TextColor::White)));
		}
		bw::play_sound(SoundId::Misc_Buzz_wav_1);
		bw::println();
		bw::println(TextColor::Red, "Crig Error: {}{}{}", TextColor::White, title, ss.str());
		bw::println(TextColor::Grey, "{}", current_command_line());
	}

private:
	template <typename Bitset, typename Func>
	Bitset parse_bitset(Func&& callback);

	void start_token();
	void expand_token();

	Token eat_word();
	void clear_token();
	bool skip_whitespace();
	bool skip_non_word_chars();
	bool get_negation();
	char get_separator();
	char get_relative_op();

    template <typename T>
    T get_integer(T default_value, u32 base = 10);

private:
	template <typename Bitset>
	friend class BitsetParser;

	bool m_parsed_first_command{};
	bool m_should_load_pos{};
	const char* m_start{};
	const char* m_at{};
	u32 m_command_start_pos{};
	Token m_token{};
};

template <std::integral T>
std::optional<T> sv_convert_to(std::string_view sv, int base = 10) {
	T value{};
	const auto result = std::from_chars(sv.data(), sv.data() + sv.size(), value, base);
	return (bool)result.ec ? std::nullopt : std::make_optional(value);
}

template <std::floating_point T>
std::optional<T> sv_convert_to(std::string_view sv, std::chars_format fmt = std::chars_format::general) {
	T value{};
	const auto result = std::from_chars(sv.data(), sv.data() + sv.size(), value, fmt);
	return (bool)result.ec ? std::nullopt : std::make_optional(value);
}
