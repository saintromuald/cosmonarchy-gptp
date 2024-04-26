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

#define ParserStackSize 64
struct Parser {
	bool parsed_first_command;
	bool should_load_pos;
	const char* start;
	const char* at;
	u32 command_start_pos;
	Token token;
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

Parser new_parser(const char* at);
const char* parser_marker(Parser* parser);
void parser_unwind(Parser* parser, const char* marker);


bool parser_get_char(Parser* parser, char c);
Token parser_get_word(Parser* parser, bool is_command = false);
void parser_load_command_pos(Parser* parser, u32 position);
bool parser_get_command(Parser* parser);
bool parser_get_keyword(Parser* parser, const char* keyword);
Token parser_get_string(Parser* parser);
Token parser_get_identifier(Parser* parser);

s32 parser_get_s32(Parser* parser, s32 default_value = 0);
f32 parser_get_f32(Parser* parser, f32 default_value = 0);
f32 parser_get_time_seconds(Parser* parser);
u32 parser_get_time_ticks(Parser* parser);
s32 parser_get_var_value(Parser* parser, bool is_timer);

Area parser_get_area(Parser* parser, Area default_value = {});
bool check_area(Area area, Point16 position, bool air);
Point16 parser_get_position(Parser* parser, Point16 default_value = {});

s32 parser_get_enum(Parser* parser, const char** enum_names, u32 count, s32 default_value = 0);

u32 parser_get_player(Parser* parser, bool allow_neutral = false); // 1 - 8
PlayerBitset parser_get_player_bitset(Parser* parser);

Token token_from_str(const char* str);
Token get_short_unit_name(u32 unit_id);
u32 parser_get_unit_id(Parser* parser);
UnitIdBitset parser_get_unit_id_bitset(Parser* parser);

QuotaType parser_get_quota_type(Parser* parser, QuotaType default_type);
bool check_quota(QuotaType type, s32 quota, s32 value);

bool parser_has_recolor(Parser* parser);
Recolor parser_get_recolor(Parser* parser);
RecolorSet parser_get_recolor_set(Parser* parser);

Point16 box_center(Box32 box);

bool token_eq(Token* token, const char* string);
bool token_eqi(Token* token, const char* string);
bool parser_eq(Parser* parser, const char* string);
bool parser_eqi(Parser* parser, const char* string);