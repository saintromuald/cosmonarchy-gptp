#include "blit.h"
#include "../../BW/bwdata.h"
#include "../../graphics/Bitmap.h"
#include "../../systems/display/frame_overlay.h"
#include "../../systems/graphics/translucency.h"
#include "../../storm/storm.h"

#include <algorithm>
#include <cstddef>

namespace {

// Maximum Half-open rects hinted to SDrawUnlockSurface. Past the cap do whole-surface blit instead
constexpr u32 MaxPresentHints = 256;

// True when all four protection bytes are nonzero: the whole dword is protected.
bool protection_dword_full(u32 bytes) {
    return (bytes & (bytes >> 8) & (bytes >> 16) & (bytes >> 24) & 0xFF) != 0;
}

s32 skip_protected(const u8* prot, s32 x, s32 end) {
    while (x + 4 <= end) {
        const u32 bytes = *(const u32*)(prot + x);
        if (protection_dword_full(bytes)) {
            x += 4;
            continue;
        }
        if (!prot[x]) {
            return x;
        }
        ++x;
    }
    while (x < end && prot[x]) {
        ++x;
    }
    return x;
}

s32 skip_unprotected(const u8* prot, s32 x, s32 end) {
    while (x + 4 <= end) {
        const u32 bytes = *(const u32*)(prot + x);
        if (bytes == 0) {
            x += 4;
            continue;
        }
        if (prot[x]) {
            return x;
        }
        ++x;
    }
    while (x < end && !prot[x]) {
        ++x;
    }
    return x;
}

// Coalesces dirty-cell marks into pixel runs copied from the screen buffer to the locked primary
// Runs split around protection pixels, and spans with matching edges on consecutive rows merge into a single copy
struct RunPresenter {
    u8* dest;
    s32 dest_pitch;
    const u8* src;
    s32 src_pitch;
    const u8* protection; // surface-sized, row-major. nonzero pixels are not presented
    s32 width;

    // Pending vertically merged run (half-open pixel rect).
    s32 left = 0, top = 0, right = 0, bottom = 0;
    bool pending = false;

    s32 hints[MaxPresentHints][4] = {};
    u32 hint_count = 0;
    bool hint_overflow = false;

    void close_run() {
        if (!pending) {
            return;
        }
        auto out = dest + size_t(top) * dest_pitch + left;
        auto in = src + size_t(top) * src_pitch + left;
        for (s32 row = top; row < bottom; ++row) {
            std::copy_n(in, right - left, out);
            out += dest_pitch;
            in += src_pitch;
        }
        if (!hint_overflow) {
            if (hint_count < MaxPresentHints) {
                hints[hint_count][0] = left;
                hints[hint_count][1] = top;
                hints[hint_count][2] = right;
                hints[hint_count][3] = bottom;
                ++hint_count;
            } else {
                hint_overflow = true;
                hint_count = 0;
            }
        }
        pending = false;
    }

    void emit(s32 row, s32 span_left, s32 span_right) {
        if (pending && left == span_left && right == span_right && bottom == row) {
            bottom = row + 1;
            return;
        }
        close_run();
        left = span_left;
        top = row;
        right = span_right;
        bottom = row + 1;
        pending = true;
    }

    // One dirty extent of one pixel row, split around protection pixels.
    void span(s32 row, s32 span_left, s32 span_right) {
        if (!protection) {
            emit(row, span_left, span_right);
            return;
        }
        const u8* prot = protection + size_t(row) * width;
        s32 x = span_left;
        while (x < span_right) {
            const s32 run_left = skip_protected(prot, x, span_right);
            if (run_left >= span_right) {
                return;
            }
            x = skip_unprotected(prot, run_left, span_right);
            emit(row, run_left, x);
        }
    }

    void finish() {
        close_run();
    }
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
    // 640px, which 1920-wide rects can't rely on, and a plain copy is cheaper than the ROP dispatch anyway.
    auto out = (u8*)dest + size_t(top) * dest_pitch + left;
    auto in = g_game_screen_buffer->data + size_t(top) * src_pitch + left;
    for (s32 row = 0; row < height; ++row) {
        std::copy_n(in, width, out);
        out += dest_pitch;
        in += src_pitch;
    }

    SDrawUnlockSurface(0, dest, 1, (void*)rect);
}

void present_dirty_cells(const u8* protection) {
    const u16 surface_width = g_display.surface_width();
    const u16 surface_height = g_display.surface_height();

    g_frame_overlay.draw(g_game_screen_buffer->data, surface_width, surface_height, surface_width);

    void* dest = nullptr;
    s32 dest_pitch = 0;
    if (!SDrawLockSurface(0, nullptr, &dest, &dest_pitch, 0)) {
        return;
    }

    RunPresenter presenter{
        .dest = (u8*)dest,
        .dest_pitch = dest_pitch,
        .src = g_game_screen_buffer->data,
        .src_pitch = s32(surface_width),
        .protection = protection,
        .width = s32(surface_width),
    };

    const s32 cell = s32(g_display.cell_size());
    const s32 cols = s32(g_display.grid_cols());
    const s32 rows = s32(g_display.grid_rows());
    const u8* cells = g_dirty_cell_grids.surface.get();
    for (s32 cell_row = 0; cell_row < rows; ++cell_row) {
        const u8* row_cells = cells + cell_row * cols;
        const s32 top = cell_row * cell;
        const s32 bottom = std::min(top + cell, s32(surface_height));
        s32 col = 0;
        while (col < cols) {
            if (!row_cells[col]) {
                ++col;
                continue;
            }
            s32 run_end = col + 1;
            while (run_end < cols && row_cells[run_end]) {
                ++run_end;
            }
            const s32 left = col * cell;
            const s32 right = std::min(run_end * cell, s32(surface_width));
            for (s32 y = top; y < bottom; ++y) {
                presenter.span(y, left, right);
            }
            col = run_end;
        }
    }
    presenter.finish();

    // A zero count (clean frame or hint overflow) makes the GDI fallback blit
    // the whole lock rect, matching the masked present's whole-surface unlock.
    SDrawUnlockSurface(0, dest, presenter.hint_count, presenter.hints);
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
