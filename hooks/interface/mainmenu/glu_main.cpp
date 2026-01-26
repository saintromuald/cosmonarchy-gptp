#include "custom_glues.h"
#include "BW/enumerations.h"
#include "BW/bwdata.h"
#include "BW/ui.h"
#include "BW/api.h"
#include "BW/bwhelper.h"
#include "hooks/snp.h"

static void load_game_templates(){ // See: 004dbac5
    auto ums_data = helper::glu_main_load_ums_templates();
    auto destination = reinterpret_cast<uint32_t*>(&g_game_data.got_game_type);

    for (int i = 0; i < 8; i++) {
        destination[i * 2] = ums_data[i];
    }

    SMemFree(ums_data, __FILE__, __LINE__, 0);
}

void glu_main() {
    g_multiplayer_mode = false;

    safe_copy(g_version_string, std::format("Cosmonarchy build {}", bw::get_commit_hash()));

    BinFile dialog{"rez\\gluMain.bin"};
    auto result = helper::glu_load_bin_dialog(dialog.bin_dlg(), (BinDlgProc)0x004db7a0);

    switch (static_cast<GluMainControls>(result)) {
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
    }
    change_menu();
}