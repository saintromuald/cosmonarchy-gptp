#include "custom_glues.h"
#include "BW/enumerations.h"
#include "BW/ui.h"
#include "BW/bwhelper.h"
#include "BW/bwdata.h"
#include "hooks/snp.h"

void glu_provider_select(bool from_crownlink) {
    g_gl_glues_mode = MenuId::GlueSelConn;

    g_connect_bin = load_bin_file_scaled("rez\\gluConn.bin", ui::MenuBinScale).release();

    auto result = helper::glu_load_bin_dialog(g_connect_bin, (bw::DialogInteraction*)0x004bae20);
    bool force_refresh = false;
    switch (static_cast<GluConnControls>(result)) {
    case GluConnControls::OkButton: {
        switch (g_selected_snp) {
        case 'BNET': {
            helper::stop_bw_music();
            if (helper::initialize_bnet('BNET')) {
                g_gl_glues_mode = MenuId::GlueBattlenet;
            } else {
                g_gl_glues_mode = MenuId::GlueSelConn;
            }
        } break;
        case 'CNLK':
        case 'CNDB': {
            g_gl_glues_mode = MenuId::GlueJoinCrownlink;
            force_refresh = true;
        } break;
        case 'MODM':
        case 'MDMX': {
            g_gl_glues_mode = MenuId::GlueModem;
        } break;
        case 'SCBL': {
            g_gl_glues_mode = MenuId::GlueDirect;
        } break;
        default:
            g_gl_glues_mode = MenuId::GlueGameSel;
        } break;
    } break;
    case GluConnControls::CancelButton: {
        auto& provider_manager = snp::ProviderManager::instance();
        if (from_crownlink && provider_manager.initialize_provider('CNLK')) {
            g_gl_glues_mode = MenuId::GlueJoinCrownlink;
            force_refresh = true;
        } else {
            g_gl_glues_mode = MenuId::GlueMainMenu;
        }
    } break;
    }

    change_menu(force_refresh);
}