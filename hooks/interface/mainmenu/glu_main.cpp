#include "custom_glues.h"
#include "BW/enumerations.h"
#include "BW/bwdata.h"
#include "BW/ui.h"
#include "BW/api.h"
#include "BW/bwhelper.h"
#include "hooks/snp.h"
#include "systems/audio/music_system.h"

static void load_game_templates(){ // See: 004dbac5
    auto ums_data = helper::glu_main_load_ums_templates();
    auto destination = reinterpret_cast<uint32_t*>(&g_game_data.got_game_type);

    for (int i = 0; i < 8; i++) {
        destination[i * 2] = ums_data[i];
    }

    SMemFree(ums_data);
}

static u32 __fastcall dlgproc_glu_main(bw::Dialog* dialog, DialogEvent* event) { // 004DB7A0
    if (event->number_order == DialogEventType_Spacebar && event->virtual_key == VK_SPACE) {
        return 1;
    }
    if (event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
            case DialogUserType_Create: {
                helper::glu_main_create(dialog);
                helper::dialog_swish_in(dialog);
                helper::generic_dialog_interact(dialog, event);
                MusicSystem::instance().play("title");
                return 1;
            }
            case DialogUserType_Destroy: {
                auto version_label = get_first_child_by_index(dialog, GluMainControls::VersionLabel);
                if (version_label) SMemFree((void*)version_label->text<const char*>());
                return helper::generic_dialog_interact(dialog, event);
            }
            case DialogUserType_Activate: {
                switch (static_cast<GluMainControls>(last_control_index)) {
                    case GluMainControls::SinglePlayerButton: {
                        if (*g_is_broodwar && helper::select_game_mode(0)) {
                            return helper::dialog_swish_out(dialog);
                        }
                        return 1;
                    } break;
                    case GluMainControls::MultiPlayerButton: {
                        if (*g_is_broodwar && helper::select_game_mode(1)) {
                            return helper::dialog_swish_out(dialog);
                        }
                        return 1;
                    } break;
                    case GluMainControls::EditorButton: {
                        helper::load_staredit_process(dialog);
                        return 1;
                    }
                    default: {
                        return helper::dialog_swish_out(dialog);
                    }
                }
            }
            case DialogUserType_InitChildren: {
                helper::register_menu_functions((BinDlgProc)glu_main_functions, dialog, 0x2Cu, 0);
                return helper::generic_dialog_interact(dialog, event);
            }
            default: {
                return helper::generic_dialog_interact(dialog, event);
            }
        }
    }

    return helper::generic_dialog_interact(dialog, event);
}

void glu_main() {
    g_multiplayer_mode = false;

    auto dialog = load_bin_file_scaled("rez\\gluMain.bin", ui::MenuBinScale);
    auto last_control = helper::glu_load_bin_dialog(dialog.get(), dlgproc_glu_main);

    switch (static_cast<GluMainControls>(last_control)) {
        case GluMainControls::ExitButton: {
            g_sc_main_state = MainState::Exit;
        } break;
        case GluMainControls::SinglePlayerButton: {
            g_multiplayer_mode = false;
            g_gl_glues_mode = MenuId::GlueLogin;
            load_game_templates();
        } break;
        case GluMainControls::MultiPlayerButton: {
            g_multiplayer_mode = true;
            g_gl_glues_mode = MenuId::GlueLogin;
            load_game_templates();
        } break;
        case GluMainControls::OptionsButton: {
            g_gl_glues_mode = MenuId::GlueOptions;
        } break;
    }
    change_menu();
}