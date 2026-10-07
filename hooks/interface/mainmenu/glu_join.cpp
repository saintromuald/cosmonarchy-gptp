#include "custom_glues.h"
#include "BW/ui.h"
#include "systems/audio/music_system.h"

void glu_join() {
    MusicSystem::instance().play("title");

    g_join_bin = load_bin_file_scaled("rez\\glujoin.bin", ui::MenuBinScale).release();

    auto result = helper::glu_load_bin_dialog(g_join_bin, helper::glujoin_dlgproc);
    switch (static_cast<GluJoinControls>(result)) {
        case GluJoinControls::OK: {
            g_gl_glues_mode = MenuId::GlueChatroom;
        } break;
        case GluJoinControls::Cancel: {
            g_gl_glues_mode = MenuId::GlueSelConn;
        } break;
        case GluJoinControls::Create: {
            g_gl_glues_mode = MenuId::GlueCreate;
        } break;
        default: {
            g_gl_glues_mode = MenuId::GlueMainMenu;
        } break;
    }
    change_menu();
}