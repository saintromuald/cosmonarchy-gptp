#include "parser.h"

bool parser_get_word(Parser* parser) {
	memset(&parser->token, 0, sizeof(parser->token));

	// Skip Whitespace
	while (*parser->at && isspace(*parser->at)) {
		++parser->at;
	}

	// Write Token
	while (*parser->at && !isspace(*parser->at)) {
		if (parser->token.size < MaxToken - 1) {
			parser->token.data[parser->token.size++] = *parser->at;
		}
		++parser->at;
	}

	return parser->token.size != 0;
}

Token parser_get_token(Parser* parser) {
	parser_get_word(parser);
	return parser->token;
}

Token parser_get_rest_of_the_line(Parser* parser) {
	memset(&parser->token, 0, sizeof(parser->token));

	// Skip Whitespace
	while (*parser->at && isspace(*parser->at)) {
		++parser->at;

		if (*parser->at == '\r' || *parser->at == '\n') {
			return parser->token;
		}
	}

	// Write Token
	while (*parser->at && *parser->at != '\r' && *parser->at != '\n') {
		if (parser->token.size < MaxToken - 1) {
			parser->token.data[parser->token.size++] = *parser->at;
		}
		++parser->at;
	}

	return parser->token;
}

Token parser_get_string(Parser* parser) {
	memset(&parser->token, 0, sizeof(parser->token));

	// Skip Whitespace
	while (*parser->at && isspace(*parser->at)) {
		++parser->at;

		if (*parser->at == '\r' || *parser->at == '\n') {
			return parser->token;
		}
	}

	// Write Token
	bool is_string = false;
	if (*parser->at == '"') {
		++parser->at;
		while (*parser->at && *parser->at != '"' && *parser->at != '\r' && *parser->at != '\n') {
			if (parser->token.size < MaxToken - 1) {
				parser->token.data[parser->token.size++] = *parser->at;
			}
			++parser->at;
		}
	} else {
		while (*parser->at && !isspace(*parser->at)) {
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

bool parser_eq(Parser* parser, char* string) {
	if (!string) return false;
	return strcmp(parser->token.data, string) == 0;
}