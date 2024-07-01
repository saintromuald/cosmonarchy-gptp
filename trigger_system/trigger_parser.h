#pragma once
#include "BW/api.h"
#include "color_palette.h"

#include <bitset>
#include <charconv>

bool sv_equals(std::string_view a, std::string_view b);
bool sv_iequals(std::string_view a, std::string_view b);
bool sv_lazy_equals(std::string_view a, std::string_view b);

struct Token {
	std::string_view sv;

	Token() = default;
	Token(const char* cstr) : sv{cstr} {}
	Token(std::string_view sv) : sv{sv} {}

	std::string string() const { return std::string{sv}; }
	operator bool() { return !sv.empty(); }
	bool operator==(const Token&) const = default;

	bool equals(std::string_view other) { return sv_equals(sv, other); }
	bool iequals(std::string_view other) { return sv_iequals(sv, other); }
	bool lazy_equals(std::string_view other) { return sv_lazy_equals(sv, other); }
};

enum LocationFlags {
	LocationFlags_LowGround  = 0x01,
	LocationFlags_MidGround  = 0x02,
	LocationFlags_HighGround = 0x04,
	LocationFlags_LowAir     = 0x08,
	LocationFlags_MidAir     = 0x10,
	LocationFlags_HighAir    = 0x20,
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
};

using PlayerBitset = std::bitset<FullPlayerLimit>;
using UnitIdBitset = std::bitset<UnitId::Count>;

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

class TriggerVar;

struct ParserMarker {
	const char* at;
	Token token;
};

Point16 box_center(Box32 box);
Token get_short_unit_name(u32 unit_id);

// Parses and handles <area> <players> <unit_ids>
struct Designator {
	Area area;
	PlayerBitset players;
	UnitIdBitset unit_ids;

	EntryArray<Unit> search(u32 limit = UINT_MAX) const {
		auto result = g_unit_search.rect(area.box);
		if (limit != UINT_MAX) {
			result.sort_by_distance(box_center(area.box));
		}

		u32 idx = 0;
		result.filter_by([this, limit, &idx](Unit* unit) {
			idx++;
			if (idx > limit) {
				return false;
			}
			if (!area.check(unit->position, unit->status & UnitStatus::InAir)) {
				return false;
			}
			if (unit->player >= PlayerLimit || !players[unit->player]) {
				return false;
			}
			if (unit->id >= UnitId::Count || !unit_ids[unit->id]) {
				return false;
			}
			return true;
		});
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
	Token get_string();
	Token get_identifier();

	TriggerVar* maybe_get_var();
	s32 get_s32(s32 default_value = 0);
	u32 get_count(u32 default_value = UINT_MAX);
	f32 get_f32(f32 default_value = 0);
	bool get_bool(bool default_value = false);
	f32 get_time_seconds();
	u32 get_time_ticks();
	s32 get_var_value(bool is_timer);

	Area get_area(Area default_value = {});
	Point16 get_position(Point16 default_value = {});

	s32 get_enum(const char** enum_names, size_t count, s32 default_value = 0);

	template <typename T>
	T get_typed_enum(const char* enum_names[(size_t)T::Count], T default_value = T::None) {
		return (T)get_enum(enum_names, (size_t)T::Count, (s32)default_value);
	}

	u32 get_player(bool allow_neutral = false); // 1 - 8
	PlayerBitset get_player_bitset();

	u32 get_unit_id();
	UnitIdBitset get_unit_id_bitset();

	Designator get_designator();

	QuotaType get_quota_type(QuotaType default_type);
	Quota get_quota(QuotaType default_type = QuotaType_GreaterOrEqual, s32 default_value = 0);
	Quota get_time_quota(QuotaType default_type = QuotaType_LessOrEqual);

	bool has_recolor();
	Recolor get_recolor();
	RecolorSet get_recolor_set();

	bool equals(std::string_view sv) { return m_token.equals(sv); }
	bool iequals(std::string_view sv) { return m_token.iequals(sv); }
	bool lazy_equals(std::string_view sv) { return m_token.lazy_equals(sv); }

	Token token() const { return m_token; }
	u32 command_start_pos() const { return m_command_start_pos; }

private:
	void start_token();
	void expand_token();

	Token eat_word();
	void clear_token();
	bool skip_whitespace();
	bool skip_non_word_chars();
	bool get_negation();
	char get_separator();
	char get_relative_op();

private:
	bool m_parsed_first_command{};
	bool m_should_load_pos{};
	const char* m_start{};
	const char* m_at{};
	u32 m_command_start_pos{};
	Token m_token{};
};

template <typename T>
std::optional<T> sv_convert_to(std::string_view sv) {
	T value{};
	const auto result = std::from_chars(sv.data(), sv.data() + sv.size(), value);
	return (bool)result.ec ? std::nullopt : std::make_optional(value);
}
