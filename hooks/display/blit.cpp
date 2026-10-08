#include "blit.h"
#include "../../BW/bwdata.h"
#include "../../BW/bwhelper.h"
#include "../../graphics/Bitmap.h"
#include "../../systems/display/frame_overlay.h"
#include "../../systems/graphics/translucency.h"
#include "../../storm/storm.h"

#include <algorithm>
#include <cstddef>

namespace {

struct SurfaceNode {
    u32 reserved;         // +0x00
    SurfaceNode* next;    // +0x04
    void* surface;        // +0x08
    u32 left;             // +0x0c
    u32 top;              // +0x10
    u32 right;           // +0x14
    u32 bottom;           // +0x18
};

} // namespace

namespace hooks {

void __cdecl blit_rect(const s32* rect) { // 0041D3A0
    void* dest = nullptr;
    s32 dest_pitch = 0;
    if (!SDrawLockSurface(0, nullptr, &dest, &dest_pitch, 0)) {
        return;
    }

    const s32 left = rect[0];
    const s32 top = rect[1];
    const s32 width = rect[2] - left;
    const s32 height = rect[3] - top;
    const s32 src_pitch = g_display.surface_width();

    // Direct row copy: the SBltROP3 executor stages rows through a scratch capped near
    // 640px (the same limitation translucent_fx_blit replaces), which 1920-wide rects
    // can't rely on, and a plain copy is cheaper than the ROP dispatch anyway.
    auto out = (u8*)dest + size_t(top) * dest_pitch + left;
    auto in = g_game_screen_buffer->data + size_t(top) * src_pitch + left;
    for (s32 row = 0; row < height; ++row) {
        std::copy_n(in, width, out);
        out += dest_pitch;
        in += src_pitch;
    }

    SDrawUnlockSurface(0, dest, 1, (void*)rect);
}

void __cdecl blit_screen() { // 0041D420
    const u16 width = g_display.surface_width();
    g_frame_overlay.draw(g_game_screen_buffer->data, width, g_display.surface_height(), width);

    void* dest = nullptr;
    s32 dest_pitch = 0;
    if (!SDrawLockSurface(0, nullptr, &dest, &dest_pitch, 0)) {
        return;
    }

    STransBltUsingMask(dest, g_game_screen_buffer->data, dest_pitch, width, g_storm_update_region);

    SDrawUnlockSurface(0, dest, 0, nullptr);
}

s32 __cdecl surface_setup() { // 0041D470
    SurfaceNode* node = (g_surface_descriptors > 0) ? (SurfaceNode*)g_surface_descriptors : nullptr;

    if (g_storm_blit_chain) {
        STransDelete(g_storm_blit_chain);
        g_storm_blit_chain = nullptr;
    }
    if (g_storm_update_region) {
        STransDelete(g_storm_update_region);
        g_storm_update_region = nullptr;
    }

    void* chain = nullptr;
    if (node) {
        STransInvertMask(node->surface, &chain);
        node = node->next;
        while (s32(node) > 0) {
            void* extended = nullptr;
            STransCombineMasks(chain, node->surface, node->left, node->top, 3, &extended);
            STransDelete(chain);
            chain = extended;
            node = node->next;
        }
    }

    g_storm_blit_chain = chain;

    if (!chain) {
        return 0;
    }
    STransDuplicate(chain, &g_storm_update_region);

    const s32 cell = s32(g_display.cell_size());
    const s32 rc = STransSetDirtyArrayInfo(g_display.surface_width(), g_display.surface_height(), cell, cell);
    return rc;
}

void colorkey_blit(u8* dest, const u8* src, s32 width, s32 height, s32 src_stride, s32 dest_pitch) {
    for (s32 row = 0; row < height; ++row) {
        const u8* src_row = src + size_t(row) * src_stride;
        u8* dest_row = dest + size_t(row) * dest_pitch;
        for (s32 col = 0; col < width; ++col) {
            if (src_row[col] != 0) {
                dest_row[col] = src_row[col];
            }
        }
    }
}

void translucent_fx_blit(u8* dest, const u8* src, s32 width, s32 height, s32 dest_pitch, s32 src_pitch) {
    const bool alt_palette = g_fx_blend_alt_palette != 0; // render_bitmap mode 5
    const u8 shade = alt_palette ? g_combobox_palette : g_current_palette_index;
    if (shade == 7) {
        colorkey_blit(dest, src, width, height, src_pitch, dest_pitch);
        return;
    }

    const u8* blend_row = alt_palette ? graphics::active_alt_translucency_row()
        : graphics::active_translucency_row();
    const u8 blend_index = alt_palette ? g_translucent_index_alt : graphics::translucent_index();
    for (s32 row = 0; row < height; ++row) {
        const u8* source_row = src + size_t(row) * src_pitch;
        u8* dest_row = dest + size_t(row) * dest_pitch;
        for (s32 col = 0; col < width; ++col) {
            const u8 source_index = source_row[col];
            if (source_index == 0) continue; // transparent colorkey
            dest_row[col] = source_index == blend_index
                ? blend_row[dest_row[col]]
                : source_index;
        }
    }
}

} // hooks
