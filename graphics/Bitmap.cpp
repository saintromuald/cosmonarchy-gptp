//Based on BWAPI's BW/Bitmap.cpp
#include "Bitmap.h"
#include "../BW/bwdata.h"
#include "BW/enumerations.h"
#include "systems/graphics/grp.h"
#include "systems/text/decoder.h"
#include "systems/text/measure.h"
#include "systems/text/plot.h"
#include "systems/text/text.h"
#include "systems/text/walk.h"
#include "systems/text/wrap.h"
#include <cassert>
#include <utility>
#include <string_view>
#include <vector>
#include <algorithm>
#include <player_color.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

namespace graphics
{

    u16 Bitmap::getWidth() const
    {
        assert(this);
        return this->width;
    }

    u16 Bitmap::getHeight() const
    {
        assert(this);
        return this->height;
    }

namespace {

bool draw_text_ttf(Bitmap& bitmap, std::string_view text, s32 x, s32 y, u8 slot) {
    text::TextSystem& text_system = text::TextSystem::instance();
    const text::PrintFont font = text_system.font_for_slot(slot);
    text::FontStack& stack = text_system.stack(font.stack);
    text::Shaper& shaper = text_system.shaper();
    text::GlyphCache& glyph_cache = text_system.glyph_cache();
    const u32 pixel_height = font.pixel_height;
    const s32 spacing = font.char_spacing;
    const Box32 bitmap_clip{0, 0, s32(bitmap.width) - 1, s32(bitmap.height) - 1};

    if (x + text::text_width(slot, text) < 0
        || y + text::text_height(slot, text) < 0
        || x >= bitmap.getWidth() || y >= bitmap.getHeight()) {
        return false;
    }

    text::FontFace* primary_face = stack.primary_face();
    if (!primary_face) {
        return false;
    }
    const s32 ascent = font.ascent;

    const u8* last_color = text::ramp(0);
    const u8* color = text::ramp(0);
    s32 x_offset = x;

    text::WalkScratch& scratch = text_system.walk_scratch();
    std::vector<char32_t>& run = scratch.run_codepoints;
    std::vector<text::ShapedGlyph>& shaped = scratch.shaped;
    run.clear();
    text::FontFace* run_face = nullptr;

    const auto flush_run = [&]() {
        if (run.empty() || !run_face) {
            run.clear();
            return;
        }
        shaper.shape_into(shaped, *run_face, pixel_height, run);
        for (const text::ShapedGlyph& glyph : shaped) {
            const text::GlyphBitmap& glyph_bitmap = glyph_cache.lookup(*run_face, pixel_height, glyph.glyph_index);
            const s32 glyph_x = x_offset + glyph.offset_x + glyph_bitmap.bitmap_left;
            const s32 glyph_y = y + glyph.offset_y + ascent - glyph_bitmap.bitmap_top;
            text::plot_glyph(bitmap, bitmap_clip, glyph_bitmap, glyph_x, glyph_y, color);
            x_offset += glyph.advance + spacing;
        }
        run.clear();
    };

    const u8* cursor = reinterpret_cast<const u8*>(text.data());
    const u8* end = cursor + text.size();
    const u8* view_start = cursor;
    bool print_ended = false;
    while (!print_ended) {
        const u8* token_start = cursor;
        const auto token = text::decode_next(cursor, end);
        if (!token) {
            break;
        }
        std::visit(Overloaded{
            [&](const text::Codepoint& codepoint) {
                if (text::is_cursor_pair(token_start, cursor, view_start)) {
                    // flush first so preceding glyphs plot before the cursor (vanilla z-order)
                    flush_run();
                    run_face = nullptr;
                    text::plot_cursor(bitmap, bitmap_clip, font, x_offset, y, color);
                    return;
                }
                text::FontFace* face = stack.resolve(codepoint.value, run_face);
                if (face != run_face) {
                    flush_run();
                    run_face = face;
                }
                run.push_back(codepoint.value);
            },
            [&](const text::Control& control) {
                flush_run();
                switch (control.value) {
                    case 0: {        // vanilla ends the print at 0x00 like 0x0C
                        print_ended = true;
                    } break;
                    case std::to_underlying(TextColor::Revert): {
                        if (color == text::ramp(text::GreyColorSlot)) {
                            break;
                        }
                        // The player ramp is rewritten by every pair, so a saved
                        // pointer onto it is stale; get_color_handle escapes to the
                        // next ramp row instead of re-applying the wrong shades.
                        if (last_color == text::ramp(PlayerFontSlot)) {
                            last_color += 8;
                        }
                        color = last_color;
                    } break;
                    case '\t': {
                        x_offset += text::tab_stop(font.tab_width, x_offset, x);
                    } break;
                    case '\n': {
                        x_offset = x;
                        y += font.line_advance;
                    } break;
                    case '\f': {    // vanilla ends the whole print at 0x0C
                        print_ended = true;
                    } break;
                    case '\r':
                        break;
                    case std::to_underlying(TextColor::AlignRight): {
                        const std::string_view rest{reinterpret_cast<const char*>(cursor), size_t(end - cursor)};
                        x_offset += bitmap.getWidth() - text::line_width(slot, rest, x_offset, x) - x;
                    } break;
                    case std::to_underlying(TextColor::AlignCenter): {
                        const std::string_view rest{reinterpret_cast<const char*>(cursor), size_t(end - cursor)};
                        x_offset += (bitmap.getWidth() - text::line_width(slot, rest, x_offset, x)) / 2 - x;
                    } break;
                    case ' ': {
                        x_offset += font.space_advance;
                    } break;
                    case std::to_underlying(TextColor::PlayerColor):
                    case std::to_underlying(TextColor::PlayerColor2): {
                        if (color == text::ramp(text::GreyColorSlot)) {
                            break;
                        }
                        if (cursor != end) {
                            const u8 code = *cursor;
                            if ((code >= 1 && code <= 8) || (code >= 0x0E && code <= 0x11)) {
                                ++cursor;
                                last_color = color;
                                set_player_font_ramp(player_font_ramp_row(code));
                                color = text::ramp(PlayerFontSlot);
                            }
                        }
                    } break;
                    default: {      // colour code, including the extended 0x8F-0x9F range
                        // Grey swallows the legacy codes only; the extended colors
                        // bypass the grey check (writeFontApplyColour's off_4 jump).
                        if (color == text::ramp(text::GreyColorSlot) && control.value < 0x20) {
                            break;
                        }
                        last_color = color;
                        color = text::ramp(text::color_slot(control.value));
                    } break;
                }
            },
            [&](const text::Icon& icon) {
                flush_run();
                const FrameSize icon_size = text::plot_icon(bitmap, bitmap_clip, icon.unit_id, x_offset, y, font.line_advance, pixel_height, color);
                x_offset += icon_size.height ? s32(icon_size.width) + spacing : 0;
            },
        }, *token);
    }
    flush_run();
    return true;
}

} // namespace

bool Bitmap::draw_text(std::string_view text, s32 x, s32 y, u8 size)
{
    if (size > 3) {
        return false;
    }
    return draw_text_ttf(*this, text, x, y, size);
}


    //-------- Dot and line drawing --------//

    void Bitmap::draw_dot(s32 x, s32 y, ColorId color)
    {
        if (x < 0 || x >= this->getWidth()) return;
        if (y < 0 || y >= this->getHeight()) return;
        this->drawDotUnsafe(x, y, color);
    }

    void Bitmap::draw_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color)
    {
        //If horizontal
        if (y1 == y2)
            this->drawHorizontalLine(x1, x2, y1, color);

        //If vertical
        else if (x1 == x2)
            this->drawVerticalLine(x1, y1, y2, color);

        //Trivial check
        if (this->isLineTriviallyIgnorable(x1, y1, x2, y2))
            return;

        //Use Bresenham's line algorithm
        //Code taken from http://members.chello.at/~easyfilter/bresenham.html
        else
        {
            const s32 dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
            const s32 dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
            s32 err = dx + dy;

            s32 x = x1, y = y1;
            this->draw_dot(x, y, color);
            while (x != x2 || y != y2)
            {
                const s32 e2 = err * 2;
                if (e2 >= dy)
                {
                    err += dy; x += sx;
                }
                if (e2 <= dx)
                {
                    err += dx; y += sy;
                }
                this->draw_dot(x, y, color);
            }
        }
    }

    void Bitmap::drawHorizontalLine(s32 x1, s32 x2, s32 y, ColorId color)
    {
        if (y < 0 || y >= this->getHeight()) return;
        if (x1 > x2) std::swap(x1, x2);
        if (x2 < 0 || x1 >= this->getWidth()) return;

        x1 = std::max(x1, 0);
        x2 = std::min(x2, this->getWidth() - 1);
        this->drawHorizontalLineUnsafe(x1, x2, y, color);
    }

    void Bitmap::drawVerticalLine(s32 x, s32 y1, s32 y2, ColorId color)
    {
        if (x < 0 || x >= this->getWidth()) return;
        if (y1 > y2) std::swap(y1, y2);
        if (y2 < 0 || y1 >= this->getHeight()) return;

        y1 = std::max(y1, 0);
        y2 = std::min(y2, this->getHeight() - 1);
        this->drawVerticalLineUnsafe(x, y1, y2, color);
    }


    //-------- Box drawing --------//

    void Bitmap::draw_box(s32 left, s32 top, s32 right, s32 bottom, ColorId color)
    {
        if (left > right) std::swap(left, right);
        if (right < 0 || left >= this->getWidth()) return;

        if (top > bottom) std::swap(top, bottom);
        if (bottom < 0 || top >= this->getHeight()) return;

        //Draw horizontal part
        const s32 xMin = std::max(left, 0);
        const s32 xMax = std::min(right, this->getWidth() - 1);
        if (top >= 0)
            this->drawHorizontalLineUnsafe(xMin, xMax, top, color);
        if (bottom < this->getHeight())
            this->drawHorizontalLineUnsafe(xMin, xMax, bottom, color);

        //Draw vertical part
        const s32 yMin = std::max(top, 0);
        const s32 yMax = std::min(bottom, this->getHeight() - 1);
        if (left >= 0)
            this->drawVerticalLineUnsafe(left, yMin, yMax, color);
        if (right < this->getWidth())
            this->drawVerticalLineUnsafe(right, yMin, yMax, color);
    }

    void Bitmap::draw_filled_box(s32 left, s32 top, s32 right, s32 bottom,
        ColorId color)
    {
        if (left > right) std::swap(left, right);
        if (right < 0 || left >= this->getWidth()) return;

        if (top > bottom) std::swap(top, bottom);
        if (bottom < 0 || top >= this->getHeight()) return;

        left = std::max(left, 0);
        right = std::min(right, this->getWidth() - 1);
        top = std::max(top, 0);
        bottom = std::min(bottom, this->getHeight() - 1);

        for (s32 y = top; y <= bottom; ++y)
            this->drawHorizontalLineUnsafe(left, right, y, color);
    }


    //-------- Circle drawing --------//

    void Bitmap::draw_circle(s32 x, s32 y, s32 radius, ColorId color)
    {
        if (radius <= 0) return;

        //Bresenham's circle algorithm
        //Code taken from http://members.chello.at/easyfilter/bresenham.html  
        s32 px = -radius, py = 0, err = 2 - 2 * radius; /* II. Quadrant */
        do
        {
            this->draw_dot(x - px, y + py, color); /*   I. Quadrant */
            this->draw_dot(x - py, y - px, color); /*  II. Quadrant */
            this->draw_dot(x + px, y - py, color); /* III. Quadrant */
            this->draw_dot(x + py, y + px, color); /*  IV. Quadrant */
            const s32 r = err;
            if (r <= py) err += ++py * 2 + 1;            /* e_xy+e_y < 0 */
            if (r > px || err > py) err += ++px * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
        }
        while (px < 0);
    }

    void Bitmap::draw_filled_circle(s32 x, s32 y, s32 radius, ColorId color)
    {
        if (radius <= 0) return;

        //Bresenham's circle algorithm, modified
        //Code taken from http://members.chello.at/easyfilter/bresenham.html  
        s32 px = -radius, py = 0, err = 2 - 2 * radius; /* II. Quadrant */
        do
        {
            this->drawHorizontalLine(x + px, x - px, y + py, color);  //Quadrants 1, 2
            this->drawHorizontalLine(x + px, x - px, y - py, color);  //Quadrants 3, 4
            const s32 r = err;
            if (r <= py) err += ++py * 2 + 1;            /* e_xy+e_y < 0 */
            if (r > px || err > py) err += ++px * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
        }
        while (px < 0);
    }

    //-------- Ellipse drawing --------//

    void Bitmap::draw_ellipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color)
    {

        if (left > right) std::swap(left, right);
        if (right <= 0 || left >= this->getWidth()) return;

        if (top > bottom) std::swap(top, bottom);
        if (bottom <= 0 || top >= this->getHeight()) return;

        s32 x0 = left;
        s32 y0 = bottom;
        s32 x1 = right;
        s32 y1 = top;

        //Bresenham's circle algorithm
        //Code taken from http://members.chello.at/easyfilter/bresenham.html  

        s32 a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1; /* values of diameter */
        f64 dx = 4 * (1.0 - a) * b * b, dy = 4 * (b1 + 1) * a * a; /* error increment */
        f64 err = dx + dy + b1 * a * a, e2; /* error of 1.step */

        if (x0 > x1)
        {
            x0 = x1; x1 += a;
        } /* if called with swapped points */
        if (y0 > y1) y0 = y1; /* .. exchange them */
        y0 += (b + 1) / 2; y1 = y0 - b1;   /* starting pixel */
        a = 8 * a * a; b1 = 8 * b * b;

        do
        {
            this->draw_dot(x1, y0, color); /*   I. Quadrant */
            this->draw_dot(x0, y0, color); /*  II. Quadrant */
            this->draw_dot(x0, y1, color); /* III. Quadrant */
            this->draw_dot(x1, y1, color); /*  IV. Quadrant */
            e2 = 2 * err;
            if (e2 <= dy)
            {
                y0++; y1--; err += dy += a;
            }  /* y step */
            if (e2 >= dx || 2 * err > dy)
            {
                x0++; x1--; err += dx += b1;
            } /* x step */
        }
        while (x0 <= x1);

        while (y0 - y1 < b)
        {  /* too early stop of flat ellipses a=1 */
            this->draw_dot(x0 - 1, y0, color); /* -> finish tip of ellipse */
            this->draw_dot(x1 + 1, y0++, color);
            this->draw_dot(x0 - 1, y1, color);
            this->draw_dot(x1 + 1, y1--, color);
        }

    }

    //-------- Dotted line drawing --------//

    void Bitmap::draw_dotted_line(s32 x1, s32 y1, s32 x2, s32 y2, ColorId color)
    {

        s32 nIgnoreHalf = 0;

        const s32 dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
        const s32 dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
        s32 err = dx + dy;

        s32 x = x1, y = y1;
        if (nIgnoreHalf % 20 < 5)
            this->draw_dot(x, y, color);
        nIgnoreHalf++;
        while (x != x2 || y != y2)
        {
            const s32 e2 = err * 2;
            if (e2 >= dy)
            {
                err += dy; x += sx;
            }
            if (e2 <= dx)
            {
                err += dx; y += sy;
            }
            if (nIgnoreHalf % 20 < 5)
                this->draw_dot(x, y, color);
            nIgnoreHalf++;
        }
    }

    //-------- Dotted Ellipse drawing --------//

    void Bitmap::draw_dotted_elipse(s32 left, s32 top, s32 right, s32 bottom, ColorId color)
    {

        s32 nIgnoreHalf = 0;

        if (left > right) std::swap(left, right);
        if (right <= 0 || left >= this->getWidth()) return;

        if (top > bottom) std::swap(top, bottom);
        if (bottom <= 0 || top >= this->getHeight()) return;

        s32 x0 = left;
        s32 y0 = bottom;
        s32 x1 = right;
        s32 y1 = top;

        //Bresenham's circle algorithm
        //Code taken from http://members.chello.at/easyfilter/bresenham.html  

        s32 a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1; /* values of diameter */
        f64 dx = 4 * (1.0 - a) * b * b, dy = 4 * (b1 + 1) * a * a; /* error increment */
        f64 err = dx + dy + b1 * a * a, e2; /* error of 1.step */

        if (x0 > x1)
        {
            x0 = x1; x1 += a;
        } /* if called with swapped points */
        if (y0 > y1) y0 = y1; /* .. exchange them */
        y0 += (b + 1) / 2; y1 = y0 - b1;   /* starting pixel */
        a = 8 * a * a; b1 = 8 * b * b;

        do
        {

            if (nIgnoreHalf % 20 < 5)
            {
                this->draw_dot(x1, y0, color); /*   I. Quadrant */
                this->draw_dot(x0, y0, color); /*  II. Quadrant */
                this->draw_dot(x0, y1, color); /* III. Quadrant */
                this->draw_dot(x1, y1, color); /*  IV. Quadrant */
            }

            nIgnoreHalf++;

            e2 = 2 * err;

            if (e2 <= dy)
            {
                y0++; y1--; err += dy += a;
            }  /* y step */
            if (e2 >= dx || 2 * err > dy)
            {
                x0++; x1--; err += dx += b1;
            } /* x step */

        }
        while (x0 <= x1);

        while (y0 - y1 < b)
        {  /* too early stop of flat ellipses a=1 */

            if (nIgnoreHalf % 20 < 5)
            {
                this->draw_dot(x0 - 1, y0, color); /* -> finish tip of ellipse */
                this->draw_dot(x1 + 1, y0++, color); //this->draw_dot(x1+1, y0++, color); 
                this->draw_dot(x0 - 1, y1, color);
                this->draw_dot(x1 + 1, y1--, color);//this->draw_dot(x1+1, y1--, color); 
            }
            else
            {
                y0++; y1--;
            }

            nIgnoreHalf++;

        }

    }

    //-------- Unsafe functions for fast drawing --------//

    void Bitmap::drawDotUnsafe(s32 x, s32 y, ColorId color)
    {
        assert(0 <= x && x < this->getWidth());
        assert(0 <= y && y < this->getHeight());

        this->data[y * this->getWidth() + x] = color;
    }

    void Bitmap::drawHorizontalLineUnsafe(s32 x1, s32 x2, s32 y, ColorId color)
    {
        assert(0 <= x1 && x1 < this->getWidth());
        assert(0 <= x2 && x2 < this->getWidth());
        assert(0 <= y && y < this->getHeight());
        assert(x1 <= x2);

        memset(&this->data[y * this->getWidth() + x1], color, x2 - x1 + 1);
    }

    void Bitmap::drawVerticalLineUnsafe(s32 x, s32 y1, s32 y2, ColorId color)
    {
        assert(0 <= x && x < this->getWidth());
        assert(0 <= y1 && y1 < this->getHeight());
        assert(0 <= y2 && y2 < this->getHeight());
        assert(y1 <= y2);

        for (s32 y = y1; y <= y2; ++y)
            this->drawDotUnsafe(x, y, color);
    }


    //-------- Line clipping --------//

    s32 Bitmap::computeOutcode(s32 x, s32 y)
    {
        const s32 OUTCODE_INSIDE = 0;  //0000
        const s32 OUTCODE_LEFT = 1;  //0001
        const s32 OUTCODE_RIGHT = 2;  //0010
        const s32 OUTCODE_TOP = 4;  //0100
        const s32 OUTCODE_BOTTOM = 8;  //1000

        if (x < 0)
        {
            if (y < 0)
                return OUTCODE_LEFT | OUTCODE_TOP;
            else if (y >= this->getHeight())
                return OUTCODE_LEFT | OUTCODE_BOTTOM;
            else
                return OUTCODE_LEFT;
        }
        else if (x >= this->getWidth())
        {
            if (y < 0)
                return OUTCODE_RIGHT | OUTCODE_TOP;
            else if (y >= this->getHeight())
                return OUTCODE_RIGHT | OUTCODE_BOTTOM;
            else
                return OUTCODE_RIGHT;
        }
        else
        {
            if (y < 0)
                return OUTCODE_TOP;
            else if (y >= this->getHeight())
                return OUTCODE_BOTTOM;
            else
                return OUTCODE_INSIDE;
        }
    }

    bool Bitmap::isLineTriviallyIgnorable(s32 x1, s32 y1, s32 x2, s32 y2)
    {
        return (this->computeOutcode(x1, y1) & this->computeOutcode(x2, y2)) != 0;
    }

} //graphics
