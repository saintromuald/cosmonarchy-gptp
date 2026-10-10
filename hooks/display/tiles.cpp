#include "tiles.h"

#include "terrain.h"
#include "../../BW/bwhelper.h"

#include <array>

namespace {

constexpr u32 MaxMapTiles = 256 * 256;

// The creep-border-variant flag (chk tile_id bit 15, "tile group >= 2048" under the
// shipped 12-bit group extension), relocated out of the megatile id where extended
// ids need every bit. Replaces aise.qdp's "shifted array" inside the megatile
// allocation. Written by set_map_tiles, read by draw_megatile and the update paths.
std::array<u8, MaxMapTiles> g_creep_border_variants = {};

// 0x00414440: vanilla's creep border refresh callback, passed to
// iterate_directional_creep_data by the update paths. Stays vanilla, so the
// tile-group mask widenings inside it remain in tiles_inject.cpp.
constexpr u32 CreepBorderCallback = 0x00414440;

// The tile group index of a chk tile id: bits 4-15, 12-bit per the shipped group
// extension. The single decode point a future u32 tile-id format would widen.
u32 tile_group(u16 tile_id) {
    return (tile_id >> 4) & 0xFFF;
}

// The megatile id a chk tile id resolves to through the cv5 tile group table.
u16 group_megatile(const cv5* tile_groups, u16 tile_id) {
    return tile_groups[tile_group(tile_id)].tiles[tile_id & 0xF];
}

bool tile_mismatches(const cv5* tile_groups, u16 tile_id, u16 stored_megatile, u32 tile_index) {
    // The creep-variant flag keeps flagged tiles in the mismatch state, matching the
    // shipped aise comparison: creep border changes get picked up by the update passes.
    const bool creep_variant = g_creep_border_variants[tile_index] & 0x80;
    return group_megatile(tile_groups, tile_id) != stored_megatile || creep_variant;
}

// 0049C620 / 0049C4C0 vision gate: tiles the active player cannot see get deferred
// updates; visible tiles are redrawn directly by the creep spread paths.
bool tile_needs_deferred_update(u32 tile_flags) {
    if (g_is_in_replay) {
        return (~u8(tile_flags) & (u8)*g_replay_visions) != 0;
    }
    return (tile_flags & *g_player_visions) == 0;
}

} // namespace

namespace hooks {

// 0040C3B0: blits one 8x8 VR4 minitile (byte per pixel) into the world scratch
// buffer at the given offset, wrapping at map_size. Horizontal flip byte-reverses
// each row. Replaces the ui_features hook; vanilla draw_megatile was its only
// caller, so this is now called directly.
void blit_minitile(s32 offset, vx4 ref) {
    const s32 map_size = s32(g_display.map_size());
    const s32 map_pitch = s32(g_display.map_pitch());
    u8* map_data = g_game_map_data_for_blit;
    const u8* minitile = (const u8*)(*g_vr4_file_handle)->minitile[ref.vr4_id];

    if (ref.flipped) {
        for (u32 row = 0; row < 8; ++row) {
            if (offset >= map_size) {
                offset -= map_size;
            }
            u32* dest = (u32*)&map_data[offset];
            dest[0] = (minitile[4] << 24) | (minitile[5] << 16) | (minitile[6] << 8) | minitile[7];
            dest[1] = (minitile[0] << 24) | (minitile[1] << 16) | (minitile[2] << 8) | minitile[3];
            minitile += 8;
            offset += map_pitch;
        }
    } else {
        for (u32 row = 0; row < 8; ++row) {
            if (offset >= map_size) {
                offset -= map_size;
            }
            u32* dest = (u32*)&map_data[offset];
            dest[0] = ((const u32*)minitile)[0];
            dest[1] = ((const u32*)minitile)[1];
            minitile += 8;
            offset += map_pitch;
        }
    }
}

// 0049B9F0: draws one 32x32 megatile into the world scratch buffer as a 4x4 grid of
// 8x8 minitiles, then the creep border overlay. Replaces vanilla plus vx4ex.qdp's
// u32 ref widening, aise.qdp's 0049BB08 stub, and the widescreen pitch patches.
void draw_megatile(s32 tile_x, s32 tile_y, u16 megatile, s32 offset) {
    const vx4* refs = (*g_vx4_file_handle)->tile[megatile];
    const s32 map_pitch = s32(g_display.map_pitch());

    for (u32 row = 0; row < 4; ++row) {
        const s32 row_offset = offset + s32(row) * 8 * map_pitch;
        for (u32 col = 0; col < 4; ++col) {
            blit_minitile(row_offset + s32(col) * 8, refs[row * 4 + col]);
        }
    }

    // Creep tail (vanilla 0049BB13 + the aise stub): draw the border overlay when
    // the tile carries the creep-border-variant flag and creep currently borders it,
    // except on plain creep terrain without the cv5 group flag.
    const u32 tile_index = u32(tile_y) * g_map_tile_size->width + u32(tile_x);
    bool creep_variant = g_creep_border_variants[tile_index] & 0x80;
    const u32 flags = (*g_active_tile_array_int)[tile_index];
    if ((flags & 0x40000000) && !(flags & 0x800000)) {
        creep_variant = false;
    }
    const u8 border = g_creep_tile_borders[*g_map_creep_width * u32(tile_y) + u32(tile_x)];
    if (creep_variant && border != 0) {
        blit_overlay(g_creep_border_records + border * 8 - 2, offset);
    }
}

// 004BCEA0: resolves chk tile ids into megatile ids and tile flags at map load.
// Absorbs the initialize.cpp tile-group patches (12-bit groups, full-id buildability
// class lookup) and aise.qdp's shifted-array creep flag writes.
void set_map_tiles() {
    const u16* tile_ids = *tileIdArray;
    u16* megatiles = *megatileArray;
    u32* tile_flags = *g_active_tile_array_int;
    const cv5* tile_groups = (const cv5*)g_cv5_file_handle;
    const u32* build_classes = g_megatile_classes;

    const u32 count = u32(g_map_tile_size->width) * u32(*g_map_tile_height);
    for (u32 i = 0; i < count; ++i) {
        const u16 tile_id = tile_ids[i];
        const cv5& group = tile_groups[tile_group(tile_id)];
        const u16 megatile_id = group.tiles[tile_id & 0xF];

        megatiles[i] = megatile_id;
        tile_flags[i] = (tile_flags[i] & 0xFFFF) | ((u32(group.flags) & 0xD8FA) << 16) | build_classes[megatile_id];
        g_creep_border_variants[i] = (tile_id & 0x8000) ? 0x80 : 0;
    }
}

// 0047E2D0: runtime single-tile update, called by the creep growth system and
// tile triggers. Mode 2 = creep arrival: mark the creep-border variant and request
// a redraw. Modes 0/1 set/clear the creep-terrain flag and update the megatile id
// on tiles the player cannot see (visible tiles are redrawn directly). Every call
// recomputes the buildability-class bits and refreshes neighboring creep groups.
void update_single_tile(s32 tile_x, s32 tile_y, u32 mode, u16 tile_id, const void* refresh_rect) {
    const cv5* tile_groups = (const cv5*)g_cv5_file_handle;
    const u32 tile_index = u32(tile_y) * g_map_tile_size->width + u32(tile_x);
    u32* tile_flags_entry = &(*g_active_tile_array_int)[tile_index];

    if (mode == 2) {
        g_creep_border_variants[tile_index] |= 0x80;
        refresh_map_area(refresh_rect);
        return;
    }

    const u32 flags = *tile_flags_entry;
    const bool hidden = !g_is_in_replay
        ? (flags & *g_player_visions) == 0
        : ((~u8(flags) & (u8)*g_replay_visions) != 0 || *g_replay_reveal_all != 0);
    if (hidden) {
        (*megatileArray)[tile_index] = group_megatile(tile_groups, tile_id);
        *tile_flags_entry = mode == 0 ? flags | 0x40000000 : flags & 0xBFFFFFFF;
        refresh_map_area(refresh_rect);
    }

    // Recompute the buildability-class bits from cv5 flags and the megatile classes.
    const cv5& group = tile_groups[tile_group(tile_id)];
    const u32 classes = ((u32(group.flags) & 0xFFFA) << 16) | g_megatile_classes[group.tiles[tile_id & 0xF]];
    u32 new_flags = *tile_flags_entry;
    new_flags ^= (classes ^ new_flags) & 0x450000;
    if (new_flags & 0x10000) {
        new_flags &= ~0x40000;
    }
    if ((new_flags & 0x450000) == 0) {
        new_flags |= 0x40000;
    }
    *tile_flags_entry = new_flags;
    update_creep_groups_near(tile_x, tile_y);
}

// 0049C620: checks and redraws changed tiles for one viewport row. Called from
// redraw_terrain; sole caller, so no entry hook is needed.
void update_tile_row(u16* megatiles, const u16* tile_ids, u32* tile_flags, u32 tile_y, s32 offset) {
    if (tile_y >= *g_map_tile_height) {
        return;
    }

    const u32 cols = g_display.map_tile_cols();
    u32 count = u32(g_map_tile_size->width) - g_move_to_tile_x;
    if (count > cols) {
        count = cols;
    }
    if (count == 0) {
        return;
    }

    const cv5* tile_groups = (const cv5*)g_cv5_file_handle;
    const s32 map_size = s32(g_display.map_size());
    u32 tile_x = g_move_to_tile_x;
    bool changed = false;

    for (u32 i = 0; i < count; ++i) {
        if (tile_needs_deferred_update(*tile_flags)) {
            const u16 tile_id = *tile_ids;
            const u32 tile_index = tile_y * g_map_tile_size->width + tile_x;
            if (tile_mismatches(tile_groups, tile_id, *megatiles, tile_index)) {
                const u16 expected = group_megatile(tile_groups, tile_id);
                *megatiles = expected;
                *tile_flags = (tile_id & 0xFFF0) == 0x10 ? *tile_flags | 0x40000000 : *tile_flags & 0xBFFFFFFF;
                helper::iterate_directional_creep_data(tile_x, tile_y, CreepBorderCallback, 0);
                draw_megatile(s32(tile_x), s32(tile_y), expected, offset);
                changed = true;
            }
        }
        ++tile_ids;
        ++megatiles;
        ++tile_flags;
        ++tile_x;
        if (offset < map_size - s32(display::TileSize)) {
            offset += display::TileSize;
        } else {
            offset += display::TileSize - map_size;
        }
    }

    if (changed) {
        g_minimap_surface_changed = 1;
    }
}

// 0049C4C0: whole-map pass that refreshes tile data outside the viewport while
// creep is receding. Same mismatch rule as update_tile_row, but no drawing:
// the tiles are offscreen and get drawn when they scroll back into view.
void progress_creep_disappearance() {
    const cv5* tile_groups = (const cv5*)g_cv5_file_handle;
    const u32 width = g_map_tile_size->width;
    const u32 height = *g_map_tile_height;
    const u32 cols = g_display.map_tile_cols();
    const u32 rows = g_display.map_tile_rows();
    bool changed = false;

    for (u32 tile_y = 0; tile_y < height; ++tile_y) {
        for (u32 tile_x = 0; tile_x < width; ++tile_x) {
            const u32 tile_index = tile_y * width + tile_x;
            const u32 flags = (*g_active_tile_array_int)[tile_index];
            if (!tile_needs_deferred_update(flags)) {
                continue;
            }
            const bool off_viewport = tile_x - g_move_to_tile_x >= cols || tile_y - g_move_to_tile_y >= rows;
            if (!off_viewport) {
                continue;
            }

            const u16 tile_id = (*tileIdArray)[tile_index];
            if (tile_mismatches(tile_groups, tile_id, (*megatileArray)[tile_index], tile_index)) {
                (*megatileArray)[tile_index] = group_megatile(tile_groups, tile_id);
                (*g_active_tile_array_int)[tile_index] = (tile_id & 0xFFF0) == 0x10
                    ? flags | 0x40000000
                    : flags & 0xBFFFFFFF;
                helper::iterate_directional_creep_data(tile_x, tile_y, CreepBorderCallback, 0);
                changed = true;
            }
        }
    }

    if (changed) {
        g_minimap_surface_changed = 1;
    }
}

} // hooks