#pragma once

#include "BW/structures.h"
#include "systems/ui/palette_fader.h"

namespace graphics {

// Background share out of 8 (2/8 background, 6/8 image). Matches BW 0x006D05A4 weight 2 table.
inline constexpr u32 TranslucencyWeight = 2;

// Default for themes that don't define a translucent index
inline constexpr u8 TranslucentIndex = 254;

// Palette index that blends: the theme's published tEffect entry, cached at
// 0x0051A18C. The blend maps those pixels toward the same index's palette color
u8 translucent_index();

// Excludes preserved font/widget palette indices from blend matching during menu fades.
inline ui::PalettePreserve g_translucency_preserve = ui::PalettePreserve::None;

// 256-byte LUT mapping background index to blended color index. Rebuilt on palette/tint change.
// Mode 4 (menus) builds from the 0x0051A18C tint; mode 5 (in-game, flag 0x2000000)
// builds from the 0x0051A194 tint - black, giving the uniform darkening.
// Blend outputs never land on index 0 (the colorkey); the in-game row also skips the
// tileset's color-cycling entries, which animate in the live palette (004C99C0).
const u8* active_translucency_row();
const u8* active_alt_translucency_row();

void build_translucency_row(u8* row, const Palette& palette, u32 weight,
    u8 translucent_index, ui::PalettePreserve preserve, bool exclude_cycling);

} // namespace graphics
