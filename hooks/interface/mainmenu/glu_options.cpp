#include "custom_glues.h"
#include "BW/ui.h"
#include "BW/enumerations.h"

static u32 __fastcall dlgproc_glu_options(BinDlg* dialog, DialogEvent* event) {
    if (event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
        case (DialogUserType_Create): {
            bindlg_load_background_palette(dialog);
            return helper::generic_dialog_interact(dialog, event);
        } break;
        case (DialogUserType_Activate): {
        } break;
        case (DialogUserType_Destroy): {
        } break;
        case (DialogUserType_InitChildren): {
            bindlg_load_background(dialog);
        } break;
        }
    }
    return helper::generic_dialog_interact(dialog, event);
}

void glu_options() {
    BinFile dialog{"rez\\gluoptions.bin"};
    auto result = helper::glu_load_bin_dialog(dialog.bin_dlg(), dlgproc_glu_options);
    g_gl_glues_mode = MenuId::GlueMainMenu;
    helper::change_menu();
}