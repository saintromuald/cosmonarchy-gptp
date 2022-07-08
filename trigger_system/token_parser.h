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

	s8 stack_index;
	char* stack[ParserStackSize];
};

struct PlayerBitset {
	bool set[PlayerLimit];
};

struct UnitIdBitset {
	bool set[UnitId::Count];
};

// Parser stack allows for unwinding trivial parser chunks like words, string, numbers. This does not work very well on complex types like bitsets, recolors, etc.
void parser_push(Parser* parser);
void parser_pop(Parser* parser);

bool parser_get_word(Parser* parser);
bool parser_get_command(Parser* parser);
Token parser_get_string(Parser* parser);

s32 parser_get_s32(Parser* parser, u32 radix = 10);
f32 parser_get_f32(Parser* parser);

Box32 parser_get_location(Parser* parser);
Point16 parser_get_position(Parser* parser);

u32 parser_get_enum(Parser* parser, const char** enum_names, u32 count);

u32 parser_get_player(Parser* parser);
PlayerBitset parser_get_player_bitset(Parser* parser);
u32 parser_get_unit_id(Parser* parser);
UnitIdBitset parser_get_unit_id_bitset(Parser* parser);

bool parser_has_recolor(Parser* parser);
Recolor parser_get_recolor(Parser* parser);
RecolorSet parser_get_recolor_set(Parser* parser);

Point16 box_center(Box32 box);

bool parser_eq(Parser* parser, char* string);
bool parser_eqi(Parser* parser, char* string);