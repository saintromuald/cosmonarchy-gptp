#include "refresh.h"
#include "grid.h"
#include "../../BW/bwdata.h"
#include "../../graphics/Bitmap.h"

#include <algorithm>

// Fog-of-war pass and dirty-cell walker scaled to surface pitch.
namespace {

s32 screen_pitch() {
    return s32(g_display.surface_width());
}

// 004800A0: Unexplored tile (solid black).
void tile_black(u8* dest) {
    const s32 pitch = screen_pitch();
    for (s32 y = 0; y < 8; ++y) {
        *(u32*)dest = 0;
        *(u32*)(dest + 4) = 0;
        dest += pitch;
    }
}

// 00480000: Flat fog shade across 8x8 tile via dark_pcx remap table.
void tile_flat(u8* dest, u32 shade) {
    const u8* const remap = dark_pcx_image_map->index[shade];
    const s32 pitch = screen_pitch();
    for (s32 y = 0; y < 8; ++y) {
        for (s32 x = 0; x < 8; ++x) {
            dest[x] = remap[dest[x]];
        }
        dest += pitch;
    }
}

// 0047FF10: Interpolates shade across 8x8 tile corners using 8.8 fixed point.
void tile_blend(u8* dest, u32 top_left, u32 top_right, u32 bottom_left, u32 bottom_right) {
    const s32 left_step = (s32(bottom_left) - s32(top_left)) * 32;
    const s32 right_step = (s32(bottom_right) - s32(top_right)) * 32;
    const s32 pitch = screen_pitch();

    s32 left = s32(top_left) << 8;
    s32 right = s32(top_right) << 8;

    for (s32 y = 0; y < 8; ++y) {
        const s32 step = (left - right) >> 3;
        s32 shade = right;
        for (s32 x = 7; x >= 0; --x) {
            dest[x] = dark_pcx_image_map->index[shade >> 8][dest[x]];
            shade += step;
        }
        left += left_step;
        right += right_step;
        dest += pitch;
    }
}

// Bottom map edge fade
// Dark table's usable ramp is rows 1-30 (high = light).
constexpr s32 EdgeFadeRows = 16;
constexpr s32 EdgeFadeDarkest = 2;
constexpr s32 EdgeFadeLightest = 30;
static_assert(EdgeFadeRows >= 2 && EdgeFadeDarkest >= 1 && EdgeFadeLightest <= 0x1E);

void apply_edge_fade(s32 top, s32 bottom, s32 left, s32 right, s32 seam) {
    const s32 band_top = seam - EdgeFadeRows;
    top = std::max(top, band_top);
    bottom = std::min(bottom, seam);
    if (top >= bottom || left >= right) {
        return;
    }

    const s32 pitch = screen_pitch();
    const s32 fade_width = right - left;
    auto* const target = (graphics::Bitmap*)*g_current_surface;
    u8* dest_row = target->data + top * pitch + left;

    for (s32 y = top; y < bottom; ++y) {
        const s32 depth = y - band_top;
        const s32 level = std::max(EdgeFadeLightest - (EdgeFadeLightest - EdgeFadeDarkest) * depth / (EdgeFadeRows - 1), 1);
        const u8* remap = dark_pcx_image_map->index[level];
        for (s32 x = 0; x < fade_width; ++x) {
            dest_row[x] = remap[dest_row[x]];
        }
        dest_row += pitch;
    }
}

} // namespace

namespace hooks {

void __cdecl apply_fog(s32 top, s32 bottom, s32 left, s32 right) {
    if (top >= bottom) {
        return;
    }

    // The overscroll strip below the map bottom is not gameplay terrain; keep
    // fog off it. The seam (map bottom minus camera) is 8-aligned, preserving
    // the 8px tile walk.
    const s32 map_bottom = s32(g_map_height_pixels) - g_move_to_y;
    if (bottom > map_bottom) {
        bottom = map_bottom;
    }
    if (top >= bottom) {
        return;
    }

    const s32 pitch = screen_pitch();
    const u32 vis_stride = s32(g_display.vision_stride());

    // Sub-tile scroll offsets select starting visibility row and column within fog vision buffer.
    const s32 sub_x = (g_move_to_x >> 3) & 3;
    const s32 sub_y = (g_move_to_y >> 3) & 3;

    const u8* vis_row = g_fog_vision + sub_x + (sub_y + (top >> 3)) * vis_stride
                        + (left >> 3) + vis_stride;
    auto* const target = (graphics::Bitmap*)*g_current_surface;
    u8* dest_row = target->data + top * pitch + left;

    u32 tile_rows = (u32(bottom - top - 1) >> 3) + 1;
    while (tile_rows--) {
        if (left < right) {
            const u8* vis = vis_row;
            u8* dest = dest_row;
            u32 tiles = (u32(right - left - 1) >> 3) + 1;
            while (tiles--) {
                const u32 top_left = vis[-vis_stride];
                const u32 top_right = vis[1 - vis_stride];
                const u32 bottom_left = vis[0];
                const u32 bottom_right = vis[1];

                if (top_left != top_right || top_left != bottom_left || top_left != bottom_right) {
                    tile_blend(dest, top_left, top_right, bottom_left, bottom_right);
                } else if (top_left == 0) {
                    tile_black(dest);
                } else if (top_left != 0x1f) {
                    tile_flat(dest, top_left);
                }

                dest += 8;
                ++vis;
            }
        }
        vis_row += vis_stride;
        dest_row += 8 * pitch;
    }

    apply_edge_fade(top, bottom, left, right, map_bottom);
}

// 004808E0: Walks dirty grid across world viewport, coalescing adjacent dirty cells into spans.
void __cdecl refresh_dirty_cells(u32 refresh_all) {
    const s32 cell = s32(g_display.cell_size());
    const s32 width = s32(g_display.world_width());
    const s32 height = s32(g_display.world_height());

    if (refresh_all) {
        apply_fog(0, height, 0, width);
        return;
    }

    // Fog remaps in place, so only marked dirty cells are re-fogged.
    const s32 stride = display::world_phase_stride();
    for (s32 y = 0; y < height; y += cell) {
        const u8* row = display::world_phase_cells() + (y / cell) * stride;
        s32 x = 0;
        while (x < width) {
            if (!row[x / cell]) {
                x += cell;
                continue;
            }
            s32 right = x + cell;
            while (right < width && row[right / cell]) {
                right += cell;
            }

            const s32 bottom = (y + cell > height) ? height : y + cell;
            if (right > width) {
                right = width;
            }
            apply_fog(y, bottom, x, right);
            x = right;
        }
    }
}

} // hooks
