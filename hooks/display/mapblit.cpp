#include "mapblit.h"
#include "../../BW/bwdata.h"
#include "../../graphics/Bitmap.h"

// Copies rendered terrain/units from the 672x448 wrapping scratch buffer (004BD6F0)
// to the screen surface (0040C200; called by 0040C253 full blit and 0040C2BD / 004BCDC0 dirty cells)
// using screen buffer pitch rather than hardcoded 640 (0x280).
namespace {
// Copies dword chunks matching engine alignment and wrap offset drift; returns bytes moved.
s32 copy_dwords(u8* dest, const u8* src, s32 bytes) {
    const s32 count = bytes >> 2;
    auto out = (u32*)dest;
    auto in = (const u32*)src;
    for (s32 i = 0; i < count; ++i) {
        out[i] = in[i];
    }
    return count * 4;
}

} // namespace

namespace hooks {
// 0040C200(eax = source offset, edx = rows, esi = source, edi = destination).
// Copies map rows, wrapping source across circular map_size and stepping dest by screen pitch.
void __cdecl blit_map_rows(s32 src_offset, s32 rows, const u8* src, u8* dest) {
    const s32 map_pitch = s32(g_display.map_pitch());
    const s32 map_size = s32(g_display.map_size());
    const s32 width = g_blit_row_width;
    const s32 src_step = map_pitch - width;

    // Step matches g_game_screen_buffer pitch rather than hardcoded 640.
    const s32 dest_step = s32(g_game_screen_buffer->width) - width;

    for (s32 row = 0; row < rows; ++row) {
        s32 remaining = width;

        // Wrap split across scratch buffer end.
        if (src_offset + width >= map_size) {
            if (src_offset < map_size) {
                const s32 overflow = src_offset + width - map_size;
                const s32 copied = copy_dwords(dest, src, width - overflow);
                dest += copied;
                src += copied;
                remaining = overflow;
            }
            src -= map_size;
            src_offset -= map_size;
        }

        const s32 copied = copy_dwords(dest, src, remaining);
        dest += copied;
        src += copied;

        src_offset += map_pitch;
        src += src_step;
        dest += dest_step;
    }
}

// 0040C2BD(ecx = y, edx = x, [ebp+8] = width, [ebp+0xC] = source offset), callee-clean (ret 8).
// Blits dirty cells for 004BCDC0; row count uses g_display.cell_size() clamped to world height.
void __cdecl blit_map_cell_row(s32 y, s32 x, s32 width, s32 src_offset) {
    s32 rows = s32(g_display.cell_size());
    const s32 remaining = s32(g_display.world_height()) - y;
    if (rows > remaining) {
        rows = remaining;
    }
    if (rows <= 0) {
        return;
    }

    g_blit_row_width = width;

    auto dest = g_game_screen_buffer->data + x + y * s32(g_game_screen_buffer->width);
    auto src = g_game_map_data_for_blit + src_offset;
    blit_map_rows(src_offset, rows, src, dest);
}

// 0040C253. Full viewport redraw blit (called from 004BD580).
// Sized from g_display world dimensions and map pitch rather than hardcoded 640x400 / 672 pitch.
void __cdecl blit_map_viewport() {
    const s32 width = s32(g_display.world_width());
    const s32 rows = s32(g_display.world_height());
    const s32 map_size = s32(g_display.map_size());

    g_blit_row_width = width;

    s32 offset = s32(u32(g_move_to_y) * g_display.map_pitch() + u32(g_move_to_x));
    while (offset >= map_size) {
        offset -= map_size;
    }

    blit_map_rows(offset, rows, g_game_map_data_for_blit + offset, g_game_screen_buffer->data);
}

} // hooks
