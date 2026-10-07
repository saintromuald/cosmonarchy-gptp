#include "widget_dialog_init.h"

#include "../../BW/bwhelper.h"
#include "../../BW/ui.h"
#include "../../systems/ui/widgets.h"
#include "../interface/uifeatures/ui_features.h"

namespace hooks {

void initialize_glu_dialog(bw::Dialog* dialog, bw::DialogInteraction* dialogproc) {
    end_middle_drag();

    if (ui::is_widget_dialog(dialog)) {
        initialize_widget_dialog(dialog);
        return;
    }
    helper::initialize_dialog(dialog, dialogproc);
}

} // hooks