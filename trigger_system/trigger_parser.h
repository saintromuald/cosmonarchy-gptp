#pragma once
#include "BW/api.h"
#include "color_palette.h"

#include <bitset>

#define MaxToken 256
struct Token {
	u32 size;
	char data[MaxToken + 1];

	inline bool operator==(Token& other) { return size == other.size && memcmp(data, other.data, size) == 0; }
	inline bool operator!=(Token& other) { return !(*this == other); }
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

class TriggerVar;

class Parser {
public:
	Parser(const char* at);
	const char* marker();
	void unwind(const char* marker);

	bool get_char(char c);
	Token get_word(bool is_command = false);
	void load_command_pos(u32 position);
	bool get_command();
	bool get_keyword(const char* keyword);
	Token get_string();
	Token get_identifier();

	TriggerVar* maybe_get_var();
	s32 get_s32(s32 default_value = 0);
	f32 get_f32(f32 default_value = 0);
	f32 get_time_seconds();
	u32 get_time_ticks();
	s32 get_var_value(bool is_timer);

	Area get_area(Area default_value = {});
	Point16 get_position(Point16 default_value = {});

	s32 get_enum(const char** enum_names, u32 count, s32 default_value = 0);

	u32 get_player(bool allow_neutral = false); // 1 - 8
	PlayerBitset get_player_bitset();

	u32 get_unit_id();
	UnitIdBitset get_unit_id_bitset();

	QuotaType get_quota_type(QuotaType default_type);

	bool has_recolor();
	Recolor get_recolor();
	RecolorSet get_recolor_set();

	bool eq(const char* string);
	bool eqi(const char* string);

	Token token() const { return m_token; }
	u32 command_start_pos() const { return m_command_start_pos; }

private:
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

Point16 box_center(Box32 box);
bool check_area(Area area, Point16 position, bool air);
Token token_from_str(const char* str);
Token get_short_unit_name(u32 unit_id);
bool check_quota(QuotaType type, s32 quota, s32 value);

bool token_eq(Token* token, const char* string);
bool token_eqi(Token* token, const char* string);