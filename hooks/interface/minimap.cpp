#include "minimap.h"

#include "hooks/interface/uifeatures/ui_features.h"
#include <BW\api.h>
#include "systems/display/display.h"
#include "systems/display/zoom.h"
#include "BW/ui_scale.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace {

constexpr u32 div_round_up(u32 n, u32 d) {
    return n / d + (n % d != 0);
}

// Viewport box size on the minimap in minimap pixels, following the zoom-visible
// crop. Used by the local camera rectangle and the minimap click-to-jump
// centering; other players' camera boxes assume the 200% default instead
// (their zoom isn't synced).
u16 camera_box_width() {
    const u32 width = div_round_up(u32(g_zoom.visible_width()), *g_minimap_zoom_level);
    return u16(std::max(width, 2u));
}

u16 camera_box_height() {
    const u32 height = div_round_up(u32(g_zoom.visible_height() + 16), *g_minimap_zoom_level);
    return u16(std::max(height, 1u));
}

// Fused nearest-neighbor blit of the map-resolution composite into the current
// draw target at the dialog bounds — one write pass, no intermediate surface,
// replacing the expand-to-scratch + SBltROP3 pair (which wrote the doubled
// pixels once into the scratch and again through the per-pixel ROP). Each
// source row expands into an L1-resident row buffer, then lands as one clipped
// copy per ui_scale::Factor destination rows. The white viewport box draws
// into the same target at the same bounds-relative origin, so the spaces match.
void blit_composite_scaled(const BinDlg* minimap) {
    const Bitmap* source = g_minimap_surface_fog;
    const Bitmap* target = ((Bitmap*)(*g_current_surface));
    if (!source || !source->buffer || !target || !target->buffer) {
        return;
    }
    const s32 factor = s32(ui_scale::Factor);
    const s32 origin_x = minimap->bounds.left;
    const s32 origin_y = minimap->bounds.top;
    const s32 source_width = source->width;
    const s32 source_height = source->height;

    const s32 span_left = std::max<s32>(origin_x, 0);
    const s32 span_right = std::min<s32>(origin_x + source_width * factor, target->width);
    if (span_left >= span_right) {
        return;
    }

    static std::vector<u8> row_buffer;
    if (row_buffer.size() != size_t(source_width) * factor) {
        row_buffer.assign(size_t(source_width) * factor, 0);
    }

    u8* const target_buffer = target->buffer;
    const s32 target_width = target->width;
    for (s32 source_row_index = 0; source_row_index < source_height; ++source_row_index) {
        const u8* source_row = source->buffer + size_t(source_row_index) * source_width;
        for (s32 column = 0; column < source_width; ++column) {
            std::fill_n(row_buffer.data() + column * ui_scale::Factor, ui_scale::Factor, source_row[column]);
        }
        for (s32 sub = 0; sub < factor; ++sub) {
            const s32 dest_y = origin_y + source_row_index * factor + sub;
            if (dest_y < 0 || dest_y >= target->height) {
                continue;
            }
            u8* dest_row = target_buffer + size_t(dest_y) * target_width;
            std::copy_n(row_buffer.data() + (span_left - origin_x), span_right - span_left, dest_row + span_left);
        }
    }
}

// 0041D810/0041D7D0: the letterbox border's line writes, into the same surface
// the originals draw through, clipped to the surface where they wrote blind.
void letterbox_line_h(s32 x, s32 y, s32 length, u8 color) {
    const Bitmap* surface = *g_current_surface;
    if (!surface || !surface->buffer || y < 0 || y >= surface->height || length <= 0) {
        return;
    }
    const s32 start = std::max(x, 0);
    const s32 end = std::min(x + length, s32(surface->width));
    if (end > start) {
        std::fill_n(surface->buffer + size_t(y) * surface->width + start, end - start, color);
    }
}

void letterbox_line_v(s32 x, s32 y, s32 length, u8 color) {
    const Bitmap* surface = *g_current_surface;
    if (!surface || !surface->buffer || x < 0 || x >= surface->width || length <= 0) {
        return;
    }
    const s32 start = std::max(y, 0);
    const s32 end = std::min(y + length, s32(surface->height));
    for (s32 row = start; row < end; ++row) {
        surface->buffer[size_t(row) * surface->width + x] = color;
    }
}

std::unique_ptr<u8[]> g_minimap_surface_pixels;
std::unique_ptr<u8[]> g_minimap_fog_pixels;

// 004A3720: sizes the minimap descriptors and (re)allocates their pixel storage.
void allocate_minimap_surfaces(u32 map_width, u32 map_height) {
    g_minimap_surface_pixels = std::make_unique<u8[]>(size_t(map_width) * map_height);
    g_minimap_fog_pixels = std::make_unique<u8[]>(size_t(map_width) * map_height);

    Bitmap* surface = g_minimap_surface;
    Bitmap* fog_surface = g_minimap_surface_fog;
    surface->width = u16(map_width);
    surface->height = u16(map_height);
    surface->buffer = g_minimap_surface_pixels.get();
    fog_surface->width = u16(map_width);
    fog_surface->height = u16(map_height);
    fog_surface->buffer = g_minimap_fog_pixels.get();
}

} // namespace

s8 get_minimap_color(u8 player) {
    if (g_minimap_state != MinimapState::Normal && player < PlayerLimit) {
        if (g_minimap_state == MinimapState::RecolorGame || bw::in_replay_or_witness()) {
            auto special_color_desert = *g_current_tileset != TilesetType::Desert ? 23 : 19;
            auto special_color_ice = *g_current_tileset != TilesetType::Ice ? 58 : 19;
            switch (player) {
            case 0: return g_palette_index_reference[20];
            case 1: return g_palette_index_reference[16];
            case 2: return g_palette_index_reference[213];
            case 3: return g_palette_index_reference[234];
            case 4: return g_palette_index_reference[21];
            case 5: return g_palette_index_reference[special_color_desert];
            case 6: return g_palette_index_reference[special_color_ice];
            case 7: return g_palette_index_reference[17];
            }
        }
        if (bw::is_allied_to(g_local_nation_id, player) && player != 11) {
            return g_palette_index_reference[17];
        }
        return g_palette_index_reference[20];
    }
    return g_player_minimap_colors[player];
}

namespace hooks {
    int __fastcall minimapVisionUpdate_256() {
        u32 pos_y = g_map_height;
        if (pos_y) {
            u8* surf_buff = g_minimap_surface->buffer;
            u8* surf_buff_fog = g_minimap_surface_fog->buffer;
            ActiveTile* active_tile = *g_active_tile_array;
            do {
                u32 pos_x = g_map_width;
                do {
                    u32 cur_vision = (*g_player_visions & (*active_tile).flags) == 0;
                    u32 cur_filter = (*g_player_explored_visions & (*active_tile).flags) == 0;
                    if (g_is_in_replay) {
                        cur_filter = 0;
                        if (*g_replay_reveal_all) {
                            cur_vision = 1;
                        } else {
                            cur_filter = (u8)*g_replay_visions;
                            u32 tile_vision = ~(*active_tile).flags;

                            if ((cur_filter << 8) & tile_vision) {
                                if ((u8)tile_vision & cur_filter) {
                                    cur_vision = 1;
                                    cur_filter = 0;
                                } else {
                                    cur_vision = 0;
                                    cur_filter = 1;
                                }
                            } else {
                                cur_vision = 0;
                                cur_filter = 0;
                            }
                        }
                    }
                    *surf_buff_fog++ = g_minimap59C2C0[256 * (cur_filter | 2 * cur_vision) + *surf_buff++];
                    active_tile += 2;
                } while (--pos_x);

                active_tile += g_map_tile_size->width;
            } while (--pos_y);
        }
    return 0;
    }

    int __fastcall minimapVisionUpdate_128() {
        u32 pos = g_map_height * g_map_width;
        if (pos) {
            u8* surf_buff = g_minimap_surface->buffer;
            u8* surf_buff_fog = g_minimap_surface_fog->buffer;
            ActiveTile* active_tile = *g_active_tile_array;
            do {
                u32 cur_vision = (*g_player_visions & (*active_tile).flags) == 0;
                u32 cur_filter = (*g_player_explored_visions & (*active_tile).flags) == 0;
                if (g_is_in_replay) {
                    cur_filter = 0;
                    if (*g_replay_reveal_all) {
                        cur_vision = 1;
                    } else {
                        cur_filter = (u8)*g_replay_visions;
                        u32 tile_vision = ~(*active_tile).flags;

                        if ((cur_filter << 8) & tile_vision) {
                            if ((u8)tile_vision & cur_filter) {
                                cur_vision = 1;
                                cur_filter = 0;
                            } else {
                                cur_vision = 0;
                                cur_filter = 1;
                            }
                        } else {
                            cur_vision = 0;
                            cur_filter = 0;
                        }
                    }
                }
                *surf_buff_fog++ = g_minimap59C2C0[256 * (cur_filter | 2 * cur_vision) + *surf_buff++];
                ++active_tile;
            } while (--pos);
        }
    return 0;
    }

    int __fastcall minimapVisionUpdate_64() {
        u32 pos_y = g_map_height >> 1;

        if (pos_y) {
            u8* surf_buff = g_minimap_surface->buffer;
            u8* surf_buff_fog = g_minimap_surface_fog->buffer;
            u32 map_width = g_map_width;
            u32 buff_step = (g_minimap_surface_fog->width << 1) - map_width;
            u32 map_width_2 = map_width >> 1;
            ActiveTile* active_tile = *g_active_tile_array;

            do {
                u32 pos_x = map_width_2;

                do {
                    u32 cur_vision = (*g_player_visions & (*active_tile).flags) == 0;
                    u32 cur_filter = (*g_player_explored_visions & (*active_tile).flags) == 0;

                    if (g_is_in_replay) {
                        cur_filter = 0;

                        if (*g_replay_reveal_all) {
                            cur_vision = 1;
                        } else {
                            cur_filter = (u8)*g_replay_visions;
                            u32 tile_vision = ~(*active_tile).flags;

                            if ((cur_filter << 8) & tile_vision) {
                                if ((u8)tile_vision & cur_filter) {
                                    cur_vision = 1;
                                    cur_filter = 0;
                                } else {
                                    cur_vision = 0;
                                    cur_filter = 1;
                                }
                            } else {
                                cur_vision = 0;
                                cur_filter = 0;
                            }
                        }
                    }

                    u32 cur = (cur_filter | 2 * cur_vision) << 8;
                    *surf_buff_fog = g_minimap59C2C0[*surf_buff + cur];
                    *(surf_buff_fog + 1) = g_minimap59C2C0[*(surf_buff + 1) + cur];
                    *(surf_buff_fog + map_width) = g_minimap59C2C0[*(surf_buff + map_width) + cur];
                    *(surf_buff_fog + map_width + 1) = g_minimap59C2C0[*(surf_buff + map_width + 1) + cur];
                    surf_buff += 2;
                    surf_buff_fog += 2;
                    ++active_tile;
                } while (--pos_x);

                surf_buff += buff_step;
                surf_buff_fog += buff_step;
            } while (--pos_y);
        }
    return 0;
    }

    int __fastcall minimapVisionUpdate_staircase() {
        u32 pos_y = g_map_height;
        if (pos_y) {
            u8* surf_buff = g_minimap_surface->buffer;
            u8* surf_buff_fog = g_minimap_surface_fog->buffer;
            ActiveTile* active_tile = *g_active_tile_array;
            u32 tile_y = 0;

            do {
                u32 tile_x = 0;
                u32 pos_x = g_map_width;
                u16 tileset_row = (tile_y >> 5) * g_map_tile_size->width;

                do {
                    ActiveTile cur_active_tile = active_tile[(tile_x >> 5) + tileset_row];
                    u32 cur_vision = (*g_player_visions & cur_active_tile.flags) == 0;
                    u32 cur_filter = (*g_player_explored_visions & cur_active_tile.flags) == 0;

                    if (g_is_in_replay) {
                        cur_filter = 0;

                        if (*g_replay_reveal_all) {
                            cur_vision = 1;
                        } else {
                            cur_filter = (u8)*g_replay_visions;
                            u32 tile_vision = ~cur_active_tile.flags;

                            if ((cur_filter << 8) & tile_vision) {
                                if ((u8)tile_vision & cur_filter) {
                                    cur_vision = 1;
                                    cur_filter = 0;
                                } else {
                                    cur_vision = 0;
                                    cur_filter = 1;
                                }
                            } else {
                                cur_vision = 0;
                                cur_filter = 0;
                            }
                        }
                    }

                    *surf_buff_fog++ = g_minimap59C2C0[256 * (cur_filter | 2 * cur_vision) + *surf_buff++];
                    tile_x += *g_minimap_zoom_level;
                } while (--pos_x);

                tile_y += *g_minimap_zoom_level;
            } while (--pos_y);
        }
    return 0;
    }

    void minimapSurfaceUpdate_256() {
        u32 pos_y = g_map_height;

        if (pos_y) {
            u16* map_tileset_handle = (u16*)*megatileArray;
            u8* surf_buff = g_minimap_surface->buffer;
            VX4* vx4_handle = (VX4*)(*g_vx4_file_handle);
            VR4* vr4_handle = (VR4*)(*g_vr4_file_handle);

            do {
                u32 pos_x = g_map_width;

                do {
                    u32 lookup = *map_tileset_handle;
                    map_tileset_handle += 2;
                    lookup = vx4_handle->tile[lookup][0].vr4_id;
                    lookup = vr4_handle->minitile[lookup][55];
                    *surf_buff++ = g_minimap59CB60[lookup];
                } while (--pos_x);
            
                map_tileset_handle += g_map_tile_size->width;
            } while (--pos_y);
        }
    }

    void minimapSurfaceUpdate_128() {
        u32 buff_size = g_map_height * g_map_width;

        if (buff_size) {
            u16* map_tileset_handle = (u16*)*megatileArray;
            u8* surf_buff = ((Bitmap*)g_minimap_surface)->buffer;
            VX4* vx4_handle = (VX4*)(*g_vx4_file_handle);
            VR4* vr4_handle = (VR4*)(*g_vr4_file_handle);

            do {
                u32 lookup = *map_tileset_handle;
                ++map_tileset_handle;
                lookup = vx4_handle->tile[lookup][0].vr4_id;
                lookup = vr4_handle->minitile[lookup][55];
                *surf_buff++ = g_minimap59CB60[lookup];
            } while (--buff_size);
        }
    }

    void minimapSurfaceUpdate_64() {
        u32 pos_y = g_map_height >> 1;

        if (pos_y) {
            u16* map_tileset_handle = (u16*)*megatileArray;
            u8* surf_buff = g_minimap_surface->buffer;
            u32 map_width = g_map_width;
            u32 buff_step = (g_minimap_surface->width << 1) - map_width;
            u32 map_width_2 = map_width >> 1;
            VX4* vx4_handle = (VX4*)(*g_vx4_file_handle);
            VR4* vr4_handle = (VR4*)(*g_vr4_file_handle);

            do {
                u32 pos_x = map_width_2;

                do {
                    u32 lookup = *map_tileset_handle;
                    ++map_tileset_handle;
                    vx4* tile = vx4_handle->tile[lookup];
                    lookup = tile[0].vr4_id;
                    lookup = vr4_handle->minitile[lookup][55];
                    *surf_buff = g_minimap59CB60[lookup];
                    lookup = tile[4].vr4_id;
                    lookup = vr4_handle->minitile[lookup][55];
                    *(surf_buff + map_width) = g_minimap59CB60[lookup];
                    lookup = tile[1].vr4_id;
                    lookup = vr4_handle->minitile[lookup][55];
                    *(surf_buff + 1) = g_minimap59CB60[lookup];
                    lookup = tile[5].vr4_id;
                    lookup = vr4_handle->minitile[lookup][55];
                    *(surf_buff + map_width + 1) = g_minimap59CB60[lookup];
                    surf_buff += 2;
                } while (--pos_x);

                surf_buff += buff_step;
            } while (--pos_y);
        }
    }

    void minimapSurfaceUpdate_staircase() {
        u32 pos_y = g_map_height;

        if (pos_y) {
            u16* map_tileset_handle = (u16*)*megatileArray;
            u8* surf_buff = ((Bitmap*)g_minimap_surface)->buffer;
            VX4* vx4_handle = (VX4*)(*g_vx4_file_handle);
            VR4* vr4_handle = (VR4*)(*g_vr4_file_handle);
            u32 tile_y = 0;

            do {
                u32 tile_x = 0;
                u32 pos_x = g_map_width;
                u16 tileset_row = (tile_y >> 5) * g_map_tile_size->width;
                u8 minitile_row = ((tile_y >> 3) & 3) * 4;

                do {
                    u32 lookup = map_tileset_handle[(tile_x >> 5) + tileset_row];
                    lookup = vx4_handle->tile[lookup][((tile_x >> 3) & 3) + minitile_row].vr4_id;
                    lookup = vr4_handle->minitile[lookup][55];
                    *surf_buff++ = g_minimap59CB60[lookup];
                    tile_x += *g_minimap_zoom_level;
                } while (--pos_x);

                tile_y += *g_minimap_zoom_level;
            } while (--pos_y);
        }
    }

    void __cdecl set_map_size_constants() {//TODO: Doesn't handle map dimensions smaller than 64.
        u32 map_width = g_map_tile_size->width;
        u32 map_height = g_map_tile_size->height;
        u32 map_size = map_width <= map_height ? map_height : map_width;

        if (!(map_size & 0x3F)) {//Opting against a switch statement because it generates a large jump table and this code is executed only once on mission start, so I consider it wasteful.
            if (map_size == 64) {
                map_width <<= 1;
                map_height <<= 1;
                *g_minimap_surface_CB = minimapSurfaceUpdate_64;
                g_minimap_vision_callback = minimapVisionUpdate_64;
            } else if (map_size == 128) {
                *g_minimap_surface_CB = minimapSurfaceUpdate_128;
                g_minimap_vision_callback = minimapVisionUpdate_128;
            } else if (map_size == 256) {
                map_width >>= 1;
                map_height >>= 1;
                *g_minimap_surface_CB = minimapSurfaceUpdate_256;
                g_minimap_vision_callback = minimapVisionUpdate_256;
            } else { // map size == 192
                map_width <<= 1;
                map_width = div_round_up(map_width, 3);
                map_height <<= 1;
                map_height = div_round_up(map_height, 3);
                *g_minimap_surface_CB = minimapSurfaceUpdate_staircase;
                g_minimap_vision_callback = minimapVisionUpdate_staircase;
            }
        } else if (map_size == 96) {
            map_width <<= 2;
            map_width = div_round_up(map_width, 3);
            map_height <<= 2;
            map_height = div_round_up(map_height, 3);
            *g_minimap_surface_CB = minimapSurfaceUpdate_staircase;
            g_minimap_vision_callback = minimapVisionUpdate_staircase;
        } else {
            float scale = 128.0F / (float)map_size;
            map_width = (u32)ceil(map_width * scale);
            map_height = (u32)ceil(map_height * scale);
            *g_minimap_surface_CB = minimapSurfaceUpdate_staircase;
            g_minimap_vision_callback = minimapVisionUpdate_staircase;
        }

        u32 zoom_level = div_round_up(map_size, 4);
        *g_minimap_zoom_level = zoom_level;
        // Height retains the engine's +16px centering slack (see camera_box_height).
        g_map_width = map_width;
        g_map_height = map_height;
        BinDlg* minimap_dlg = *g_minimap_dlg;
        s16 offset = (128 - map_width) >> 1;
        minimap_dlg->bounds.left += offset;
        minimap_dlg->bounds.right -= offset;
        offset = (128 - map_height) >> 1;
        minimap_dlg->bounds.top += offset;
        minimap_dlg->bounds.bottom -= offset;

        allocate_minimap_surfaces(map_width, map_height);

        refresh_minimap_rect(minimap_dlg);
    }

    void minimap_ping(s32 pos_x, s32 pos_y, u8 color) {
        Ping* ping_array = (*minimap_ping_array).ping_array;

        while (ping_array->isActive) {
            if (!ping_array[1].isActive) {
                ++ping_array;
                break;
            }

            if (!ping_array[2].isActive) {
                ping_array += 2;
                break;
            }

            if (!ping_array[3].isActive) {
                ping_array += 3;
                break;
            }

            if ((ping_array += 4) == (*minimap_ping_array).ping_array + 16)
                return;
        }

        ping_array->isActive = true;
        u32 zoom_level = *g_minimap_zoom_level;
        ping_array->x = (pos_x / zoom_level) - 64;
        ping_array->y = (pos_y / zoom_level) - 64;
        ping_array->timer = 0;
        ping_array->player = color;
    }

    // Republishes g_minimap_rect with root dialog offset added, ensuring hit test (PtInRect @ 004D1460, clicks, right-clicks) matches widescreen HUD placement.
    const RECT& refresh_minimap_rect(BinDlg* minimap) {
        RECT& rect = *g_minimap_rect;
        if (minimap && minimap->parent) {
            const Box16& root = minimap->parent->bounds;
            rect.left = minimap->bounds.left + root.left;
            rect.top = minimap->bounds.top + root.top;
            rect.right = minimap->bounds.right + root.left;
            rect.bottom = minimap->bounds.bottom + root.top;
        }
        return rect;
    }

    void __fastcall minimap_overlay_draw(bw::Dialog* dialog, DialogEvent* event, u32 user, void* draw_ctx) {
        helper::g_default_dialog_draw_functions[std::to_underlying(dialog->type)](dialog, event, user, draw_ctx);

        bw::Dialog* root = dialog->type != DialogControlType::DialogBoxDC ? dialog->parent : dialog;
        bw::Dialog** children = root ? root->child<bw::Dialog*>() : nullptr;
        bw::Dialog* ctrl = children ? *children : nullptr;
        while (ctrl && ctrl->index != 1) {  // vanilla: the minimap control is index 1
            ctrl = ctrl->next;
        }
        if (!ctrl || !(ctrl->flags & DialogFlags::Visible)) {
            return;
        }

        const DialogBounds& bounds = ctrl->bounds;
        const s32 width = g_map_width;
        const s32 height = g_map_height;
        const s32 factor = s32(ui_scale::Factor);
        const u8 light = g_palette_index_reference[2];
        const u8 dark = g_palette_index_reference[5];

        const s32 box_width = bounds.right() - bounds.left() + 1;
        const s32 box_height = bounds.bottom() - bounds.top() + 1;

        // With the neutrals preview (or a map too small to fill either axis), all
        // four sides are outlined instead of only the letterboxed ones.
        if (!g_minimap_draw_neutrals) {
            if (width >= 128) {
                if (height < 128) {
                    letterbox_line_h(bounds.left(), bounds.top() - factor, box_width, light);
                    letterbox_line_h(bounds.left(), bounds.bottom() + factor, box_width, dark);
                }
                return;
            }
            if (height >= 128) {
                letterbox_line_v(bounds.right() + factor, bounds.top(), box_height, light);
                letterbox_line_v(bounds.left() - factor, bounds.top(), box_height, dark);
                return;
            }
        }
        letterbox_line_v(bounds.right() + factor, bounds.top() - factor, box_height + 2 * factor, light);
        letterbox_line_h(bounds.left() - factor, bounds.top() - factor, box_width + 2 * factor, light);
        letterbox_line_v(bounds.left() - factor, bounds.top() - factor, box_height + 2 * factor, dark);
        letterbox_line_h(bounds.left() - factor, bounds.bottom() + factor, box_width + 2 * factor, dark);
    }

    void __fastcall minimap_image_update(BinDlg* minimap, DialogEvent* event, u32 user, void* draw_context) {
        refresh_minimap_rect(minimap);

        // The composite stays at map resolution — terrain, fog, unit boxes, camera
        // boxes, and pings all write map-res pixels into it — so the blit scales it
        // to the dialog's ui_scale::Factor interior on the way out.
        blit_composite_scaled(minimap);

        if (!g_minimap_draw_neutrals) {
            u32 zoom_level = *g_minimap_zoom_level;
            // Minimap pixels scale to the dialog interior; the box draws on top of the
            // expanded blit at the same bounds-relative origin.
            u32 box_y = (u32)minimap->bounds.top + (g_move_to_y + g_zoom.source_top()) / zoom_level * ui_scale::Factor;
            u32 box_x = (u32)minimap->bounds.left + (g_move_to_x + g_zoom.source_left()) / zoom_level * ui_scale::Factor;
            const u16 visible_box_width = u16(camera_box_width() * ui_scale::Factor);

            // inline and simplification of DrawRectangle @ 0x004A37A0 - GPTP
            u32 surf_w = ((Bitmap*)(*g_current_surface))->width;
            u8* surf_buff = ((Bitmap*)(*g_current_surface))->buffer + surf_w * box_y + box_x;
            u32 height = u16(camera_box_height() * ui_scale::Factor) - 1;
            u32 bounds_bottom = (u32)minimap->bounds.bottom;

            if (box_y + height >= bounds_bottom) {
                height = bounds_bottom > box_y ? bounds_bottom - box_y : 0;
            }
            if (height == 0) {
                return;
            }

            u8 remainder = visible_box_width & 3;
            u32 _surf_w = surf_w * height;
            u32 width = visible_box_width >> 2;
            u32 color = g_palette_index_reference[0];
            color |= (color << 24) | (color << 16) | (color << 8);
            s32 row = 1;

            do {
                u8* dest = &surf_buff[row * _surf_w];

                if (remainder) {
                    u32 _is_odd = width * 4 + remainder;

                    do {
                        dest[--_is_odd] = (u8)color;
                    } while (_is_odd & 3);
                }

                u32 rep = width;

                do {
                    (u32&)dest[--rep * 4] = color;
                } while (rep);
            } while (--row >= 0);

            // similar to DrawVerticalLine @ 0x0041D7D0, but draws both lines simultaneously -GPTP
            width = visible_box_width;
            row = height - 1;

            do {
                u8* dest = &surf_buff[row * surf_w];
                dest[0] = (u8)color;
                dest[width - 1] = (u8)color;
            } while (--row >= 0); // height == 1 must stop at row 0; --row alone spins on negative row
        }
    }

    void draw_unit_box(s32 posX, u8 color, s32 posY, u32 sizeX, u32 sizeY, bool is_building) {
        ++*g_minimap_unit_box_count;
        s32 size_x;
        s32 size_y;
        u32 zoom_level = *g_minimap_zoom_level;

        if (is_building) {
            size_x = sizeX >> 1;
            size_y = sizeY >> 1;
            sizeX = div_round_up(sizeX, zoom_level);
            sizeY = div_round_up(sizeY, zoom_level);

            if (sizeX < 2) {
                sizeX = 2;
            }

            if (sizeY < 2) {
                sizeY = 2;
            }
        } else {
            size_x = 32;
            size_y = 32;
            sizeX = 2;
            sizeY = 2;
        }

        posX -= size_x;
        if (posX < 0) {
            posX = 0;
        }
        posY -= size_y;
        if (posY < 0) {
            posY = 0;
        }

        u32 pos_x = div_round_up(posX, zoom_level);
        u32 pos_y = div_round_up(posY, zoom_level);

        if (pos_x + sizeX >= g_map_width) {
            pos_x = g_map_width - sizeX;
        }

        if (pos_y + sizeY >= g_map_height) {
            pos_y = g_map_height - sizeY;
        }

        // inline and simplification of DrawBox @ 0x004E1D20 -GPTP
        u32 surf_w = ((Bitmap*)(*g_current_surface))->width;
        u8* surf_buff = ((Bitmap*)(*g_current_surface))->buffer + surf_w * pos_y + pos_x;
        u8 remainder = sizeX & 1;
        u32 width = sizeX >> 1;
        u16 _color = (color << 8) | color;
        s32 row = sizeY - 1;

        do {
            u8* dest = &surf_buff[row * surf_w];

            if (remainder) {
                dest[width * 2] = (u8)_color;
            }

            u32 rep = width;

            do {
                (u16&)dest[--rep * 2] = _color;
            } while (rep);
        } while (--row >= 0);
    }

    // 004A3D70: Converts minimap pixel coords to world map coords via g_minimap_rect.
    u16 __cdecl minimap_cursor_to_world(u16* x, u16* y) {
        const RECT& rect = refresh_minimap_rect(*g_minimap_dlg);
        const s32 zoom_level = s32(*g_minimap_zoom_level);

        // The rect lives in ui_scale::Factor band coordinates; the surface is map-resolution.
        s32 pos_x = (s32(s16(*x)) - rect.left) / s32(ui_scale::Factor);
        pos_x = std::clamp(pos_x, 0, s32(g_minimap_surface->width) - 1);
        *x = u16(pos_x * zoom_level);

        s32 pos_y = (s32(s16(*y)) - rect.top) / s32(ui_scale::Factor);
        pos_y = std::clamp(pos_y, 0, s32(g_minimap_surface->height) - 1);
        *y = u16(pos_y * zoom_level);

        return *y;
    }

    // Centers the visible crop by backing off half the visible box from the click.
    void minimap_game_mouse_update(bw::Dialog* minimap) {
        u16 pos_x = (*g_mouse).x - (camera_box_width() >> 1) * ui_scale::Factor;
        u16 pos_y = (*g_mouse).y - (camera_box_height() >> 1) * ui_scale::Factor;
        helper::get_minimap_cursor_pos(&pos_x, &pos_y);

        const v2s32 camera = g_zoom.camera_for_visible_origin({ s32(pos_x), s32(pos_y) });
        if ((camera.x & -8) != g_move_to_x || (camera.y & -8) != g_move_to_y) {
            helper::move_screen({ u16(camera.x), u16(camera.y) });
        }

        if (!(minimap->flags & DialogFlags::UpdateNeeded)) {
            minimap->flags |= DialogFlags::UpdateNeeded;
            helper::update_dialog(minimap);
        }
    }

    void minimap_timer() {
        static u32 last_frame = u32(-1);
        static s32 last_move_x = -1;
        static s32 last_move_y = -1;

        const bool surface_changed = g_minimap_surface_changed != 0;
        const bool view_changed = g_move_to_x != last_move_x || g_move_to_y != last_move_y;
        const bool frame_changed = g_elapsed_time_frames != last_frame;
        if (!surface_changed && !view_changed && !frame_changed) {
            return;
        }
        last_frame = g_elapsed_time_frames;
        last_move_x = g_move_to_x;
        last_move_y = g_move_to_y;

        if (surface_changed && *g_minimap_surface_CB) {
            reinterpret_cast<void(__cdecl*)()>(*g_minimap_surface_CB)();
        }
        helper::show_or_hide_minimap();
        draw_all_minimap_boxes();
        helper::draw_minimap_pings();

        BinDlg* dialog = *g_minimap_dlg;
        if (dialog) {
            if (!(dialog->flags & DialogFlags::UpdateNeeded)) {
                dialog->flags |= DialogFlags::UpdateNeeded;
                helper::update_dialog(dialog);
            }
        }
        g_minimap_surface_changed = 0;
    }
}