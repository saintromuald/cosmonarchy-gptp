#include "custom_glues.h"
#include "BW/ui.h"

void glu_join() {
    if (!g_music_playing) helper::dialog_music_fade(MusicId::MusicId_Title);
    g_join_bin = load_file_smemalloc<BinDlg>("rez\\gluJoin.bin");
    g_join_bin->flags |= BinDlgFlags::Active;
    helper::fixup_and_init_dialog_data(g_join_bin, helper::alloc_background_image, __FILE__, __LINE__);

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