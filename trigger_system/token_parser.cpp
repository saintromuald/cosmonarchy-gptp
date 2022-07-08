#include "token_parser.h"

static bool is_normal_space(char c) {
	return c == ' ' || c == '\t';
}

static bool is_any_space(char c) {
	return isspace(c);
}

static u32 is_newline(char c) {
	if (c == '\r' || c == '\n') {
		return 2;
	}
	return 0;
}

static bool parser_skip_whitespace(Parser* parser) {
	while (*parser->at && is_any_space(*parser->at)) {
		if (is_newline(*parser->at)) {
			return false;
		}
		++parser->at;
	}
	return true;
}

void parser_push(Parser* parser) {
	memset(&parser->token, 0, sizeof(parser->token));
	parser->stack[parser->stack_index] = parser->at;
	parser->stack_index += 1;
	parser->stack_index %= ParserStackSize;
}

void parser_pop(Parser* parser) {
	memset(&parser->token, 0, sizeof(parser->token));
	if (parser->stack[parser->stack_index]) {
		parser->at = parser->stack[parser->stack_index];
		parser->stack_index -= 1;
		if (parser->stack_index < 0) {
			parser->stack_index = ParserStackSize - 1;
		}
	}
}

bool parser_get_word(Parser* parser) {
	parser_push(parser);

	if (!parser_skip_whitespace(parser)) {
		return false;
	}

	while (*parser->at && !is_any_space(*parser->at)) {
		if (parser->token.size < MaxToken - 1) {
			parser->token.data[parser->token.size++] = *parser->at;
		}
		++parser->at;
	}

	return parser->token.size != 0;
}

bool parser_get_command(Parser* parser) {
	if (!parser->inited) {
		parser->inited = true;
	} else {
		while (*parser->at && !is_newline(*parser->at)) {
			++parser->at;
		}
	}

	while (*parser->at && is_newline(*parser->at)) {
		++parser->at;
	}

	return parser_get_word(parser);
}

Token parser_get_string(Parser* parser, bool consume_rest) {
	parser_push(parser);

	if (!parser_skip_whitespace(parser)) {
		return parser->token;
	}

	bool is_string = false;
	if (*parser->at == '"') {
		++parser->at;
		while (*parser->at && *parser->at != '"' && !is_newline(*parser->at)) {
			if (parser->token.size < MaxToken - 1) {
				parser->token.data[parser->token.size++] = *parser->at;
			}
			++parser->at;
		}
	} else {
		while (*parser->at) {
			if (!consume_rest && is_any_space(*parser->at)) {
				break;
			}
			if (consume_rest && is_newline(*parser->at)) {
				break;
			}

			if (parser->token.size < MaxToken - 1) {
				parser->token.data[parser->token.size++] = *parser->at;
			}
			++parser->at;
		}
	}

	return parser->token;
}

s32 parser_get_s32(Parser* parser, u32 radix) {
	parser_get_word(parser);
	return strtol(parser->token.data, 0, radix);
}

f32 parser_get_f32(Parser* parser) {
	parser_get_word(parser);
	return strtof(parser->token.data, 0);
}

Box32 parser_get_location(Parser* parser) {
	parser_push(parser);

	if (!parser_skip_whitespace(parser)) {
		return {};
	}

	if (*parser->at == '(') {
		++parser->at;

		while (*parser->at && *parser->at != ')') {
			if (is_newline(*parser->at)) {
				break;
			}

			if (parser->token.size < MaxToken - 1) {
				parser->token.data[parser->token.size++] = *parser->at;
			}
			++parser->at;
		}

		if (*parser->at == ')') {
			++parser->at;
		}

		Box32 box = {};
		sscanf(parser->token.data, "%u %u %u %u", &box.x, &box.y, &box.w, &box.h);
		return box;
	}

	Token string = parser_get_string(parser, true);
	for (u32 i = 0; i < 256; ++i) {
		char* name = helper::get_chk_string(g_location_table[i].string_id);
		if (name && name[0] && strcmp(name, string.data) == 0) {
			Box32 box = g_location_table[i].dimensions;
			box.w -= box.x;
			box.h -= box.y;
			return box;
		}
	}
	parser_pop(parser); // string wasn't valid, let's unwind and try an int

	s32 i = parser_get_s32(parser);
	if (i >= 0 && i < 256) {
		Box32 box = g_location_table[i].dimensions;
		box.w -= box.x;
		box.h -= box.y;
		return box;
	}

	return {};
}

Point16 parser_get_position(Parser* parser) {
	parser_push(parser);

	if (!parser_skip_whitespace(parser)) {
		return {};
	}

	if (*parser->at == '(') {
		++parser->at;

		while (*parser->at && *parser->at != ')') {
			if (is_newline(*parser->at)) {
				break;
			}

			if (parser->token.size < MaxToken - 1) {
				parser->token.data[parser->token.size++] = *parser->at;
			}
			++parser->at;
		}

		if (*parser->at == ')') {
			++parser->at;
		}

		Point16 pos = {};
		sscanf(parser->token.data, "%u %u", &pos.x, &pos.y);
		return pos;
	}

	Token string = parser_get_string(parser, true);
	for (u32 i = 0; i < 256; ++i) {
		char* name = helper::get_chk_string(g_location_table[i].string_id);
		if (name && name[0] && strcmp(name, string.data) == 0) {
			Box32 box = g_location_table[i].dimensions;
			box.w -= box.x;
			box.h -= box.y;
			return box_center(box);
		}
	}
	parser_pop(parser); // string wasn't valid, let's unwind and try an int

	s32 i = parser_get_s32(parser);
	if (i >= 0 && i < 256) {
		Box32 box = g_location_table[i].dimensions;
		box.w -= box.x;
		box.h -= box.y;
		return box_center(box);
	}

	return {};
}

u32 parser_get_enum(Parser* parser, const char** enum_names, u32 count) {
	Token token = parser_get_string(parser, false);

	for (u32 i = 0; i < count; ++i) {
		if (stricmp(token.data, enum_names[i]) == 0) {
			return i;
		}
	}

	return 0;
}

static Token get_short_unit_name(u32 unit_id) {
	Token token = {};
	const char* name = Unit::get_name(unit_id);

	bool after_space = false;
	for (const char* c = name; *c; ++c) {
		if (after_space) {
			if (token.size < MaxToken) {
				token.data[token.size++] = *c;
			} else {
				break;
			}
		}
		if (*c == ' ') {
			after_space = true;
		}
	}

	return token;
}

u32 parser_get_player(Parser* parser) {
	u32 player = parser_get_s32(parser);
	if (player < 8) {
		return player;
	}
	return *g_current_trigger_player_id;
}

u32 parser_get_unit_id(Parser* parser) {
	Token token = parser_get_string(parser, false);
	for (u32 i = 0; i < UnitId::Count; ++i) {
		if (stricmp(token.data, get_short_unit_name(i).data) == 0) {
			return i;
		}
	}
	parser_pop(parser);

	s32 unit_id = parser_get_s32(parser);
	if (parser->token.size) {
		return unit_id;
	}
	return UnitId::None;
}

UnitIdBitset parser_get_unit_id_bitset(Parser* parser) {
	parser_push(parser);

	UnitIdBitset bitset = {};
	if (*parser->at == '(') {
		++parser->at;

		while (*parser->at) {
			u32 buffer_index = 0;
			char buffer[MaxToken] = {};

			if (is_newline(*parser->at)) {
				break;
			}

			if (*parser->at == '|' || *parser->at == ')') {
				for (u32 i = 0; i < UnitId::Count; ++i) {
					if (stricmp(buffer, get_short_unit_name(i).data) == 0) {
						bitset.set[i] = true;
					}
				}

				s32 unit_id = strtol(buffer, 0, 10);
				if (unit_id >= 0 && unit_id < UnitId::Count) {
					bitset.set[unit_id] = true;
				}

				if (*parser->at == ')') {
					++parser->at;
					break;
				}
			} else {
				buffer[buffer_index++] = *parser->at; 
			}

			if (parser->token.size < MaxToken - 1) {
				parser->token.data[parser->token.size++] = *parser->at;
			}
			++parser->at;
		}
	}
	return bitset;
}

bool parser_has_recolor(Parser* parser) {
	if (!parser_skip_whitespace(parser)) {
		return false;
	}
	return *parser->at == '(';
}

Recolor parser_get_recolor(Parser* parser) {
	parser_push(parser);

	Recolor recolor = default_recolor();
	if (!parser_skip_whitespace(parser)) {
		return recolor;
	}

	if (*parser->at == '(') {
		++parser->at;

		while (*parser->at && *parser->at != ')') {
			if (is_newline(*parser->at)) {
				break;
			}

			if (parser->token.size < MaxToken - 1) {
				parser->token.data[parser->token.size++] = *parser->at;
			}
			++parser->at;
		}

		if (*parser->at == ')') {
			++parser->at;
		}

		Box32 box = {};
		sscanf(parser->token.data, "%f %f %f %f %f", &recolor.r, &recolor.g, &recolor.b, &recolor.gamma, &recolor.saturation);
	}
	return recolor;
}

RecolorSet parser_get_recolor_set(Parser* parser) {
	RecolorSet set = {};
	while (set.count < RecolorCap) {
		if (!parser_has_recolor(parser)) {
			break;
		}
		set.recolors[set.count++] = parser_get_recolor(parser);
	}
	if (!set.count) {
		set = default_recolor_set();
	}
	return set;
}

Point16 box_center(Box32 box) {
	Point16 point = {};
	point.x = box.x + box.w / 2;
	point.y = box.y + box.h / 2;
	return point;
}

bool parser_eq(Parser* parser, char* string) {
	if (!string) return false;
	return strcmp(parser->token.data, string) == 0;
}