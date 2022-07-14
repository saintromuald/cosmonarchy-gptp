#pragma once
#include "SCBW/api.h"
#include "color_palette.h"

#define MaxToken 256
struct Token {
	u32 size;
	char data[MaxToken];
};

#define ParserStackSize 64
struct Parser {
	bool inited;
	char* at;
	Token token;
	bool number_parsed_correctly;

	s8 stack_index;
	char* stack[ParserStackSize];
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

struct PlayerBitset {
	bool set[PlayerLimit];
};

struct UnitIdBitset {
	bool set[UnitId::Count];
};

enum QuotaType {
	QuotaType_Greater,
	QuotaType_Less,
	QuotaType_GreaterOrEqual,
	QuotaType_LessOrEqual,
	QuotaType_Equal,
	QuotaType_NotEqual,

	QuotaType_Count
};

// Parser stack allows for unwinding parser chunks like words, string, numbers. Currently doesn't work on bitsets, since they are recursively parsed
void parser_push(Parser* parser);
void parser_pop(Parser* parser);

Token parser_get_word(Parser* parser);
bool parser_get_command(Parser* parser);
Token parser_get_string(Parser* parser);

s32 parser_get_s32(Parser* parser, u32 radix = 10);
f32 parser_get_f32(Parser* parser);
f32 parser_get_time(Parser* parser);

Area parser_get_area(Parser* parser);
bool check_area(Area area, Point16 position, bool air);
Point16 parser_get_position(Parser* parser);

u32 parser_get_enum(Parser* parser, const char** enum_names, u32 count);

bool parser_get_negation(Parser* parser);
char parser_get_separator(Parser* parser);

u32 parser_get_player(Parser* parser, bool allow_neutral = false); // 1 - 8
PlayerBitset parser_get_player_bitset(Parser* parser);

Token get_short_unit_name(u32 unit_id);
u32 parser_get_unit_id(Parser* parser);
UnitIdBitset parser_get_unit_id_bitset(Parser* parser);

QuotaType parser_get_quota_type(Parser* parser, QuotaType default_type);
bool check_quota(QuotaType type, s32 quota, s32 value);

bool parser_has_recolor(Parser* parser);
Recolor parser_get_recolor(Parser* parser);
RecolorSet parser_get_recolor_set(Parser* parser);

Point16 box_center(Box32 box);

bool token_eq(Token* token, char* string);
bool token_eqi(Token* token, char* string);
bool parser_eq(Parser* parser, char* string);
bool parser_eqi(Parser* parser, char* string);