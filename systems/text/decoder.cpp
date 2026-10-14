#include "decoder.h"

#include "BW/bwdata.h"

#include <array>

namespace text {
namespace {

// Space is included: draw_text handles it inline with the other controls.
constexpr u8 ControlLimit = 0x20;

constexpr u8 WitnessIconByte = 0x82;

// cp1252 for 0x80-0x9F with the legacy icon-byte overrides applied. cp1252's
// undefined slots keep their C1 codepoint (tofu); 0x82 never reaches the
// table; the extended color bytes never reach it either (intercepted in
// decode_next); 0xA0-0xFF are Latin-1 identity.
constexpr std::array<char32_t, 32> LegacyHighBytes{
    0x1F44B,                        // 0x80 waving hand
    0x1F1FA,                        // 0x81 regional indicator U
    0,                              // 0x82 witness icon
    0x1F915,                        // 0x83 head-bandage face
    0x2705,                         // 0x84 white heavy check mark
    0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, // 0x85-0x8C
    0x2610,                         // 0x8D ballot box
    0x2612,                         // 0x8E ballot box with X
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 0x8F-0x98 extended color codes
    0x2122,                         // 0x99 trademark
    0, 0, 0, 0, 0, 0,               // 0x9A-0x9F extended color codes
};

char32_t legacy_codepoint(u8 byte) {
    return byte >= 0xA0 ? byte : LegacyHighBytes[byte - 0x80];
}

struct DecodedUtf8 {
    char32_t codepoint;
    u8 length;
};

// Returns nullopt for invalid or truncated sequences.
std::optional<DecodedUtf8> decode_utf8(const u8* cursor, const u8* end) {
    const u8 lead = cursor[0];
    u32 codepoint;
    u8 continuation_count;
    u32 minimum;
    if ((lead & 0xE0) == 0xC0) {
        continuation_count = 1;
        minimum = 0x80;
        codepoint = lead & 0x1F;
    } else if ((lead & 0xF0) == 0xE0) {
        continuation_count = 2;
        minimum = 0x800;
        codepoint = lead & 0x0F;
    } else if ((lead & 0xF8) == 0xF0) {
        continuation_count = 3;
        minimum = 0x10000;
        codepoint = lead & 0x07;
    } else {
        return std::nullopt;
    }
    if (end - cursor < 1 + continuation_count) {
        return std::nullopt;
    }
    for (u8 i = 1; i <= continuation_count; ++i) {
        const u8 byte = cursor[i];
        if ((byte & 0xC0) != 0x80) {
            return std::nullopt;
        }
        codepoint = (codepoint << 6) | (byte & 0x3F);
    }
    if (codepoint < minimum || codepoint > 0x10FFFF
        || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
        return std::nullopt;
    }
    return DecodedUtf8{(char32_t)codepoint, u8(continuation_count + 1)};
}

}

std::optional<Token> decode_next(const u8*& cursor, const u8* end) {
    if (cursor == end) {
        return std::nullopt;
    }
    const u8 lead = *cursor;
    if (lead <= ControlLimit) {
        ++cursor;
        return Control{lead};
    }
    if (lead < 0x80) {
        ++cursor;
        return Codepoint{lead};
    }
    if (lead == WitnessIconByte) {
        ++cursor;
        return Icon{(u16)UnitId::ProtossWitness};
    }
    if (const auto decoded = decode_utf8(cursor, end)) {
        cursor += decoded->length;
        return Codepoint{decoded->codepoint};
    }
    if (ExtendedColorFirst <= lead && lead <= ExtendedColorLast && lead != TrademarkByte) {
        ++cursor;
        return Control{lead};
    }
    ++cursor;
    return Codepoint{legacy_codepoint(lead)};
}

bool is_cursor_pair(const u8* token_start, const u8* token_end, const u8* view_start) {
    return token_end - token_start == 1
        && *token_start == g_font_invisible_marker
        && token_start > view_start
        && token_start[-1] == CursorPrefixByte;
}

} // namespace text