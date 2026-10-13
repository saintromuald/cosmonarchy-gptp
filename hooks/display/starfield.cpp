#include "starfield.h"

#include "../../BW/bwdata.h"
#include "../../graphics/Bitmap.h"
#include "../../systems/display/display.h"
#include "grid.h"

#include <algorithm>

// Space-map parallax star field, loaded from parallax\star.spk by 0047F2C0: five layers of
// 8-byte entries {x, y, sprite*} whose sprites are {width, height, packed pixels}. Entries and
// parallax offsets live in a periodic 648x488 space (legacy world + 8px lead-in; the periods
// are g_scratch_wrap_x/y >> 8). The vanilla walkers wrapped every star into that single period
// and clipped to the legacy 640x400 window, which confined stars to the top-left corner of the
// widescreen surface. Drawing the period tiled across the current world window instead keeps
// the vanilla star density everywhere, and since the tiled field is invariant under period
// shifts the parallax wrap snap stays invisible.
//
// draw_frame_composite (004BD580) branches between the two vanilla walkers on its full-redraw
// flag: full redraws call 0047EE20, which redraws every star because the whole world was just
// re-blitted; partial frames call 0047EBF0, which skips star copies over clean cells since only
// dirty cells were re-blitted and everything else keeps its pixels. Both variants are
// reimplemented here over one tiled core.
namespace {

constexpr s32 StarMargin = 8;

struct StarSprite {
    u16 width;
    u16 height;
    // Packed pixel rows (width bytes each) follow the header.
};

struct StarEntry {
    u16 x;
    u16 y;
    const StarSprite* sprite;
};

// On partial frames only dirty cells were re-blitted, so only star copies over dirty cells
// need redrawing.
bool star_cells_dirty(s32 x, s32 y, s32 width, s32 height) {
    const s32 cell = s32(g_display.cell_size());
    const s32 stride = display::world_phase_stride();
    const u8* grid = display::world_phase_cells();
    const s32 left = x / cell;
    const s32 right = (x + width - 1) / cell;
    const s32 top = y / cell * stride;
    const s32 bottom = (y + height - 1) / cell * stride;
    return grid[top + left] || grid[top + right] || grid[bottom + left] || grid[bottom + right];
}

// Blits a star over black (palette index 0) pixels only - the space-map void - clipped to the
// world window, mirroring vanilla 0047EA60's masked copy with the surface pitch.
void draw_star(const StarSprite& sprite, s32 x, s32 y, bool skip_clean) {
    const s32 world_width = s32(g_display.world_width());
    const s32 world_height = s32(g_display.world_height());

    s32 width = sprite.width;
    s32 height = sprite.height;
    const u8* src = (const u8*)(&sprite + 1);
    if (x < 0) {
        src -= x;
        width += x;
        x = 0;
    }
    if (y < 0) {
        src += -y * s32(sprite.width);
        height += y;
        y = 0;
    }
    width = std::min(width, world_width - x);
    height = std::min(height, world_height - y);
    if (width <= 0 || height <= 0) {
        return;
    }
    if (skip_clean && !star_cells_dirty(x, y, width, height)) {
        return;
    }

    const s32 pitch = s32(g_display.surface_width());
    auto* const target = (graphics::Bitmap*)*g_current_surface;
    u8* dest = target->data + y * pitch + x;
    for (s32 row = 0; row < height; ++row) {
        const u8* src_row = src + row * s32(sprite.width);
        u8* dest_row = dest + row * pitch;
        for (s32 col = 0; col < width; ++col) {
            if (dest_row[col] == 0) {
                dest_row[col] = src_row[col];
            }
        }
    }
}

void draw_starfield_tiled(bool skip_clean) {
    auto entries = (const StarEntry*)g_starfield_entries;
    if (!entries) {
        return;
    }

    const s32 period_x = s32(g_scratch_wrap_x >> 8);
    const s32 period_y = s32(g_scratch_wrap_y >> 8);
    if (period_x <= 0 || period_y <= 0) {
        return;
    }
    const s32 world_width = s32(g_display.world_width());
    const s32 world_height = s32(g_display.world_height());

    // File entry blocks pair with parallax layers back to front, as the vanilla engine walks them
    for (s32 layer = 4; layer >= 0; --layer) {
        const s32 offset_x = s32(g_parallax_x[layer] >> 8) - StarMargin;
        const s32 offset_y = s32(g_parallax_y[layer] >> 8) - StarMargin;
        for (u32 i = g_starfield_layer_counts[layer]; i; --i, ++entries) {
            s32 x = s32(entries->x) + offset_x;
            s32 y = s32(entries->y) + offset_y;
            x %= period_x;
            y %= period_y;
            if (x < 0) x += period_x;
            if (y < 0) y += period_y;

            for (; x < world_width; x += period_x) {
                for (s32 star_y = y; star_y < world_height; star_y += period_y) {
                    draw_star(*entries->sprite, x, star_y, skip_clean);
                }
            }
        }
    }
}

} // namespace

namespace hooks {

// 0047EE20: draw_frame_composite's full-redraw branch - the whole world was re-blitted, so
// every star copy redraws.
void __cdecl draw_starfield() {
    draw_starfield_tiled(false);
}

// 0047EBF0: draw_frame_composite's partial-frame branch - only dirty cells were re-blitted, so
// star copies over clean cells are skipped.
void __cdecl draw_starfield_wrapped() {
    draw_starfield_tiled(true);
}

} // hooks