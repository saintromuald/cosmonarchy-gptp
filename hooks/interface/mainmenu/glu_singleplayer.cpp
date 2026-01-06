#include "glu_singleplayer.h"
#include "BW/ui.h"
#include "BW/enumerations.h"

namespace {
static u32 __fastcall dlgproc_glu_campaign_ex(BinDlg* dialog, DialogEvent* event) {
    if (event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
        case (DialogUserType_Create): {
            bindlg_load_background_palette(dialog);
            if (!g_music_playing) {
                helper::dialog_music_fade(MusicId_Title);
            }
        } break;
        case (DialogUserType_Activate): {
            return helper::dlgproc_glu_campaign_activate(dialog);
        } break;
        case (DialogUserType_InitChildren): {
            bindlg_load_background(dialog);

            std::map<GluExCampgnControls, BinDlgProc> interaction_functions{
                {GluExCampgnControls::LoadSavedButton, (BinDlgProc)0x004e1560},         // menu_generic_button
                {GluExCampgnControls::ProtossCampaignButton, (BinDlgProc)0x004e17e0},   // generic_light_up_button
                {GluExCampgnControls::TerranCampaignButton, (BinDlgProc)0x004b4f50},    // bindlg_excampaign
                {GluExCampgnControls::ZergCampaignButton, (BinDlgProc)0x004b4f50},      // bindlg_excampaign
                {GluExCampgnControls::CancelButton, (BinDlgProc)0x004e1560},            // menu_generic_button
                {GluExCampgnControls::PlayCustomButton, (BinDlgProc)0x004e1560},        // menu_generic_button
                {GluExCampgnControls::LoadReplayButton, (BinDlgProc)0x004e1560},        // menu_generic_button
            };
            bindlg_load_interaction_functions(dialog, interaction_functions);

            return helper::generic_dialog_interact(dialog, event);
        } break;
        }
    }
    return helper::generic_dialog_interact(dialog, event);
}
}

namespace hooks {
void glu_campaign_ex() {
    g_ophelia_enabled = false;
    g_multiplayer_mode = false;
    helper::load_campaign_progress();

    BinFile dialog{"rez\\gluexpcmpgn.bin"};

    auto result = helper::glu_load_bin_dialog(dialog.bin_dlg(), dlgproc_glu_campaign_ex);

    switch (static_cast<GluExCampgnControls>(result)) {
    case (GluExCampgnControls::LoadSavedButton): {
        g_gl_glues_mode = MenuId::GlueLoad;
    } break;
    case (GluExCampgnControls::ProtossCampaignButton): {
        g_gl_glues_mode = MenuId::GlueReadyP;
    } break;
    case (GluExCampgnControls::TerranCampaignButton): {
        g_gl_glues_mode = MenuId::GlueReadyT;
    } break;
    case (GluExCampgnControls::ZergCampaignButton): {
        g_gl_glues_mode = MenuId::GlueReadyZ;
    } break;
    case (GluExCampgnControls::CancelButton): {
        g_gl_glues_mode = MenuId::GlueLogin;
    } break;
    case (GluExCampgnControls::PlayCustomButton): {
        g_gl_glues_mode = MenuId::GlueCreate;
    } break;
    case (GluExCampgnControls::LoadReplayButton): {
        g_gl_glues_mode = MenuId::GlueCreateMultiplayer;
    } break;
    default: {
        g_gl_glues_mode = MenuId::GlueMainMenu;
    } break;
    }
    change_menu();
}
} // namespace hooks