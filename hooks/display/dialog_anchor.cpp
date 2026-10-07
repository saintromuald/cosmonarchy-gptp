#include "dialog_anchor.h"

#include "../../systems/display/display.h"
#include "../../systems/display/hud_layout.h"
#include "../interface/uifeatures/ui_features.h"
#include "../../BW/bwhelper.h"

namespace hooks {

void initialize_popup_dialog(BinDlg* dialog, BinDlgProc dialogproc) {
    end_middle_drag();

    if (dialog && g_display.is_widescreen()) {
        dialog->bounds.right += dialog->bounds.left;
        dialog->bounds.bottom += dialog->bounds.top;
        hud_layout::place_popup_dialog(dialog);
        dialog->bounds.right -= dialog->bounds.left;
        dialog->bounds.bottom -= dialog->bounds.top;
    }
    helper::initialize_dialog(dialog, dialogproc);
}

} // hooks
