#include "terrain.h"
#include "tiles.h"
#include "../../BW/bwdata.h"

#include <algorithm>

// Terrain renderers for the world scratch ring buffer.
// Tile offset is (ty * pitch + tx) * 32 % map_size, stepping 32 bytes per tile column and 32 * pitch per row.
namespace {
// Computes tile byte offset within the world scratch buffer modulo map_size.
s32 tile_offset(u32 tile_x, u32 tile_y) {
    const u32 raw = (tile_y * g_display.map_pitch() + tile_x) * display::TileSize;
    return s32(raw % g_display.map_size());
}

// Advances one tile column (32 bytes), wrapping at map_size.
s32 next_tile_col(s32 offset) {
    offset += display::TileSize;
    if (offset >= s32(g_display.map_size())) {
        offset -= s32(g_display.map_size());
    }
    return offset;
}

s32 next_tile_row(s32 offset) {
    offset += s32(display::TileSize * g_display.map_pitch());
    if (offset >= s32(g_display.map_size())) {
        offset -= s32(g_display.map_size());
    }
    return offset;
}

// Fills one 32x32 tile cell of the scratch ring with a flat color (palette
// index 0 = black).
void fill_tile(s32 offset, u8 color) {
    const s32 map_pitch = s32(g_display.map_pitch());
    const s32 map_size = s32(g_display.map_size());
    u8* map_data = g_game_map_data_for_blit;

    for (u32 row = 0; row < display::TileSize; ++row) {
        if (offset >= map_size) {
            offset -= map_size;
        }
        std::fill_n(map_data + offset, display::TileSize, color);
        offset += map_pitch;
    }
}

// Matches g_parallax_steps / g_parallax_x / g_parallax_y in bwdata.h.
constexpr s32 ParallaxLayers = 5;
static_assert(std::size(g_parallax_steps) == ParallaxLayers);
static_assert(std::size(g_parallax_x) == ParallaxLayers);
static_assert(std::size(g_parallax_y) == ParallaxLayers);

// Advances parallax layer positions by step * amount with wrap limit.
void advance_parallax(std::array<u32, ParallaxLayers>& layers, s32 amount, s32 wrap, bool subtract) {
    for (s32 i = 0; i < ParallaxLayers; ++i) {
        const s32 step = s32(g_parallax_steps[i]) * amount;
        s32 value = s32(layers[i]);
        if (subtract) {
            value -= step;
            if (value < 0) {
                value += wrap;
            }
        } else {
            value += step;
            if (value > wrap) {
                value -= wrap;
            }
        }
        layers[i] = u32(value);
    }
}

} // namespace

namespace hooks {
// 0049B8D0(eax = &tile_x, ecx = &tile_y, [stack] &width, &height), callee-clean.
// Clips tile rectangle to map and viewport bounds; returns 0 when not visible.
u32 __cdecl clip_tile_rect(s32* tile_x, s32* tile_y, s32* out_width, s32* out_height) {
    const s32 cols = s32(g_display.map_tile_cols());
    const s32 rows = s32(g_display.map_tile_rows());
    const s32 view_x = g_move_to_x >> 5;
    const s32 view_y = g_move_to_y >> 5;

    if (*tile_x < 0) {
        *tile_x = 0;
    } else if (s32(g_map_tile_size->width) <= *out_width + *tile_x) {
        *out_width = s32(g_map_tile_size->width) - *tile_x;
    }

    if (*tile_y < 0) {
        *tile_y = 0;
    } else if (s32(*g_map_tile_height) <= *out_height + *tile_y) {
        *out_height = s32(*g_map_tile_height) - *tile_y;
    }

    *tile_x -= view_x;
    *tile_y -= view_y;

    if (*tile_x < 0) {
        *out_width += *tile_x;
        *tile_x = 0;
    }
    if (*out_width + *tile_x > cols - 1) {
        *out_width = cols - *tile_x;
    }
    if (*tile_y < 0) {
        *out_height += *tile_y;
        *tile_y = 0;
    }
    if (*out_height + *tile_y > rows - 1) {
        *out_height = rows - *tile_y;
    }

    if (*out_width > 0 && *out_height > 0 && *tile_x < cols && *tile_y < rows) {
        *tile_x += view_x;
        *tile_y += view_y;
        if (*tile_x < s32(g_map_tile_size->width) && *tile_y < s32(*g_map_tile_height)) {
            return 1;
        }
    }
    return 0;
}

// 0040AAE0(ecx = record, edx = offset): blits RLE overlay (creep/doodad) into scratch buffer.
// Header is [x, y, w, h] followed by row offsets table. 0x80 skips, 0x40 repeats, else copies.
void __cdecl blit_overlay(const u8* record, s32 offset) {
    const s32 map_size = s32(g_display.map_size());
    const s32 map_pitch = s32(g_display.map_pitch());
    auto map_data = g_game_map_data_for_blit;

    g_overlay_end = map_data + map_size;

    auto dest = map_data + offset + record[1] * map_pitch + record[0];
    if (dest >= g_overlay_end) {
        dest -= map_size;
    }

    g_overlay_width = record[2];
    g_overlay_height = record[3];
    g_overlay_row_step = u32(map_pitch) - g_overlay_width;

    auto table = *(const u16* const*)(record + 4);
    auto row_offset = table;

    for (;;) {
        auto src = (const u8*)table + *row_offset;
        ++row_offset;

        s32 remaining = s32(g_overlay_width);
        while (remaining > 0) {
            const u8 op = *src++;
            if (op & 0x80) {
                const s32 run = op & 0x7F;
                remaining -= run;
                dest += run;
            } else if (op & 0x40) {
                const s32 run = op & 0x3F;
                const u8 value = *src++;
                for (s32 i = 0; i < run; ++i) {
                    *dest++ = value;
                }
                remaining -= run;
            } else {
                for (s32 i = 0; i < op; ++i) {
                    *dest++ = *src++;
                }
                remaining -= op;
            }
        }

        dest += g_overlay_row_step;
        if (dest >= g_overlay_end) {
            dest -= map_size;
        }
        if (--g_overlay_height == 0) {
            return;
        }
    }
}

// 0049BE20(eax = tile_x, [ebp+8] = megatiles, [ebp+0xC] = tile_y), callee-clean.
// Draws one viewport-width horizontal strip of megatiles. Beyond-map regions fill
// black: below-map rows (console overscroll) and columns past the map's right
// edge (the camera pins at x=0 when the map is narrower than the window).
void __cdecl draw_tile_row(const u16* megatiles, u32 tile_y, u32 tile_x) {
    s32 offset = tile_offset(tile_x, tile_y);
    const u32 cols = g_display.map_tile_cols();

    if (tile_y >= *g_map_tile_height) {
        for (u32 i = 0; i < cols; ++i) {
            fill_tile(offset, 0);
            offset = next_tile_col(offset);
        }
        return;
    }

    // Clamps tile strip count to remaining map width or viewport columns.
    u32 count = u32(g_map_tile_size->width) - tile_x;
    if (count > cols) {
        count = cols;
    }

    for (u32 i = 0; i < count; ++i) {
        draw_megatile(s32(tile_x), s32(tile_y), megatiles[i], offset);
        offset = next_tile_col(offset);
        ++tile_x;
    }
    for (u32 i = count; i < cols; ++i) {
        fill_tile(offset, 0);
        offset = next_tile_col(offset);
    }
}

// 0049BD40(eax = tile_y, [ebp+8] = megatiles, [ebp+0xC] = tile_x), callee-clean.
// Draws one viewport-height vertical strip of megatiles across the full scratch
// window; below-map rows and columns past the map's right edge fill black.
// Horizontal scrolling while overscrolled past the map bottom must repaint
// every below-map row the window covers, not just vanilla's single sub-tile
// margin row. Returns the final ring offset, or the map width when the column
// starts beyond the map (vanilla leaves that in EAX).
u32 __cdecl draw_tile_column(const u16* megatiles, u32 tile_x, u32 tile_y) {
    const u32 count = g_display.map_tile_rows();
    s32 offset = tile_offset(tile_x, tile_y);

    if (tile_x >= g_map_tile_size->width) {
        for (u32 i = 0; i < count; ++i) {
            fill_tile(offset, 0);
            offset = next_tile_row(offset);
        }
        return u32(g_map_tile_size->width);
    }

    const u32 stride = g_map_tile_size->width;
    for (u32 i = 0; i < count; ++i) {
        if (tile_y < *g_map_tile_height) {
            draw_megatile(s32(tile_x), s32(tile_y), *megatiles, offset);
        } else {
            fill_tile(offset, 0);
        }
        megatiles += stride;
        ++tile_y;
        offset = next_tile_row(offset);
    }
    return u32(offset);
}

// 0049BC20([ebp+8] = tile_x, +0xC = tile_y, +0x10 = width, +0x14 = height, in tiles), callee-clean.
// Draws an arbitrary rectangle of terrain, clipped to the viewport first.
void __cdecl draw_tile_rect(s32 tile_x, s32 tile_y, s32 width, s32 height) {
    if (!clip_tile_rect(&tile_x, &tile_y, &width, &height)) {
        return;
    }

    s32 offset = tile_offset(u32(tile_x), u32(tile_y));
    const u32 stride = g_map_tile_size->width;
    auto row = *megatileArray + u32(tile_y) * stride + u32(tile_x);

    for (s32 y = 0; y < height; ++y) {
        s32 cell = offset;
        auto megatile = row;
        for (s32 x = 0; x < width; ++x) {
            draw_megatile(tile_x + x, tile_y + y, *megatile, cell);
            cell = next_tile_col(cell);
            ++megatile;
        }
        offset = next_tile_row(offset);
        row += stride;
    }
}

// 0049BF20: Redraws every tile row of the viewport via 0049BE20.
void __cdecl draw_all_tile_rows() {
    const u32 stride = g_map_tile_size->width;
    u32 tile_y = g_move_to_tile_y;
    auto row = *megatileArray + stride * tile_y + g_move_to_tile_x;

    for (u32 i = 0; i < g_display.map_tile_rows(); ++i) {
        draw_tile_row(row, tile_y, g_move_to_tile_x);
        ++tile_y;
        row += stride;
    }
}

// 0049C780: Redraws viewport terrain via 0049C620 per tile row.
void __cdecl redraw_terrain() {
    const u32 stride = g_map_tile_size->width;
    u32 tile_y = g_move_to_tile_y;
    const u32 first = stride * tile_y + g_move_to_tile_x;

    auto ids = *megatileArray + first;
    auto tile_ids = *tileIdArray + first;
    auto active = *g_active_tile_array_int + first;
    s32 offset = tile_offset(g_move_to_tile_x, tile_y);

    for (u32 i = 0; i < g_display.map_tile_rows(); ++i) {
        update_tile_row(ids, tile_ids, active, tile_y, offset);
        offset = next_tile_row(offset);
        ids += stride;
        tile_ids += stride;
        active += stride;
        ++tile_y;
    }
}

// 0049C0C0(edx = how far right).
u32 __cdecl scroll_right(s32 amount) {
    g_move_to_x += amount;
    if (g_move_to_x > g_move_to_x_max) {
        amount += g_move_to_x_max - g_move_to_x;
        g_move_to_x = g_move_to_x_max;
    }

    g_move_to_tile_x = u16(g_move_to_x >> 5);
    if (*g_screen_x == g_move_to_x) {
        return 0;
    }
    *g_screen_x = g_move_to_x;

    advance_parallax(g_parallax_x, amount, g_scratch_wrap_x, true);

    while (g_drawn_tile_x != g_move_to_tile_x) {
        const u16 column = u16(g_drawn_tile_x + g_display.map_tile_cols());
        ++g_drawn_tile_x;
        draw_tile_column(*megatileArray + u32(g_map_tile_size->width) * g_move_to_tile_y + column, column, g_move_to_tile_y);
    }
    return 1;
}

// 0049C1A0(eax = how far left).
u32 __cdecl scroll_left(s32 amount) {
    g_move_to_x -= amount;
    if (g_move_to_x < 0) {
        amount += g_move_to_x;
        g_move_to_x = 0;
    }

    g_move_to_tile_x = u16(g_move_to_x >> 5);
    if (*g_screen_x == g_move_to_x) {
        return 0;
    }
    *g_screen_x = g_move_to_x;

    advance_parallax(g_parallax_x, amount, g_scratch_wrap_x, false);

    while (g_drawn_tile_x != g_move_to_tile_x) {
        --g_drawn_tile_x;
        draw_tile_column(*megatileArray + u32(g_map_tile_size->width) * g_move_to_tile_y + g_drawn_tile_x, g_drawn_tile_x, g_move_to_tile_y);
    }
    return 1;
}

// 0049C280(edx = how far down).
u32 __cdecl scroll_down(s32 amount) {
    g_move_to_y += amount;
    if (g_move_to_y > g_move_to_y_max) {
        amount += g_move_to_y_max - g_move_to_y;
        g_move_to_y = g_move_to_y_max;
    }

    g_move_to_tile_y = u16(g_move_to_y >> 5);
    if (*g_screen_y == g_move_to_y) {
        return 0;
    }
    *g_screen_y = g_move_to_y;

    advance_parallax(g_parallax_y, amount, g_scratch_wrap_y, true);

    while (g_drawn_tile_y != g_move_to_tile_y) {
        const u16 row = u16(g_drawn_tile_y + g_display.map_tile_rows());
        ++g_drawn_tile_y;
        draw_tile_row(*megatileArray + u32(g_map_tile_size->width) * row + g_move_to_tile_x, row, g_move_to_tile_x);
    }
    return 1;
}

// 0049C360(eax = how far up).
u32 __cdecl scroll_up(s32 amount) {
    g_move_to_y -= amount;
    if (g_move_to_y < 0) {
        amount += g_move_to_y;
        g_move_to_y = 0;
    }

    g_move_to_tile_y = u16(g_move_to_y >> 5);
    if (*g_screen_y == g_move_to_y) {
        return 0;
    }
    *g_screen_y = g_move_to_y;

    advance_parallax(g_parallax_y, amount, g_scratch_wrap_y, false);

    while (g_drawn_tile_y != g_move_to_tile_y) {
        --g_drawn_tile_y;
        draw_tile_row(*megatileArray + u32(g_map_tile_size->width) * g_drawn_tile_y + g_move_to_tile_x, g_drawn_tile_y, g_move_to_tile_x);
    }
    return 1;
}

} // hooks
