#pragma once
#include "../../types.h"
#include "../../BW/ui_scale.h"
#include "../../BW/structures/common.h"
#include "../../BW/structures/ui_structures.h"

namespace hud_layout {

enum class Anchor : u8 { HudLeft, HudRight, TopBarRight, SurfaceCenter };

// 640x480 HUD coordinate space the .bin dialogs are authored against, scaled to
// the 2x presentation size. Screen placement offsets derive from it.
constexpr u16 BandWidth = 640 * ui_scale::Factor;
constexpr u16 BandHeight = 480 * ui_scale::Factor;

// Screen-space offset applied to an anchored dialog. The panel anchors ignore
// the box; SurfaceCenter centers the box on the surface.
v2s32 anchor_offset(Anchor anchor, Box16 box = {});

// Places a root dialog at its anchor. Child controls draw and hit-test relative
// to the root (0041C080 / 00418340), so only roots move.
void place_dialog(BinDlg* dialog, Anchor anchor);

// Centers in-game popups (F10, objectives, options) on screen; front-end glue
// screens remain at (0,0).
void place_popup_dialog(BinDlg* dialog);

} // hud_layout
