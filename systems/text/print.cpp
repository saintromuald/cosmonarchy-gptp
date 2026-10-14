#include "print.h"

#include "BW/bwdata.h"
#include "graphics/Bitmap.h"
#include "logger.h"
#include "measure.h"
#include "plot.h"
#include "player_color.h"
#include "text.h"
#include "walk.h"
#include "wrap.h"

#include <algorithm>
#include <cstring>
#include <format>
#include <optional>
#include <utility>

namespace text {
namespace {

// Alignment byte bits (decoded from bw_print_xy 0x004202B0 / adjust_print_y_vertical_align 0x0041F270 / font_print_text_lines).
constexpr u8 AlignCenterBit = 0x02;
constexpr u8 AlignRightBit = 0x04;
constexpr u8 AlignAdaptiveBit = 0x08;
constexpr u8 AlignMeasureMask = 0x68;             // bits 3|5|6 run the measure pass
constexpr u8 AlignNoVerticalAdjust = 0x10;
constexpr u8 AlignVerticalCenter = 0x20;

// The vanilla alignment byte (decoded from bw_print_xy 0x004202B0 / adjust_print_y_vertical_align 0x0041F270):
// horizontal bits 0x02/0x04/0x08, bit 0x10 = no vertical adjust, bit 0x20 halves it (vertical center), bit 0x40 triggers the measure pass for bottom-align.
u8 align_byte(HAlign h, VAlign v) {
    u8 bits = 0x10;
    switch (h) {
        case HAlign::Center:   bits |= 0x02; break;
        case HAlign::Right:    bits |= 0x04; break;
        case HAlign::Adaptive: bits |= 0x08; break;
        case HAlign::Left:     break;
    }
    switch (v) {
        case VAlign::Center: bits = u8((bits & ~0x10) | 0x20); break;
        case VAlign::Bottom: bits = u8((bits & ~0x10) | 0x40); break;
        case VAlign::None:   break;
    }
    return bits;
}

struct PrintParams {
    PrintFont font;
    Box32 clip;
    s32 x;
    s32 y;
    u8 alignment;
    bool word_wrap;
    const u8* color;
    const u8* saved_color;
};

struct PrintOutcome {
    const u8* remaining = nullptr;
    u16 line_count = 0;
    Box32 bounds = {};
    s32 final_y = 0;
    const u8* color = nullptr;
    const u8* saved_color = nullptr;
};

struct CursorRecord {
    s32 x;
    const u8* ramp;
};

struct LineRenderer {
    graphics::Bitmap& bitmap;
    Box32 clip;
    GlyphCache& glyph_cache;
    u32 pixel_height;
    s32 ascent;
    s32 line_advance;
    s32 line_y = 0;
    const u8* color;
    const u8* saved_color;
    const u8* grey_ramp;
    Box32 bounds;
    std::optional<CursorRecord> cursor;
    u8 pending_align = 0;

    bool operator()(const Atom& atom) {
        switch (atom.kind) {
            case AtomKind::Glyph: {
                const GlyphBitmap& glyph = glyph_cache.lookup(*atom.face, pixel_height, atom.glyph_index);
                const s32 x = atom.x - atom.advance + atom.offset_x + glyph.bitmap_left;
                const s32 y = line_y + atom.offset_y + ascent - glyph.bitmap_top;
                plot_glyph(bitmap, clip, glyph, x, y, color);
                bounds.left = std::min<s32>(bounds.left, x);
                bounds.right = std::max<s32>(bounds.right, x + s32(glyph.width));
                bounds.bottom = std::max<s32>(bounds.bottom, y + s32(glyph.height));
                return true;
            }
            case AtomKind::Icon: {
                const s32 x = atom.x - atom.advance;
                const graphics::FrameSize size = plot_icon(bitmap, clip, atom.unit_id, x, line_y, line_advance, pixel_height, color);
                bounds.left = std::min<s32>(bounds.left, x);
                bounds.right = std::max<s32>(bounds.right, x + s32(size.width));
                bounds.bottom = std::max<s32>(bounds.bottom, line_y + (line_advance + s32(size.height)) / 2);
                return true;
            }
            case AtomKind::Cursor: {
                cursor = {atom.x, color};
                return true;
            }
            case AtomKind::Space:
            case AtomKind::Tab:
                return true;
            case AtomKind::PlayerColor: {
                if (color == grey_ramp || atom.payload == 0) {
                    return true;
                }
                saved_color = color;
                set_player_font_ramp(player_font_ramp_row(atom.payload));
                color = ramp(PlayerFontSlot);
                return true;
            }
            case AtomKind::Control: {
                switch (atom.control) {
                    case std::to_underlying(TextColor::AlignRight):
                    case std::to_underlying(TextColor::AlignCenter):
                        pending_align = atom.control;
                        return false;
                    case '\r':
                        return true;
                    default:
                        // Grey swallows the legacy codes only; the extended colors
                        // bypass the grey check (writeFontApplyColour's off_4 jump).
                        if (color == grey_ramp && atom.control < 0x20) {
                            return true;
                        }
                        if (atom.control == std::to_underlying(TextColor::Revert)) {
                            // The player ramp is rewritten by every pair, so a saved
                            // pointer onto it is stale; get_color_handle escapes to the
                            // next ramp row instead of re-applying the wrong shades.
                            if (saved_color == ramp(PlayerFontSlot)) {
                                saved_color += 8;
                            }
                            color = saved_color;
                        } else {
                            saved_color = color;
                            color = ramp(color_slot(atom.control));
                        }
                        return true;
                }
            }
        }
        return true;
    }
};

void grow_draw_area_bounds(const Box32& bounds, const Box32& clip) {
    Box32 grown = bounds;
    grown.left = std::max<s32>(grown.left - 2, clip.left);
    grown.top = std::max<s32>(grown.top - 2, clip.top);
    grown.right = std::min<s32>(grown.right + 2, clip.right);
    grown.bottom = std::min<s32>(grown.bottom + 2, clip.bottom);
    g_draw_area_bounds.left = s16(grown.left);
    g_draw_area_bounds.top = s16(grown.top);
    g_draw_area_bounds.right = s16(grown.right);
    g_draw_area_bounds.bottom = s16(grown.bottom);
}

// find_line_break returns the line start unchanged when the first word (word wrap) or the first glyph (char wrap) alone overflows
// Retry in char-wrap mode, then consume a single overflowing atom, so a print always advances.
LineBreak next_line_break(const PrintFont& font, const u8* start, const u8* end, s32 x_start, s32 x_origin, s32 clip_right, bool word_wrap) {
    LineBreak line = find_line_break(font, start, end, x_start, x_origin, clip_right, word_wrap);
    if (line.stop != WalkStop::Sink || line.break_at != start) {
        return line;
    }
    if (word_wrap) {
        line = find_line_break(font, start, end, x_start, x_origin, clip_right, false);
        if (line.stop != WalkStop::Sink || line.break_at != start) {
            return line;
        }
    }
    const WalkSummary forced = walk_line({font, start, x_start, x_origin}, start, end, [](const Atom&) { return false; });
    return {forced.cursor, forced.x - x_start, forced.stop};
}

PrintOutcome print_core(graphics::Bitmap& surface, const PrintParams& params, std::string_view text) {
    TextSystem& text_system = TextSystem::instance();
    const PrintFont& font = params.font;
    FontStack& stack = text_system.stack(font.stack);
    FontFace* primary_face = stack.primary_face();
    const s32 line_advance = font.line_advance;
    const s32 font_height = font.font_height;
    const s32 scaled_gap = font.line_gap;

    const u8* start = (const u8*)text.data();
    const u8* end = start + text.size();

    PrintOutcome outcome;
    outcome.remaining = end;
    outcome.color = params.color;
    outcome.saved_color = params.saved_color;

    if (!primary_face
        || params.x < params.clip.left
        || params.y < params.clip.top
        || params.x > params.clip.right
        || params.y + font_height > params.clip.bottom) {
        outcome.bounds = {params.x, params.y, params.x, params.y};
        outcome.final_y = params.y;
        return outcome;
    }

    u8 alignment = params.alignment;
    s32 y = params.y;

    if (alignment & AlignMeasureMask) {
        // Measure pass: how many lines fit and how much height they span.
        u16 lines = 0;
        bool page_ended = false; // '\f' ends the print without the gap unwind
        const u8* cursor = start;
        s32 line_x = params.x;
        while (cursor < end) {
            const LineBreak line = next_line_break(font, cursor, end, line_x, params.clip.left, params.clip.right, params.word_wrap);
            ++lines;
            y += line_advance;
            if (line.stop == WalkStop::PrintEnd) {
                page_ended = line.break_at[-1] == '\f';
                break;
            }
            if (line.stop == WalkStop::End) {
                break;
            }
            if (y + font_height > params.clip.bottom) {
                break;
            }
            line_x = params.clip.left; // vanilla starts every line after the first at the clip's left
            cursor = line.break_at;
        }
        if (!page_ended) {
            y -= scaled_gap;
        }
        if (!(alignment & AlignNoVerticalAdjust)) {
            // Center/bottom the VISIBLE text band, the analog of vanilla
            // centering its bitmap-ink box: the band is [ascent - cap, ascent]
            // (cap = the 'O' bitmap top), measured from the same anchors the
            // renderer plots with. The .fnt metric box and the em both fail
            // here: the regenerated fonts' yMax is already 2x-authored
            // (font16: yMax 19), so the metric box double-scales, and the em
            // band is taller than the visible ink (the lightup-button offset).
            // band_height is the band mirrored below the baseline (line-relative
            // [0, 2*ascent - cap]) so the centering halves work.
            const s32 band_height = 2 * font.ascent - font.cap_height;
            s32 printed = page_ended
                ? s32(lines) * line_advance
                : (s32(lines) - 1) * line_advance + band_height;
            s32 available = params.clip.bottom - params.y;
            if (alignment & AlignVerticalCenter) {
                printed /= 2;
                available /= 2;
            }
            y = (params.y - printed) + available;
        }
        if (alignment & AlignAdaptiveBit) {
            const u16 count = lines + (page_ended ? 1 : 0);
            alignment = count <= 1 ? u8(alignment ^ 0x0A) : u8(alignment ^ 0x09);
        }
    }

    LineRenderer renderer{
        .bitmap = surface,
        .clip = params.clip,
        .glyph_cache = text_system.glyph_cache(),
        .pixel_height = font.pixel_height,
        .ascent = font.ascent,
        .line_advance = line_advance,
        .color = params.color,
        .saved_color = params.saved_color,
        .grey_ramp = ramp(GreyColorSlot),
        .bounds = {params.x, params.y, params.x, params.y},
    };

    u16 lines_printed = 0;
    bool page_ended = false;
    const u8* cursor = start;
    s32 line_x = params.x;
    while (cursor < end) {
        const LineBreak line = next_line_break(font, cursor, end, line_x, params.clip.left, params.clip.right, params.word_wrap);

        s32 x_line = line_x;
        if (alignment & AlignCenterBit) {
            x_line = (params.clip.right - line_x) / 2 - line.width / 2 + line_x;
        } else if (alignment & AlignRightBit) {
            x_line = params.clip.left + params.clip.right - line.width - line_x;
        }

        renderer.line_y = y;
        renderer.cursor.reset();
        const u8* walk_cursor = cursor;
        s32 walk_x = x_line;
        while (true) {
            const WalkSummary summary = walk_line({font, start, walk_x, params.clip.left}, walk_cursor, line.break_at, renderer);
            if (summary.stop != WalkStop::Sink) {
                break;
            }
            // In-band align (0x12/0x13): shift by the slack of the unprinted remainder.
            const std::string_view rest{(const char*)summary.cursor, size_t(line.break_at - summary.cursor)};
            const s32 slack = params.clip.right - summary.x - line_width(font, rest, summary.x, params.clip.left);
            walk_x = summary.x + (renderer.pending_align == std::to_underlying(TextColor::AlignCenter) ? slack / 2 : slack);
            walk_cursor = summary.cursor;
            renderer.pending_align = 0;
        }

        // The cursor plots last so it lands on top of the glyph it overlays.
        if (renderer.cursor) {
            plot_cursor(surface, params.clip, font, renderer.cursor->x, y, renderer.cursor->ramp);
            renderer.bounds.left = std::min<s32>(renderer.bounds.left, renderer.cursor->x);
            renderer.bounds.right = std::max<s32>(renderer.bounds.right, renderer.cursor->x + cursor_width(font));
            renderer.bounds.bottom = std::max<s32>(renderer.bounds.bottom, y + s32(font.pixel_height));
        }

        ++lines_printed;
        cursor = line.break_at;
        y += line_advance;

        if (line.stop == WalkStop::PrintEnd) {
            page_ended = line.break_at[-1] == '\f';
            outcome.remaining = page_ended ? line.break_at : line.break_at - 1;
            break;
        }
        if (line.stop == WalkStop::End) {
            outcome.remaining = line.break_at;
            break;
        }
        if (y + font_height > params.clip.bottom) {
            outcome.remaining = cursor;
            break;
        }
        line_x = params.clip.left;
    }

    if (!page_ended) {
        y -= scaled_gap;
    }

    outcome.line_count = lines_printed;
    outcome.final_y = y;
    outcome.bounds = renderer.bounds;
    outcome.color = renderer.color;
    outcome.saved_color = renderer.saved_color;
    return outcome;
}

}

PrintResult print(std::string_view text, s32 x, s32 y) {
    return print(text, x, y, PrintOptions{});
}

PrintResult print(std::string_view text, s32 x, s32 y, const PrintOptions& options) {
    graphics::Bitmap* target = options.m_target ? options.m_target : g_compositor_active_draw_target;
    TextSystem& text_system = TextSystem::instance();
    if (!target) {
        return {};
    }

    PrintParams params;
    params.font = options.m_px ? text_system.font_for_px(options.m_px) : text_system.font_for_slot(options.m_slot);
    params.clip = options.m_clip ? *options.m_clip
        : Box32{0, 0, s32(target->width) - 1, s32(target->height) - 1};
    params.x = x;
    params.y = y;
    params.alignment = align_byte(options.m_halign, options.m_valign);
    params.word_wrap = options.m_word_wrap;
    if (options.m_color && is_color_code(std::to_underlying(*options.m_color))) {
        params.color = ramp(color_slot(std::to_underlying(*options.m_color)));
        params.saved_color = params.color;
    } else {
        if (options.m_color) {
            static bool logged_non_color = false;
            if (!logged_non_color) {
                logged_non_color = true;
                gptp::g_logger << std::format("print: {} is not a color code, using the ambient print color\n",
                    magic_enum::enum_name(*options.m_color));
            }
        }
        // Unspecified inherits the ambient print color state, like every vanilla
        // caller: they set it via get_color_handle (or point it at a custom
        // ramp) before printing.
        params.color = (const u8*)g_current_font_color;
        params.saved_color = (const u8*)g_saved_font_color;
    }

    const PrintOutcome outcome = print_core(*target, params, text);

    PrintResult result;
    result.remaining = {(const char*)outcome.remaining, size_t((const u8*)text.data() + text.size() - outcome.remaining)};
    result.line_count = outcome.line_count;
    result.bounds = outcome.bounds;
    result.final_y = outcome.final_y;
    return result;
}

const char* print_vanilla(const char* text, s32 x, s32 y) {
    graphics::Bitmap* surface = g_compositor_active_draw_target;
    if (!surface || !text) {
        return nullptr;
    }

    // Clamp the clip box to the surface (bw_print_xy's entry block writes the clamped values back).
    Box32 clip{
        std::max<s32>(g_draw_text.left, 0),
        std::max<s32>(g_draw_text.top, 0),
        std::min<s32>(g_draw_text.right, s32(surface->width) - 1),
        std::min<s32>(g_draw_text.bottom, s32(surface->height) - 1),
    };
    g_draw_text.left = s16(clip.left);
    g_draw_text.top = s16(clip.top);
    g_draw_text.right = s16(clip.right);
    g_draw_text.bottom = s16(clip.bottom);

    const graphics::Font* font = g_current_font;
    if (!font) {
        return nullptr;
    }

    u8 slot = DefaultSlot;
    if (const std::optional<u8> found = slot_of(font)) {
        slot = *found;
    } else {
        static const graphics::Font* logged = nullptr;
        if (font != logged) {
            logged = font;
            gptp::g_logger << std::format("print_vanilla: non-slot font {:p} renders at slot {}\n", (void*)font, DefaultSlot);
        }
    }

    PrintParams params;
    params.font = TextSystem::instance().font_for_slot(slot);
    params.clip = clip;
    params.x = x;
    params.y = y;
    params.alignment = g_print_xy_alignment;
    params.word_wrap = g_word_wrap_mode != 0;
    params.color = (const u8*)g_current_font_color;
    params.saved_color = (const u8*)g_saved_font_color;

    const PrintOutcome outcome = print_core(*surface, params, {text, strlen(text)});

    g_current_text_x = clip.left;
    g_current_text_y = outcome.final_y;
    g_current_font_color = (u32)outcome.color;
    g_saved_font_color = (u32)outcome.saved_color;
    grow_draw_area_bounds(outcome.bounds, clip);
    return *outcome.remaining ? (const char*)outcome.remaining : nullptr;
}

} // namespace text