#pragma once
#include "SCBW/api.h"
#include "color_palette.h"

#define MaxToken 256
struct Token {
	u32 size;
	char data[MaxToken];
};

struct Parser {
	bool inited;
	char* at;
	Token token;
};

bool parser_get_word(Parser* parser);
bool parser_get_command(Parser* parser);
Token parser_get_string(Parser* parser, bool consume_rest = false);
s32 parser_get_s32(Parser* parser, u32 radix = 10);
f32 parser_get_f32(Parser* parser);
Box32 parser_get_location(Parser* parser);
bool parser_has_recolor(Parser* parser);
Recolor parser_get_recolor(Parser* parser);
RecolorSet parser_get_recolor_set(Parser* parser);
Point16 box_center(Box32 box);
bool parser_eq(Parser* parser, char* string);