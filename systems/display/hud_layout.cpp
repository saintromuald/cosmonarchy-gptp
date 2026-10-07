#include "hud_layout.h"
#include "display.h"
#include "../../BW/bwdata.h"

namespace hud_layout {

v2s32 anchor_offset(Anchor anchor, Box16 box) {
    const s32 bottom = s32(g_display.surface_height()) - BandHeight;
    const s32 band = s32(g_display.surface_width()) - BandWidth;
    v2s32 offset{0, 0};

    switch (anchor) {
        case Anchor::HudLeft: {
            offset = {0, bottom};
        } break;
        case Anchor::HudRight: {
            offset = {band, bottom};
        } break;
        case Anchor::TopBarRight: {
            offset = {band, 0};
        } break;
        case Anchor::SurfaceCenter: {
            offset = {
                (s32(g_display.surface_width()) - (box.right - box.left + 1)) / 2 - box.left,
                (s32(g_display.surface_height()) - (box.bottom - box.top + 1)) / 2 - box.top,
            };
        } break;
    }
    return offset;
}

void place_dialog(BinDlg* dialog, Anchor anchor) {
    if (!dialog) {
        return;
    }

    const Box16 box = dialog->bounds;
    const v2s32 offset = anchor_offset(anchor, box);

    dialog->bounds.left = s16(box.left + offset.x);
    dialog->bounds.right = s16(box.right + offset.x);
    dialog->bounds.top = s16(box.top + offset.y);
    dialog->bounds.bottom = s16(box.bottom + offset.y);
}

void place_popup_dialog(BinDlg* dialog) {
    if (g_sc_main_state != MainState::Run) {
        return;
    }
    place_dialog(dialog, Anchor::SurfaceCenter);
}

} // hud_layout
