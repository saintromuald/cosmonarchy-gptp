#include "custom_glues.h"
#include "hooks/snp.h"

void glu_main_switch() {
    switch (g_gl_glues_mode) {
        case (MenuId::GlueMainMenu): {
            g_out_of_game = false;
            glu_main();
        } break;
        case (MenuId::GlueSelConn): {
            glu_provider_select(false);
        } break;
        case (MenuId::GlueChatroom): {
            glu_chat();
        } break;
        case (MenuId::GlueBattlenet): {
            g_is_bnet = true;
            g_glue_background_id = -1;
            helper::glu_bnet();
        } break;
        case (MenuId::GlueLogin): {
            g_is_bnet = false;
            glu_login();
        } break;
        case (MenuId::GlueCampaign): {
            helper::glu_campaign();
        } break;
        case (MenuId::GlueReadyT): {
            helper::glu_ready_terran();
        } break;
        case (MenuId::GlueReadyZ): {
            helper::glu_ready_zerg();
        } break;
        case (MenuId::GlueReadyP): {
            helper::glu_ready_protoss();
        } break;
        case (MenuId::GlueGameSel):
        case (MenuId::GlueDirect): {
            auto& provider_manager = snp::ProviderManager::instance();
            if (provider_manager.is_crownlink()) {
                glu_join_crownlink();
            } else {
                glu_join();
            }
        } break;
        case (MenuId::GlueCreate): {
            helper::glu_create(false);
        } break;
        case (MenuId::GlueCreateMultiplayer): {
            helper::glu_create(true);
        } break;
        case (MenuId::GlueLoad): {
            helper::glu_load();
        } break;
        case (MenuId::GlueScoreZd):
        case (MenuId::GlueScoreZv):
        case (MenuId::GlueScoreTd):
        case (MenuId::GlueScoreTv):
        case (MenuId::GlueScorePd):
        case (MenuId::GlueScorePv): {
            // Temporary for Askosi
            for (auto& player : g_player_table) {
                if (player.race > 2) player.race = RaceId::Terran;
            }
            change_menu(true); // Force Refresh
            helper::glu_scorescreen();
        } break;
        case (MenuId::GlueModem): {
            helper::glu_modem();
        } break;
        case (MenuId::GlueExCampaign): {
            glu_singleplayer();
        } break;
        case (MenuId::GlueGeneric): {
            helper::glu_generic();
        } break;
        case (MenuId::GlueJoinCrownlink): {
            glu_join_crownlink();
        } break;
        case (MenuId::GlueProviderSelectCrownlink): {
            glu_provider_select(true);
        } break;
        case (MenuId::GlueOptions): {
            glu_options();
        } break;
        default: { g_gl_glues_mode = MenuId::GlueMainMenu; } break;
    }
}