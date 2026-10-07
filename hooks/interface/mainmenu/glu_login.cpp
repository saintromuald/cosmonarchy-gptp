#include "custom_glues.h"
#include "BW/enumerations.h"
#include "BW/ui.h"
#include "BW/bwhelper.h"
#include "BW/bwdata.h"
#include "hooks/snp.h"
#include "storm/sfile.h"

void glu_login() {
    auto multiplayer_mode = g_multiplayer_mode;

    // Force multiplayer mode to unify single/multi characters
    g_multiplayer_mode = true;
    g_login_bin = load_bin_file_scaled("rez\\gluLogin.bin", ui::MenuBinScale).release();
    
    auto last_control = helper::glu_load_bin_dialog(g_login_bin, (bw::DialogInteraction*)0x004edaa0);
    auto& provider_manager = snp::ProviderManager::instance();
    bool force_refresh = false;
    g_multiplayer_mode = multiplayer_mode;

    switch (static_cast<GluLoginControls>(last_control)) {
    case GluLoginControls::OkButton: {
        if (g_multiplayer_mode) {
            if (provider_manager.initialize_provider('CNLK')) {
                g_gl_glues_mode = MenuId::GlueJoinCrownlink;
                force_refresh = true;
            } else {
                g_gl_glues_mode = MenuId::GlueSelConn;
            }
        } else {
            g_gl_glues_mode = MenuId::GlueExCampaign;
        }
    } break;
    case GluLoginControls::CancelButton: {
        helper::glu_login_leave_game();
        g_gl_glues_mode = MenuId::GlueMainMenu;
    } break;
    default: {
        g_gl_glues_mode = MenuId::GlueMainMenu;
    } break;
    }
    change_menu(force_refresh);
}