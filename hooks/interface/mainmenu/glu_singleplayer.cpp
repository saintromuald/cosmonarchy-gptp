#include "custom_glues.h"
#include "BW/ui.h"
#include "BW/enumerations.h"
#include "BW/game_runner.h"

#include <minwindef.h>
#include <filesystem>
#include <optional>

namespace {
static const fs::path starcraft_dir = []{
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(0, buffer, MAX_PATH);
    return fs::path{buffer}.parent_path();
}();

static const fs::path tutorials_root = starcraft_dir / "Maps" / "CMBW-Root" / "CMBW-maps" / "campaign" / "Tutorials";

static std::optional<fs::path> tutorial_path(RaceId::Enum race) {
    switch (race) {
        case RaceId::Terran: return tutorials_root / "1. Further Notice.scx";
        case RaceId::Protoss: return tutorials_root / "2. The Initiate.scx";
        case RaceId::Zerg: return tutorials_root / "3. Autophagy.scx";
    }
    return {};
}

static u32 start_tutorial(BinDlg* dialog, RaceId::Enum race) {
    auto& game_runner = GameRunner::instance();
    auto scenario_path = tutorial_path(race);
    if (!scenario_path) return 1;
    if (game_runner.create_singleplayer_game(*scenario_path,
        GameType::Scenario, 1) == GameRunner::Result::Success) {
        return helper::dialog_swish_out(dialog);
    }
    return 1;
}

static u32 dlgproc_tutorial_activate(BinDlg* dialog, DialogEvent* event) {
    switch (static_cast<GluTutorialControls>(last_control_index)) {
        case GluTutorialControls::ZergTutorialButton: {
            return start_tutorial(dialog, RaceId::Zerg);
        } break;
        case GluTutorialControls::TerranTutorialButton: {
            return start_tutorial(dialog, RaceId::Terran);
        } break;
        case GluTutorialControls::ProtossTutorialButton: {
            return start_tutorial(dialog, RaceId::Protoss);
        } break;
    }
    return helper::generic_dialog_interact(dialog, event);
}

static u32 __fastcall dlgproc_glu_tutorials(BinDlg* dialog, DialogEvent* event) {
    if (event->number_order == DialogEventType_Dialog) {
        switch (event->user) {
        case DialogUserType_Create: {
            bindlg_load_background_palette(dialog);
            if (!g_music_playing) {
                helper::dialog_music_fade(MusicId_Title);
            }
        } break;
        case DialogUserType_Activate: {
            return dlgproc_tutorial_activate(dialog, event);
            //return helper::dlgproc_glu_campaign_activate(dialog);
        } break;
        case DialogUserType_InitChildren: {
            bindlg_load_background(dialog);

            std::map<GluTutorialControls, BinDlgProc> interaction_functions{
                {GluTutorialControls::LoadSavedButton, (BinDlgProc)0x004e1560},         // menu_generic_button
                {GluTutorialControls::ProtossTutorialButton, (BinDlgProc)0x004e17e0},   // generic_light_up_button
                {GluTutorialControls::TerranTutorialButton, (BinDlgProc)0x004e17e0},    // generic_light_up_button
                {GluTutorialControls::ZergTutorialButton, (BinDlgProc)0x004e17e0},      // generic_light_up_button
                {GluTutorialControls::CancelButton, (BinDlgProc)0x004e1560},            // menu_generic_button
                {GluTutorialControls::PlayCustomButton, (BinDlgProc)0x004e1560},        // menu_generic_button
                {GluTutorialControls::LoadReplayButton, (BinDlgProc)0x004e1560},        // menu_generic_button
            };
            bindlg_load_interaction_functions(dialog, interaction_functions);

            return helper::generic_dialog_interact(dialog, event);
        } break;
        }
    }
    return helper::generic_dialog_interact(dialog, event);
}
}

void glu_singleplayer() {
    g_ophelia_enabled = false;
    g_multiplayer_mode = false;
    helper::load_campaign_progress();

    auto dialog = load_bin_file("rez\\glututorials.bin").release();
    auto last_control = helper::glu_load_bin_dialog(dialog, dlgproc_glu_tutorials);

    switch (static_cast<GluExCampgnControls>(last_control)) {
    case GluExCampgnControls::LoadSavedButton: {
        g_gl_glues_mode = MenuId::GlueLoad;
    } break;
    case GluExCampgnControls::ProtossCampaignButton: {
        g_gl_glues_mode = MenuId::GlueReadyP;
    } break;
    case GluExCampgnControls::TerranCampaignButton: {
        g_gl_glues_mode = MenuId::GlueReadyT;
    } break;
    case GluExCampgnControls::ZergCampaignButton: {
        g_gl_glues_mode = MenuId::GlueReadyZ;
    } break;
    case GluExCampgnControls::CancelButton: {
        g_gl_glues_mode = MenuId::GlueLogin;
    } break;
    case GluExCampgnControls::PlayCustomButton: {
        g_gl_glues_mode = MenuId::GlueCreate;
    } break;
    case GluExCampgnControls::LoadReplayButton: {
        g_gl_glues_mode = MenuId::GlueCreateMultiplayer;
    } break;
    default: {
        g_gl_glues_mode = MenuId::GlueMainMenu;
    } break;
    }
    change_menu();
}