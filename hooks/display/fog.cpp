#include "fog.h"
#include "sprites.h"
#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"
#include "../../storm/storm.h"

#include <algorithm>

// Fog of war mask arrays (mask.cpp): three 1-byte/tile masks with 1-tile border
// (g_fog_shade, g_fog_drawn, g_fog_raw) and a 4x4 minitile array (g_fog_vision),
// dynamically sized via g_display. (Tile writers at 004805F0 are in refresh.cpp).
namespace {
BW_FUNC(0x00480430, void __cdecl, reset_sight_state);

u8* alloc_zeroed(u32 bytes, u32 line) {
    auto data = (u8*)SMemAlloc(bytes, g_stormsource_mask_cpp, line, 0);
    if (data) {
        std::fill_n(data, bytes, 0);
    }
    return data;
}

} // namespace

namespace hooks {
// 00480960(no arguments). Allocates the four arrays when a map is loaded.
void __cdecl alloc_fog_arrays() {
    // Map-load entry: resolve the replay/witness role, apply its zoom floor (which
    // sets the rendered world extent) and reset the zoom to the default, before
    // anything below sizes from g_display. Re-applied at the first game frame in
    // case the role resolves later than map load (witness slots).
    apply_display_role();
    patch_image_clip();

    auto active = *g_active_tile_array_int;
    for (u32 i = 0; i < 0x10000; ++i) {
        active[i] |= 0xFFFF;
    }

    reset_sight_state();
    helper::init_sight_values(0xC, (u32*)g_sight_values, g_map_tile_size->width);

    g_fog_max_shade = 0x1F;

    // Allocated at the full-FOV extent: a witness role resolved after map load
    // (see game_init_first_frame) grows the world extent into these safely.
    const u32 mask_bytes = g_display.full_mask_bytes();
    g_fog_shade = alloc_zeroed(mask_bytes, 0xFE);
    g_fog_drawn = alloc_zeroed(mask_bytes, 0x101);
    g_fog_raw = alloc_zeroed(mask_bytes, 0x104);
    g_fog_vision = alloc_zeroed(g_display.full_vision_bytes(), 0x10A);
}

// 00480350. Frees fog arrays and the 12 sight value tile tables.
void __cdecl free_fog_arrays() {
    for (u32 i = 0; i < 12; ++i) {
        if (g_sight_values[i].tile) {
            SMemFree(g_sight_values[i].tile, g_stormsource_sight_cpp, 0x14D, 0);
            g_sight_values[i].tile = nullptr;
        }
    }

    u8** const arrays[4] = { &g_fog_shade, &g_fog_drawn, &g_fog_raw, &g_fog_vision };
    const u32 lines[4] = { 0x118, 0x11D, 0x122, 0x129 };
    for (u32 i = 0; i < 4; ++i) {
        if (*arrays[i]) {
            SMemFree(*arrays[i], g_stormsource_mask_cpp, lines[i], 0);
            *arrays[i] = nullptr;
        }
    }
}

// 0047FC50. Converts player vision flags to a 1-byte/tile raw mask starting at
// (-1,-1) viewport tile offset (clamped to map edges), then blurs into g_fog_shade.
void __cdecl build_fog_mask() {
    const u32 stride = g_display.mask_stride();
    const u32 rows = g_display.mask_rows();
    const u32 tile_width = g_map_tile_size->width;
    const u32 tile_height = *g_map_tile_height;

    const s32 first_x = s32(g_move_to_tile_x) - 1 < 0 ? 0 : s32(g_move_to_tile_x) - 1;
    s32 row_y = s32(g_move_to_tile_y) - 1;

    auto vision = *g_active_tile_array_int + (tile_width * u32(row_y < 0 ? 0 : row_y) + u32(first_x));
    auto out = g_fog_raw;
    const u8 half_shade = u8(g_fog_max_shade >> 1);

    for (u32 y = 0; y < rows; ++y) {
        auto cell = vision;
        auto mask = out;
        u32 x = u32(s32(g_move_to_tile_x) - 1);

        for (u32 i = 0; i < stride; ++i) {
            const u32 flags = *cell;
            if (!g_is_in_replay) {
                if (*g_player_explored_visions & flags) {
                    *mask = 0;
                } else if (*player_visions & flags) {
                    *mask = half_shade;
                } else {
                    *mask = g_fog_max_shade;
                }
            } else if (!*g_replay_reveal_all) {
                const u8 replay = *(const u8*)g_replay_visions;
                if ((~flags & (u32(replay) << 8)) == 0) {
                    *mask = 0;
                } else if ((replay & u8(~flags)) == 0) {
                    *mask = half_shade;
                } else {
                    *mask = g_fog_max_shade;
                }
            } else {
                *mask = g_fog_max_shade;
            }

            if (x < tile_width - 1) {
                ++cell;
            }
            ++mask;
            ++x;
        }

        if (u32(row_y) < tile_height - 1) {
            vision += tile_width;
        }
        out += stride;
        ++row_y;
    }

    // 9-tap blur: weight 2 on center and edge neighbors, 1 on corners, divided by 16.
    auto src = g_fog_raw + stride + 1;
    auto dest = g_fog_shade + stride + 1;
    const s32 up = -s32(stride);
    const s32 down = s32(stride);

    for (u32 y = 0; y < rows - 2; ++y) {
        for (u32 x = 0; x < stride - 2; ++x) {
            const u32 sum = src[up - 1] + src[up + 1] + src[down - 1] + src[down + 1] + (src[up] + src[0] * 2 + src[down] + src[-1] + src[1]) * 2;
            *dest = u8(sum >> 4);
            ++src;
            ++dest;
        }
        src += 2;
        dest += 2;
    }
}

// 0047FE10. Bilinear interpolation of g_fog_shade into 4x4 minitiles per terrain tile;
// uses vertical interpolation with horizontal pairs looked up in g_fog_shade_pairs.
void __cdecl build_fog_vision() {
    const u32 mask_stride = g_display.mask_stride();
    const u32 vision_stride = g_display.vision_stride();
    const u32 cols = g_display.map_tile_cols();
    const u32 rows = g_display.map_tile_rows();

    for (u32 ty = 0; ty < rows; ++ty) {
        for (u32 tx = 0; tx < cols; ++tx) {
            auto tile = g_fog_shade + (ty + 1) * mask_stride + (tx + 1);

            s32 left = s32(tile[0]) << 8;
            s32 right = s32(tile[1]) << 8;
            const s32 left_step = (s32(tile[mask_stride]) - s32(tile[0])) << 8;
            const s32 right_step = (s32(tile[mask_stride + 1]) - s32(tile[1])) << 8;

            for (u32 sub = 0; sub < 4; ++sub) {
                auto out = (u32*)(g_fog_vision + (ty * 4 + sub) * vision_stride + tx * 4);
                *out = g_fog_shade_pairs[(left >> 8) * 0x20 + (right >> 8)];
                left += left_step / 4;
                right += right_step / 4;
            }
        }
    }
}

// 004804D0. Rebuilds fog mask and marks dirty rects for tiles changed since last
// frame. Walks viewport + 1 border tile in pixels offset by sub-tile scroll.
void __cdecl mark_fog_changes() {
    build_fog_mask();

    const u32 stride = g_display.mask_stride();
    const s32 tile = s32(display::TileSize);
    const s32 sub_x = g_move_to_x & (tile - 1);
    const s32 right = s32((g_display.map_tile_cols() + 1) * display::TileSize) - sub_x;
    const s32 top = -(s32(g_move_to_y) & (tile - 1));
    const s32 bottom = top + s32((g_display.map_tile_rows() + 1) * display::TileSize);

    auto shade_row = g_fog_shade + stride + 1;
    auto drawn_row = g_fog_drawn + stride + 1;

    for (s32 y = top; y < bottom; y += tile) {
        auto shade = shade_row;
        auto drawn = drawn_row;

        for (s32 x = -sub_x; x < right; x += tile) {
            auto next_shade = shade + 1;
            auto next_drawn = drawn + 1;

            if (*drawn != *shade) {
                *drawn = *shade;

                // Coalesce horizontal run of changed tiles into a single rect.
                s32 run = tile;
                for (s32 scan = x + tile; scan < right && *next_drawn != *next_shade; scan += tile) {
                    *next_drawn = *next_shade;
                    run += tile;
                    ++next_shade;
                    ++next_drawn;
                }

                // 1-tile margin to cover adjacent blurred neighbors.
                helper::bw_refresh(x - tile, y + 2 * tile - 1, y - tile, x + run + tile - 1);
                x += run - tile;
            }

            shade = next_shade;
            drawn = next_drawn;
        }

        shade_row += stride;
        drawn_row += stride;
    }
}

} // hooks
