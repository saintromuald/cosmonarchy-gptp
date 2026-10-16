#include "popup_dialog.h"

#include "../../BW/bwhelper.h"
#include "../../BW/ui.h"
#include "../../systems/display/display.h"
#include "../../systems/display/hud_layout.h"
#include "../interface/uifeatures/ui_features.h"

namespace hooks {

void scale_and_initialize_popup_dialog(bw::Dialog* dialog, bw::DialogInteraction* dialogproc) {
    end_middle_drag();

    if (dialog) {
        ui::scale_bin(*dialog);

        // Root bounds are x/y/w/h until 00419D20 converts them, so hand
        // place_popup_dialog an l/t/r/b box and convert back on return.
        auto& bounds = dialog->bounds;
        bounds.set_ltrb(bounds.left(), bounds.top(), bounds.right() + bounds.left(), bounds.bottom() + bounds.top());
        hud_layout::place_popup_dialog(dialog->to_bindlg());
        bounds.set_ltrb(bounds.left(), bounds.top(), bounds.right() - bounds.left(), bounds.bottom() - bounds.top());
    }
    helper::initialize_dialog(dialog, dialogproc);
}

void __fastcall dialog_center_on_screen(bw::Dialog* dialog) {
    if (g_sc_main_state != MainState::Glues) {
        return;
    }
    auto& bounds = dialog->bounds;
    const s16 width = bounds.right();
    const s16 height = bounds.bottom();
    // Pre-init modals are actually stored xywh not ltrb.
    // We're using set_ltrb because it doesn't translate the coordinates, but passing xywh because that's what the modal actually uses
    bounds.set_ltrb(
        s16((display::SurfaceWidth - width) / 2),
        s16((display::SurfaceHeight - height) / 2),
        width,
        height
    );
}

} // hooks
