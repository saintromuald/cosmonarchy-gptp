#include "hud.h"

#include "BW/api.h"
#include "BW/structures.h"
#include "BW/bwdata.h"
#include "BW/ui.h"
#include "hooks/audiovisuals/sfxhooks.h"
#include "hooks/interface/uifeatures/ui_features.h"
#include "storm/storm.h"
#include "storm/sfile.h"

#include <magic_enum/magic_enum.hpp>

namespace hooks {
void load_race_grp_and_sounds() {
    reset_race_audio_state(true);

    // TODO: Replace the music handling -Jesse
    switch (g_hud_index) {
        case RaceId::Zerg: {
            helper::dialog_grp_construct("dlgs\\zerg.grp", __FILE__, __LINE__);
        } break;
        case RaceId::Protoss: {
            helper::dialog_grp_construct("dlgs\\protoss.grp", __FILE__, __LINE__);
        } break;
        default: // Temporary default for Askosi etc TODO: Make this configurable in huds.yml -Jesse
        case RaceId::Terran: {
            helper::dialog_grp_construct("dlgs\\terran.grp", __FILE__, __LINE__);
        } break;
    }
}

void load_progressbar(BinDlg* dialog, BinDlgProc update_function, Bitmap& empty, Bitmap& full) {
    const auto& config = g_hud_manager->current();
    dialog->fxn_update = update_function;
    dialog->graphic = 0;
    if (!empty.buffer) {
        load_bitmap(config.progressbar_empty.c_str(), empty, nullptr);
    }
    if (!full.buffer) {
        load_bitmap(config.progressbar_full.c_str(), full, nullptr);
    }
}

void load_hud() { // 004c3bb0
    if (g_is_in_replay) {
        HudManager::install_replay();
    } else {
        HudManager::install(static_cast<RaceId::Enum>(g_hud_index));
    }

    *g_client_selection_group = {};
    g_client_selection_count = 0;
    *g_active_portrait_unit = nullptr;
}

void unload_hud() { // 004c3780
    // This mirrors the vanilla implementation but not sure if we can do something better now it's extended -Jesse
    if (g_stat_txt_ptr) {
        SMemFree(g_stat_txt_ptr);
    }
    g_stat_txt_ptr = nullptr;

    *g_active_portrait_unit = nullptr;
    g_old_client_selection_group.fill(0);

    g_hud_manager.reset();
}

void __stdcall conover_filename(char(&buffer)[260]) {
    const auto& overlay_path = g_hud_manager->current().hud_overlay;
    safe_copy(buffer, overlay_path);
    buffer[260] = '\0';
    g_conover_image = {};
}

void set_text_string(BinDlg* dialog) { // 004263E0
    const char* string = "";
    Unit* active_unit = *g_active_portrait_unit;
    auto active_id = active_unit->id;
    s32 max_hp_real = units_dat::max_hitpoints[active_id] >> 8;
    if ((max_hp_real || (max_hp_real = (active_unit->hitpoints + 255) >> 8) != 0) && max_hp_real > 9999
        ||   units_dat::max_shield_points[active_id] > 9999
        ||   active_unit->status & UnitStatus::IsHallucination) {
        string = 0;
    } else {
        s32 max_hp_raw = helper::get_max_hp(active_unit);
        u8 hp_state = 0;
        if (helper::unit_hp_below_33_percent(*g_active_portrait_unit)) {
            hp_state = 6;
        } else {
            hp_state = helper::unit_hp_between_33_66_percent(active_unit) != 0 ? 3 : 7;
        }

        static char buffer[32] = {};
        if (bw::unit_has_shields(active_unit)) {
            s32 hp_plus_1 = (active_unit->hitpoints + 255) >> 8;
            u16 last_queue_id = helper::get_last_queue_slot_id(active_unit);
            sprintf_s(buffer, 32, "%d/%d %c%d/%d", active_unit->shields >> 8, units_dat::max_shield_points[last_queue_id], hp_state, hp_plus_1, max_hp_raw);
        } else {
            sprintf_s(buffer, 32, "%c%d/%d", hp_state, (active_unit->hitpoints + 255) >> 8, max_hp_raw);
        }
        string = buffer;
    }
    helper::add_text_to_dialog(dialog, -7, string);
    helper::stats_panel_draw_energy(dialog);
}
} // namespace hooks
