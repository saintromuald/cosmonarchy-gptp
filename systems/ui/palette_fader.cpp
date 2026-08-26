#include "palette_fader.h"

#include "BW/bwdata.h"
#include "BW/bwhelper.h"
#include "interpolation.h"
#include "storm/storm.h"

#include <algorithm>

#include <windows.h>

namespace ui {
static constexpr u16 FaderTimerId = 0x30;
static constexpr u32 FaderTickIntervalMs = 10;

static void __fastcall fader_timer_callback(BinDlg*, u16) {
    g_palette_fader.tick();
}

void copy_preserved_indices(Palette& destination, const Palette& source,
                            PalettePreserve mask) {
    if (mask == PalettePreserve::None) return;
    for (u32 i = 0; i < 256; ++i) {
        if (preserves_index(mask, i)) {
            destination.colors[i] = source.colors[i];
        }
    }
}

void PaletteFader::start(BinDlg* dialog, const Palette& target, u32 duration_ms,
    std::move_only_function<void()> on_complete, PalettePreserve preserve) {
    if (m_active) cancel();

    m_dialog = dialog;
    m_timer_id = FaderTimerId;
    m_start = g_display_palette;
    m_target = target;
    m_duration_ms = std::max<u32>(1, duration_ms);
    m_start_tick_ms = GetTickCount();
    m_on_complete = std::move(on_complete);
    m_active = true;
    copy_preserved_indices(m_target, m_start, preserve);

    helper::set_callback_timer(dialog, m_timer_id, FaderTickIntervalMs, fader_timer_callback);
}

void PaletteFader::cancel() {
    if (!m_active) return;
    m_active = false;
    if (m_dialog) {
        helper::clear_callback_timer(m_dialog, m_timer_id);
    }
    m_dialog = nullptr;
    m_on_complete = nullptr;
}

void PaletteFader::tick() {
    if (!m_active) return;

    const u32 elapsed_ms = GetTickCount() - m_start_tick_ms;
    const bool done = elapsed_ms >= m_duration_ms;

    Palette display;
    if (done) {
        display = m_target;
    } else {
        const f32 t = static_cast<f32>(elapsed_ms) / static_cast<f32>(m_duration_ms);
        for (u32 i = 0; i < 256; ++i) {
            display.colors[i].r = static_cast<u8>(utils::lerp<f32>(t, m_start.colors[i].r, m_target.colors[i].r));
            display.colors[i].g = static_cast<u8>(utils::lerp<f32>(t, m_start.colors[i].g, m_target.colors[i].g));
            display.colors[i].b = static_cast<u8>(utils::lerp<f32>(t, m_start.colors[i].b, m_target.colors[i].b));
            display.colors[i].a = 0;
        }
    }

    g_display_palette = display;
    SDrawUpdatePalette(0, 256, &g_display_palette, 1);
    helper::refresh_region();

    if (done) {
        // Grab the callback before cancel clears it
        auto completion = std::move(m_on_complete);
        cancel();
        if (completion) completion();
    }
}
} // namespace ui
