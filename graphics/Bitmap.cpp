//Based on BWAPI's BW/Bitmap.cpp
#include "Bitmap.h"
#include "Font.h"
#include "../BW/bwdata.h"
#include "BW/enumerations.h"
#include "systems/graphics/grp.h"
#include "systems/text/decoder.h"
#include "systems/text/measure.h"
#include "systems/text/text.h"
#include <cassert>
#include <utility>
#include <string_view>
#include <vector>
#include <algorithm>
#include <player_color.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

// Defined in glucustom.h. Using extern because this is a low-level graphics file that shouldn't pull in random headers
// TODO: Move this to a more appropriate location -Jesse
extern u8 g_tfontgame_image_data[]; 

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

// Control byte -> font ramp slot, matching the mod's get_color_handle remap
// (glucustom.cpp + the 0x004200AC offset-table patches in initialize.cpp):
// 0x02-0x08 -> 0-6, 0x0E-0x11 -> 7-10, 0x15-0x1F -> 11-21, and the extended
// codes 0x8F-0x98 / 0x9A-0x9E -> 22-37 (0x99 is the trademark glyph and never
// arrives as a code). Player colors use PlayerFontSlot.
u8 color_slot(u8 code) {
    if (code >= 0x8F) {
        return code - 0x79;
    }
    if (code <= 0x08) {
        return code - 0x02;
    }
    if (code <= 0x11) {
        return code - 0x07;
    }
    return code - 0x0A; // 0x15-0x1F, including 0x1A (slot 16)
}

// Ramp layout: bytes 1-4 are the coverage shades (byte 1 = bright core), byte 7
// is the shadow color baked into .fnt glyphs as mask-7 pixels.
constexpr u32 ShadowRampByte = 7;

u8 ramp_byte(u8 color, u32 mask) {
    return g_tfontgame_image_data[color * 8 + mask];
}

// The .fnt path, kept for when TextSystem can't come up.
bool draw_text_legacy(Bitmap& bitmap, std::string_view text, s32 x, s32 y, u8 size) {
    Font* fnt = font_base[size];
    if (!fnt) {
        return false;
    }

    // verify if drawing should be done
    if (x + fnt->getTextWidth(text) < 0 ||
        y + fnt->getTextHeight(text) < 0 ||
        x >= bitmap.getWidth() || y >= bitmap.getHeight()) {
        return false;
    }

    // Reference an unsigned character array
    const u8* pbChars = (const u8*)text.data();
    const s32 length = (s32)text.size();

    u8 last_color = 0;
    u8 color = 0;
    s32 Xoffset = x, Yoffset = y;

    // Iterate all characters in the message
    for (s32 c = 0; c < length; ++c) {
        char prev_c = c > 0 ? pbChars[c - 1] : 0;
        // Perform control character and whitespace functions
        if (pbChars[c] <= ' ') {
            switch (pbChars[c]) {
                case 0:         // vanilla ends the print at 0x00 like 0x0C
                case '\f':      // 12   formfeed
                    return true;
                case 1: {       // restore last colour
                    color = last_color;
                } continue;
                case '\t': {    // 9    tab
                    Xoffset += fnt->getCharWidth(pbChars[c], prev_c);
                } continue;
                case '\n': {    // 10   newline
                    Xoffset = x;
                    Yoffset += fnt->maxHeight();
                } continue;
                case '\r':     // 13   carriage return
                    continue;
                case 18: {      // right align
                    Xoffset += bitmap.getWidth() - fnt->getTextWidth(text) - x;
                } continue;
                case 19: {      // center align
                    Xoffset += (bitmap.getWidth() - fnt->getTextWidth(text)) / 2 - x;
                } continue;
                case ' ': {     // space
                    Xoffset += fnt->maxWidth() / 2;
                } continue;
                case std::to_underlying(TextColor::PlayerColor):
                case std::to_underlying(TextColor::PlayerColor2): { // player color
                    if (c + 1 < length) {
                        const u8 code = pbChars[c + 1];
                        if ((code >= 1 && code <= 8) || (code >= 0x0E && code <= 0x11)) {
                            ++c;
                            last_color = color;
                            set_player_font_ramp(code <= 8 ? u8(code - 1) : u8(code + 8));
                            color = PlayerFontSlot;
                        }
                    }
                } continue;
                default: {       // colour code
                    last_color = color;
                    color = color_slot(pbChars[c]);
                } continue;
            }
        } else if (pbChars[c] >= text::ExtendedColorFirst
            && pbChars[c] <= text::ExtendedColorLast
            && pbChars[c] != text::TrademarkByte) {
            last_color = color;
            color = color_slot(pbChars[c]);
            continue;
        }

        // Skip if the character is not supported by the font
        FontChar* fntChr = fnt->getChar(pbChars[c]);
        if (!fntChr) {
            continue;
        }

        // begin drawing character process
        for (s32 i = 0, pos = 0; pos < fntChr->getHeight() * fntChr->getWidth(); ++i, ++pos) {
            // font position
            pos += fntChr->pixelOffset(i);

            // x offset
            s32 newX = Xoffset + (fntChr->get_x() + pos % fntChr->getWidth());
            if (newX >= bitmap.getWidth()) break;
            if (newX < 0) continue;

            // y offset
            s32 newY = Yoffset + (fntChr->get_y() + pos / fntChr->getWidth());
            if (newY >= bitmap.getHeight()) break;
            if (newY < 0) continue;

            // blit offset
            s32 offset = newY * bitmap.getWidth() + newX;
            if (offset >= bitmap.getWidth() * bitmap.getHeight()) break;
            if (offset < 0) continue;

            // Plot pixel
            u32 mask_index = fntChr->colorMask(i);
            if (mask_index) {
                bitmap.data[offset] = ramp_byte(color, mask_index);
            }
        }

        // Increment the X offset for the width of the character
        Xoffset += fntChr->getWidth();
    }
    return true;
}

bool draw_text_ttf(Bitmap& bitmap, std::string_view text, s32 x, s32 y, u8 slot) {
    text::TextSystem& text_system = text::TextSystem::instance();
    const text::SlotConfig& config = text_system.slot(slot);
    text::FontStack& stack = text_system.stack(config.stack);
    text::Shaper& shaper = text_system.shaper();
    text::GlyphCache& glyph_cache = text_system.glyph_cache();
    const u32 pixel_height = config.pixel_height;
    const s32 spacing = text_system.char_spacing();

    if (x + text::text_width(slot, text) < 0
        || y + text::text_height(slot, text) < 0
        || x >= bitmap.getWidth() || y >= bitmap.getHeight()) {
        return false;
    }

    text::FontFace* primary_face = stack.primary_face();
    if (!primary_face) {
        return false;
    }
    const s32 ascent = primary_face->ascender(pixel_height);

    u8 last_color = 0;
    u8 color = 0;
    s32 x_offset = x;

    std::vector<char32_t> run;
    text::FontFace* run_face = nullptr;

    const auto flush_run = [&]() {
        if (run.empty() || !run_face) {
            run.clear();
            return;
        }
        for (const text::ShapedGlyph& glyph : shaper.shape(*run_face, pixel_height, run)) {
            const text::GlyphBitmap& glyph_bitmap = glyph_cache.lookup(*run_face, pixel_height, glyph.glyph_index);
            const s32 glyph_x = x_offset + glyph.offset_x + glyph_bitmap.bitmap_left;
            const s32 glyph_y = y + glyph.offset_y + ascent - glyph_bitmap.bitmap_top;
            const u8 shadow = ramp_byte(color, ShadowRampByte);
            for (u32 v = 0; v < glyph_bitmap.height; ++v) {
                for (u32 u = 0; u < glyph_bitmap.width; ++u) {
                    if (glyph_bitmap.pixels[v * glyph_bitmap.width + u]) {
                        bitmap.draw_dot(glyph_x + u + 1, glyph_y + v + 1, shadow);
                    }
                }
            }
            for (u32 v = 0; v < glyph_bitmap.height; ++v) {
                for (u32 u = 0; u < glyph_bitmap.width; ++u) {
                    const u8 mask = glyph_bitmap.pixels[v * glyph_bitmap.width + u];
                    if (mask) {
                        bitmap.draw_dot(glyph_x + u, glyph_y + v, ramp_byte(color, mask));
                    }
                }
            }
            x_offset += glyph.advance + spacing;
        }
        run.clear();
    };

    const u8* cursor = (const u8*)text.data();
    const u8* end = cursor + text.size();
    bool print_ended = false;
    while (!print_ended) {
        const auto token = text::decode_next(cursor, end);
        if (!token) {
            break;
        }
        std::visit(Overloaded{
            [&](const text::Codepoint& codepoint) {
                text::FontFace* face = stack.resolve(codepoint.value);
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
                        color = last_color;
                    } break;
                    case '\t': {
                        x_offset += text_system.tab_advance(slot);
                    } break;
                    case '\n': {
                        x_offset = x;
                        y += text_system.line_advance(slot);
                    } break;
                    case '\f': {    // vanilla ends the whole print at 0x0C
                        print_ended = true;
                    } break;
                    case '\r':
                        break;
                    case std::to_underlying(TextColor::AlignRight): {
                        const std::string_view rest{(const char*)cursor, size_t(end - cursor)};
                        x_offset += bitmap.getWidth() - text::line_width(slot, rest) - x;
                    } break;
                    case std::to_underlying(TextColor::AlignCenter): {
                        const std::string_view rest{(const char*)cursor, size_t(end - cursor)};
                        x_offset += (bitmap.getWidth() - text::line_width(slot, rest)) / 2 - x;
                    } break;
                    case ' ': {
                        x_offset += text_system.space_advance(slot);
                    } break;
                    case std::to_underlying(TextColor::PlayerColor):
                    case std::to_underlying(TextColor::PlayerColor2): {
                        if (cursor != end) {
                            const u8 code = *cursor;
                            if ((code >= 1 && code <= 8) || (code >= 0x0E && code <= 0x11)) {
                                ++cursor;
                                last_color = color;
                                set_player_font_ramp(code <= 8 ? u8(code - 1) : u8(code + 8));
                                color = PlayerFontSlot;
                            }
                        }
                    } break;
                    default: {      // colour code, including the extended 0x8F-0x9E range
                        last_color = color;
                        color = color_slot(control.value);
                    } break;
                }
            },
            [&](const text::Icon& icon) {
                flush_run();
                const auto unit_icon = graphics::unit_icon(icon.unit_id);
                if (!unit_icon) {
                    return; // no icon source yet - nothing to draw or advance
                }
                const FrameSize size = fit_height(unit_icon->grp->frame_size(unit_icon->frame), pixel_height);
                if (!size.height) {
                    return; // degenerate frame - nothing to draw or advance
                }
                const s32 icon_y = y + (text_system.line_advance(slot) - s32(size.height)) / 2;
                // Icon frames carry their own 15-step ramp (1 = brightest)
                // map it onto the current text color's font-ramp shades
                u8 icon_remap[16] = {};
                for (u8 level = 1; level < 16; ++level) {
                    icon_remap[level] = ramp_byte(color, 1 + ((level - 1) >> 2));
                }
                unit_icon->grp->blit_frame(unit_icon->frame, bitmap, {
                    .x = x_offset,
                    .y = icon_y,
                    .format = FrameFormat::Rle,
                    .width = size.width,
                    .height = size.height,
                    .remap = icon_remap,
                    .remap_length = 16,
                });
                x_offset += unit_icon_advance(icon.unit_id, pixel_height, spacing);
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
    if (text::TextSystem::instance().available()) {
        return draw_text_ttf(*this, text, x, y, size);
    }
    return draw_text_legacy(*this, text, x, y, size);
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
