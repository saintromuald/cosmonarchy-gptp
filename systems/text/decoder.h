#pragma once
#include "types.h"
#include "BW/enumerations/UnitId.h"

#include <optional>
#include <variant>

namespace text {

struct Control {
    u8 value;
};

struct Codepoint {
    char32_t value;
};

struct Icon {
    u16 unit_id;
};

using Token = std::variant<Control, Codepoint, Icon>;

// The patched writeFontApplyColour (glucustom.cpp) treats 0x8F-0x9F as
// additional color codes — its "Exclude <A0> to <FF>" comment is off by one:
// the comparison excludes >= 0xA0, so 0x9F passes. 0x99 is excluded and
// stays the trademark glyph.
constexpr u8 ExtendedColorFirst = 0x8F;
constexpr u8 ExtendedColorLast = 0x9F;
constexpr u8 TrademarkByte = 0x99;

// Decodes one token and advances the cursor past it; returns nullopt once the
// cursor reaches end. Bytes that fail UTF-8 validation resolve as legacy bytes
// through the cp1252 table with the icon overrides applied, except the extended
// color bytes 0x8F-0x9F (0x99 excepted), which resolve as controls like the
// engine's patched writeFontApplyColour does.
std::optional<Token> decode_next(const u8*& cursor, const u8* end);

constexpr u8 CursorPrefixByte = 0x04;

bool is_cursor_pair(const u8* token_start, const u8* token_end, const u8* view_start);

} // namespace text