#pragma once

#include "BW/structures.h"
#include "BW/structures/ui_structures.h"

#include <functional>

namespace ui {
enum class PalettePreserve : u32 {
    None         = 0,
    FontShade    = 1,  // indices 1..25
    WidgetColors = 2,  // indices 26..53
};

// True if `index` falls inside any range enabled by `mask`.
constexpr bool preserves_index(PalettePreserve mask, u32 index) {
    if ((mask & PalettePreserve::FontShade)    != PalettePreserve::None
        && index >= 1  && index <= 25)  return true;
    if ((mask & PalettePreserve::WidgetColors) != PalettePreserve::None
        && index >= 26 && index <= 53)  return true;
    return false;
}

void copy_preserved_indices(Palette& destination, const Palette& source, PalettePreserve mask);

class PaletteFader {
public:
    // Start new fade (cancelling any in progress).
    // on_complete is called when the fade finishes successfully.
    void start(BinDlg* dialog, const Palette& target, u32 duration_ms,
               std::move_only_function<void()> on_complete = nullptr,
               PalettePreserve preserve = PalettePreserve::None);

    void cancel();
    bool is_active() const { return m_active; }
    void tick();

private:
    Palette m_start{};
    Palette m_target{};
    u32 m_start_tick_ms{};
    u32 m_duration_ms{};
    BinDlg* m_dialog{};
    u16 m_timer_id{};
    bool m_active{};
    std::move_only_function<void()> m_on_complete;
};

inline PaletteFader g_palette_fader;
} // namespace ui
