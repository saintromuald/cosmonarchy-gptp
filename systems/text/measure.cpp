#include "measure.h"

#include "BW/bwdata.h"
#include "text.h"
#include "walk.h"

#include <optional>

namespace text {

s32 line_width(u8 slot, std::string_view text, s32 x_start, s32 x_origin) {
    return line_width(TextSystem::instance().font_for_slot(slot), text, x_start, x_origin);
}

s32 line_width(const PrintFont& font, std::string_view text, s32 x_start, s32 x_origin) {
    const u8* start = (const u8*)text.data();
    const u8* end = start + text.size();
    const u8* cursor = start;
    const auto sink = [](const Atom&) {
        return true;
    };
    const WalkSummary summary = walk_line({font, start, x_start, x_origin}, cursor, end, sink);
    return summary.x - x_start;
}

s32 text_width(u8 slot, std::string_view text) {
    const u8* start = (const u8*)text.data();
    const u8* end = start + text.size();
    const u8* cursor = start;
    s32 width = 0;
    const PrintFont font = TextSystem::instance().font_for_slot(slot);
    const auto sink = [](const Atom&) {
        return true;
    };
    while (cursor < end) {
        const WalkSummary summary = walk_line({font, start, 0, 0}, cursor, end, sink);
        width += summary.x;
        if (summary.stop != WalkStop::Newline) {
            break; // 0x00 and 0x0C end the print in vanilla
        }
        cursor = summary.cursor;
    }
    return width;
}

s32 text_height(u8 slot, std::string_view text) {
    const s32 advance = TextSystem::instance().line_advance(slot);
    s32 lines = 1;
    for (const char c : text) {
        if (c == '\f' || c == '\0') {
            break; // 0x00 and 0x0C end the print in vanilla
        }
        if (c == '\n') {
            ++lines;
        }
    }
    return lines * advance;
}

std::optional<u8> slot_of(const graphics::Font* font) {
    for (u8 slot = 0; slot < 4; ++slot) {
        if (font_base[slot] == font) {
            return slot;
        }
    }
    return std::nullopt;
}

} // namespace text
