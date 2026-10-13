#include "measure.h"

#include "BW/bwdata.h"
#include "BW/enumerations.h"
#include "decoder.h"
#include "systems/graphics/grp.h"
#include "text.h"

#include <cassert>
#include <utility>
#include <variant>
#include <vector>

namespace text {
namespace {

// Sums advances from cursor until a newline or the end of input
// cursor advances with the scan
s32 measure_line(u8 slot, const u8*& cursor, const u8* end) {
    TextSystem& text_system = TextSystem::instance();
    assert(text_system.available());

    const SlotConfig& config = text_system.slot(slot);
    FontStack& stack = text_system.stack(config.stack);
    Shaper& shaper = text_system.shaper();
    const s32 spacing = text_system.char_spacing();

    s32 width = 0;
    std::vector<char32_t> run;
    FontFace* run_face = nullptr;

    const auto flush_run = [&]() {
        if (!run.empty() && run_face) {
            for (const ShapedGlyph& glyph : shaper.shape(*run_face, config.pixel_height, run)) {
                width += glyph.advance + spacing;
            }
        }
        run.clear();
    };

    bool at_newline = false;
    while (!at_newline) {
        const auto token = decode_next(cursor, end);
        if (!token) {
            break;
        }
        std::visit(Overloaded{
            [&](const Codepoint& codepoint) {
                FontFace* face = stack.resolve(codepoint.value);
                if (face != run_face) {
                    flush_run();
                    run_face = face;
                }
                run.push_back(codepoint.value);
            },
            [&](const Control& control) {
                flush_run();
                switch (control.value) {
                    case '\t': {
                        width += text_system.tab_advance(slot);
                    } break;
                    case ' ': {
                        width += text_system.space_advance(slot);
                    } break;
                    case '\n':
                    case '\f':     // 0x0C ends the print in vanilla, so does 0x00
                    case '\0':
                        at_newline = true;
                        break;
                    case std::to_underlying(TextColor::PlayerColor):
                    case std::to_underlying(TextColor::PlayerColor2): {
                        if (cursor != end) {
                            const u8 code = *cursor;
                            if ((code >= 1 && code <= 8) || (code >= 0x0E && code <= 0x11)) {
                                ++cursor; // valid player color pair; the payload byte renders nothing
                            }
                        }
                    } break;
                }
            },
            [&](const Icon& icon) {
                flush_run();
                width += graphics::unit_icon_advance(icon.unit_id, config.pixel_height, spacing);
            },
        }, *token);
    }
    flush_run();
    return width;
}

}

s32 line_width(u8 slot, std::string_view text) {
    const u8* scan = (const u8*)text.data();
    const u8* end = scan + text.size();
    return measure_line(slot, scan, end);
}

s32 text_width(u8 slot, std::string_view text) {
    s32 width = 0;
    const u8* cursor = (const u8*)text.data();
    const u8* end = cursor + text.size();
    while (cursor != end) {
        width += measure_line(slot, cursor, end);
        if (cursor == end || cursor[-1] == '\f' || cursor[-1] == '\0') {
            break; // 0x00 and 0x0C end the print in vanilla
        }
        // measure_line already consumed the '\n'; the next line starts at cursor
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