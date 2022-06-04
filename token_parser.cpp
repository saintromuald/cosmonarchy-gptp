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

bool parser_get_word(Parser* parser) {
	memset(&parser->token, 0, sizeof(parser->token));

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
	memset(&parser->token, 0, sizeof(parser->token));

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
	memset(&parser->token, 0, sizeof(parser->token));

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
		char* name = helper::get_chk_string(locationTable[i].string_id);
		if (name && name[0] && strcmp(name, string.data) == 0) {
			Box32 box = locationTable[i].dimensions;
			box.w -= box.x;
			box.h -= box.y;
			return box;
		}
	}

	s32 i = parser_get_s32(parser);
	if (i >= 0 && i < 256) {
		Box32 box = locationTable[i].dimensions;
		box.w -= box.x;
		box.h -= box.y;
		return box;
	}

	return {};
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