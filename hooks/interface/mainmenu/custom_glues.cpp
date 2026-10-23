#include "custom_glues.h"
#include "hooks/snp.h"
#include "hooks/text/text_input.h"
#include "glu_campaign.h"
#include "glu_options.h"
#include "glu_chat.h"

static void update_window() {
    if (!g_main_window) return;

    static bool attempted = false;
    if (attempted) return;

    TextInput::instance().ensure_installed();

    attempted = true;

    SetWindowTextA(g_main_window, "Cosmonarchy");

    auto icon_path = "icon.ico";

    auto small_cx = GetSystemMetrics(SM_CXSMICON);
    auto small_cy = GetSystemMetrics(SM_CYSMICON);
    auto large_cx = GetSystemMetrics(SM_CXICON);
    auto large_cy = GetSystemMetrics(SM_CYICON);

    HICON small_icon = (HICON)LoadImageA(NULL, icon_path, IMAGE_ICON, small_cx, small_cy, LR_LOADFROMFILE);
    HICON large_icon = (HICON)LoadImageA(NULL, icon_path, IMAGE_ICON, large_cx, large_cy, LR_LOADFROMFILE);

    if (small_icon) {
        SendMessageA(g_main_window, WM_SETICON, ICON_SMALL, (LPARAM)small_icon);
        SetClassLongPtrA(g_main_window, GCLP_HICONSM, (LONG_PTR)small_icon);
    }
    if (large_icon) {
        SendMessageA(g_main_window, WM_SETICON, ICON_BIG, (LPARAM)large_icon);
        SetClassLongPtrA(g_main_window, GCLP_HICON, (LONG_PTR)large_icon);
    }

    safe_copy(g_version_string, std::format("Cosmonarchy build {}", bw::get_commit_hash()));
}

void glu_main_switch() {
    update_window();
    switch (g_gl_glues_mode) {
        case MenuId::GlueMainMenu: {
            g_out_of_game = false;
            glu_main();
        } break;
        case MenuId::GlueSelConn: {
            glu_provider_select(false);
        } break;
        case MenuId::GlueChatroom: {
            glu_chat();
        } break;
        case MenuId::GlueBattlenet: {
            g_is_bnet = true;
            g_glue_background_id = -1;
            helper::glu_bnet();
        } break;
        case MenuId::GlueLogin: {
            g_is_bnet = false;
            glu_login();
        } break;
        case MenuId::GlueCampaign: {
            helper::glu_campaign();
        } break;
        case MenuId::GlueReadyT: {
            helper::glu_ready_terran();
        } break;
        case MenuId::GlueReadyZ: {
            helper::glu_ready_zerg();
        } break;
        case MenuId::GlueReadyP: {
            helper::glu_ready_protoss();
        } break;
        case MenuId::GlueGameSel:
        case MenuId::GlueDirect: {
            auto& provider_manager = snp::ProviderManager::instance();
            if (provider_manager.is_crownlink()) {
                glu_join_crownlink();
            } else {
                glu_join();
            }
        } break;
        case MenuId::GlueCreate: {
            helper::glu_create(false);
        } break;
        case MenuId::GlueCreateMultiplayer: {
            helper::glu_create(true);
        } break;
        case MenuId::GlueLoad: {
            helper::glu_load();
        } break;
        case MenuId::GlueScoreZd:
        case MenuId::GlueScoreZv:
        case MenuId::GlueScoreTd:
        case MenuId::GlueScoreTv:
        case MenuId::GlueScorePd:
        case MenuId::GlueScorePv: {
            // Temporary for Askosi
            for (auto& player : g_player_table) {
                if (player.race > 2) player.race = RaceId::Terran;
            }
            change_menu(true); // Force Refresh
            helper::glu_scorescreen();
        } break;
        case MenuId::GlueModem: {
            helper::glu_modem();
        } break;
        case MenuId::GlueExCampaign: {
            GluCampaignList dialog{};
            dialog.load();
        } break;
        case MenuId::GlueGeneric: {
            helper::glu_generic();
        } break;
        case MenuId::GlueJoinCrownlink: {
            glu_join_crownlink();
        } break;
        case MenuId::GlueProviderSelectCrownlink: {
            glu_provider_select(true);
        } break;
        case MenuId::GlueOptions: {
            GluOptions dialog{};
            dialog.load();
        } break;
        case MenuId::GluCampaignList: {
            GluCampaignList dialog{};
            dialog.load();
        } break;
        case MenuId::GluCampaignDetails: {
            GluCampaignDetails details{};
            details.load();
        } break;
        default: { g_gl_glues_mode = MenuId::GlueMainMenu; } break;
    }
}

namespace hooks {

void chat_set_slot_label(bw::Dialog* label, const char* text) {
    label->graphic |= 0x0100; // Chat-visible marker. Show handlers suppress the label while clear

    if (!g_chat_unkown_3) {
        helper::show_dialog(label->to_bindlg());
    }
    if (!g_chat_slots_locked) {
        helper::enable_dialog(label);
    }

    const s32 factor = s32(ui_scale::Factor);
    constexpr s32 ChatSlotLabelWidth = 184;
    constexpr s32 ChatSlotLabelHeight = 18;
    label->bounds.set_ltrb(label->bounds.left(), label->bounds.top(),
        s16(label->bounds.left() + ChatSlotLabelWidth * factor), s16(label->bounds.top() + ChatSlotLabelHeight * factor));

    auto flags = label->flags;
    if (g_turn_count >= g_chat_team_switch_deadline) {
        flags &= ~(DialogFlags::PropagateUpdate | DialogFlags::UpdateNeeded);
    }
    flags |= (DialogFlags::UseAlternateStyle | DialogFlags::RespondsToEvents);
    label->flags = flags;

    label->interaction_function = reinterpret_cast<bw::DialogInteraction*>(0x004504A0); // chat_click_on_team_name
    label->draw_function = reinterpret_cast<bw::DialogDraw*>(0x00450330); // chat slot label draw
    *label->text<const char*>() = text;

    if (!(flags & DialogFlags::UpdateNeeded)) {
        label->flags = flags | DialogFlags::UpdateNeeded;
        helper::update_dialog(label);
    }
    // update_dialog consumes UpdateNeeded; force the redraw through once more the way vanilla does
    if (!(label->flags & DialogFlags::UpdateNeeded)) {
        label->flags |= DialogFlags::UpdateNeeded;
        helper::update_dialog(label);
    }
}

// For vanilla savegame (save replay), ok, okcancel modals
void scale_glue_modal(bw::Dialog* dialog) {
    ui::scale_bin(*dialog);

    auto& bounds = dialog->bounds;
    // Pre-init modals are actually stored xywh not ltrb.
    // We're using set_ltrb because it doesn't translate the coordinates, but passing xywh because that's what the modal actually uses
    bounds.set_ltrb(
        bounds.left() + ui::MenuBinScale.offset.x,
        bounds.top() + ui::MenuBinScale.offset.y,
        bounds.right(),
        bounds.bottom()
    );
}

} // hooks
