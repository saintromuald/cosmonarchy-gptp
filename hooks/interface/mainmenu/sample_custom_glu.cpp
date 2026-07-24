#include "custom_glues.h"
#include "BW/enumerations.h"
#include "BW/ui.h"
#include "BW/bwhelper.h"
#include "BW/bwdata.h"

// Minimal example of creating a custom menu using the new dynamic theme

static u32 __fastcall dlgproc_dyn_sample(BinDlg* dialog, DialogEvent* event) {
    if (event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
            case DialogUserType_InitChildren: {
                // InitChildren is called first
                bindlg_load_background_pcx(dialog, "glue\\dynamic\\gears.pcx", true);
                static const std::map<GluJoinControls, BinDlgProc> interaction_functions{
                    // Provide custom dialogprocs for any controls that need them. Add a new controls enum
                };
                bindlg_load_interaction_functions(dialog, interaction_functions);
            } break;
            case DialogUserType_Create: {
                bindlg_load_background_palette(dialog);
                // Any tasks on first load after children are initialized
                // E.g. populating listboxes or updating selections
            } break;
            case DialogUserType_Activate: {
                switch (static_cast<GluJoinControls>(last_control_index)) {
                    // Override specific controls based on last control selected
                }
            } break;
            case DialogUserType_Destroy: {
            } break;
        }
    }
    return helper::generic_dialog_interact(dialog, event);
}

void glu_dynamic_test() {
    BinFile dialog{"rez\\dynamic_sample.bin"};

    auto result = helper::glu_load_bin_dialog(dialog.bin_dlg(), dlgproc_dyn_sample);
    
    // Result contains the index of the last button pressed, use it to control next menu
    if (static_cast<GluJoinControls>(result) == GluJoinControls::OK) {
        g_gl_glues_mode = MenuId::GlueOptions;
    } else {
        g_gl_glues_mode = MenuId::GlueMainMenu;
    }
    
    change_menu();
}