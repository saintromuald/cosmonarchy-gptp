#pragma once
#include "SCBW/api.h"

#define MaxToken 256
struct Token {
	u32 size;
	char data[MaxToken];
};

struct Parser {
	char* at;
	Token token;
};

bool parser_get_word(Parser* parser);
Token parser_get_token(Parser* parser);
Token parser_get_rest_of_the_line(Parser* parser);
Token parser_get_string(Parser* parser);
s32 parser_get_s32(Parser* parser, u32 radix = 10);
f32 parser_get_f32(Parser* parser);
bool parser_eq(Parser* parser, char* string);