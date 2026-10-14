#include "plot.h"

#include "graphics/Bitmap.h"
#include "hooks/interface/mainmenu/glucustom.h"
#include "text.h"

namespace text {

bool is_color_code(u8 code) {
    return (code >= 0x02 && code <= 0x08)
        || (code >= 0x0E && code <= 0x11)
        || (code >= 0x15 && code <= 0x1F)
        || (code >= 0x8F && code <= 0x9F && code != 0x99);
}

u8 color_slot(u8 code) {
    if (code >= 0x9A) {
        return code - 0x7A; // 0x99 is the trademark glyph, so codes above it shift down one, like get_color_handle
    }
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

const u8* ramp(u8 slot) {
    return g_tfontgame_image_data + slot * 8;
}

void plot_pixel(graphics::Bitmap& bitmap, const Box32& clip, s32 x, s32 y, u8 value) {
    if (x < clip.left || x > clip.right || y < clip.top || y > clip.bottom) {
        return;
    }
    bitmap.draw_dot(x, y, value);
}

void plot_glyph(graphics::Bitmap& bitmap, const Box32& clip, const GlyphBitmap& glyph, s32 x, s32 y, const u8* ramp) {
    const u8 shadow = ramp[ShadowRampByte];
    for (u32 row = 0; row < glyph.height; ++row) {
        for (u32 column = 0; column < glyph.width; ++column) {
            if (glyph.pixels[row * glyph.width + column]) {
                plot_pixel(bitmap, clip, x + column + 1, y + row + 1, shadow);
            }
        }
    }
    for (u32 row = 0; row < glyph.height; ++row) {
        for (u32 column = 0; column < glyph.width; ++column) {
            const u8 mask = glyph.pixels[row * glyph.width + column];
            if (mask) {
                plot_pixel(bitmap, clip, x + column, y + row, ramp[mask]);
            }
        }
    }
}

void plot_cursor(graphics::Bitmap& bitmap, const Box32& clip, const PrintFont& font, s32 x, s32 line_y, const u8* ramp) {
    TextSystem& text_system = TextSystem::instance();
    FontFace* face = text_system.stack(font.stack).resolve(0xA6);
    if (!face) {
        return;
    }
    const char32_t codepoint = 0xA6;
    std::vector<ShapedGlyph>& shaped = text_system.walk_scratch().shaped;
    text_system.shaper().shape_into(shaped, *face, font.pixel_height, {&codepoint, 1});
    if (shaped.empty()) {
        return;
    }
    const GlyphBitmap& glyph = text_system.glyph_cache().lookup(*face, font.pixel_height, shaped.front().glyph_index);
    plot_glyph(bitmap, clip, glyph,
        x + shaped.front().offset_x,
        line_y + shaped.front().offset_y + font.ascent - glyph.bitmap_top,
        ramp);
}

graphics::FrameSize plot_icon(graphics::Bitmap& bitmap, const Box32& clip, u16 unit_id, s32 x, s32 line_y, s32 line_advance, u32 pixel_height, const u8* ramp) {
    const auto unit_icon = graphics::unit_icon(unit_id);
    if (!unit_icon) {
        return {};
    }
    const graphics::FrameSize size = graphics::fit_height(unit_icon->grp->frame_size(unit_icon->frame), pixel_height);
    if (!size.height) {
        return size;
    }
    // Icon frames carry their own 15-step ramp (1 = brightest)
    // map it onto the current text color's font-ramp shades
    u8 remap[16] = {};
    for (u8 level = 1; level < 16; ++level) {
        remap[level] = ramp[1 + ((level - 1) >> 2)];
    }
    unit_icon->grp->blit_frame(unit_icon->frame, bitmap, {
        .x = x,
        .y = line_y + (line_advance - s32(size.height)) / 2,
        .format = graphics::FrameFormat::Rle,
        .width = size.width,
        .height = size.height,
        .clip = clip,
        .remap = remap,
        .remap_length = 16,
    });
    return size;
}

} // namespace text