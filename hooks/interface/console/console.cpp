#include "console.h"

#include "BW/api.h"
#include "BW/structures.h"
#include "BW/bwdata.h"
#include "hooks/interface/uifeatures/ui_features.h"
#include "storm/storm.h"
#include "storm/sfile.h"

#include <numeric>

namespace hooks {
void load_racial_sounds(bool enable) { // 0048fb40
    g_sound_cooldown_1 = 0;
    g_sound_cooldown_2 = 0;
    *g_place_building_parent_unit = 0;
    g_selection_sound_counter = 0;
    *notify_cooldown = 0;
    g_use_quiet_sounds = 0;

    if (!g_racial_sounds_loaded) {
        constexpr int initial_count = 113;
        constexpr int zerg_count = 213;
        constexpr int zerg_base = 755;
        constexpr int terran_count = 299;
        constexpr int terran_base = 174;
        constexpr int protoss_count = 282;
        constexpr int protoss_base = 473;

        std::iota(g_zerg_sounds.begin(), g_zerg_sounds.begin() + initial_count, 1);
        std::iota(g_terran_sounds.begin(), g_terran_sounds.begin() + initial_count, 1);
        std::iota(g_protoss_sounds.begin(), g_protoss_sounds.begin() + initial_count, 1);

        u32 next_index = initial_count;

        std::iota(g_zerg_sounds.begin() + next_index, g_zerg_sounds.begin() + next_index + zerg_count, zerg_base);
        g_zerg_sounds[next_index + zerg_count] = 0;

        std::iota(g_terran_sounds.begin() + next_index, g_terran_sounds.begin() + next_index + terran_count, terran_base);
        g_terran_sounds[next_index + terran_count] = 0;

        std::iota(g_protoss_sounds.begin() + next_index, g_protoss_sounds.begin() + next_index + protoss_count, protoss_base);
        g_protoss_sounds[next_index + protoss_count] = 0;

        g_racial_sounds_loaded = true;
    }

    if (enable) {
        // TODO: Add Askosi
        u8 sound_index = (g_console_index > 2) ? 1 : g_console_index;
        helper::enable_sound_list(g_racial_sound_arrays[sound_index]);
    } else {
        helper::enable_sound_list(0);
    }
}

void load_race_grp_and_sounds() {
    load_racial_sounds(true);

    switch (g_console_index) {
    case RaceId::Zerg: {
        helper::dialog_grp_construct("dlgs\\zerg.grp", __FILE__, __LINE__);
        g_current_music_id = 1;
    } break;
    case RaceId::Protoss: {
        helper::dialog_grp_construct("dlgs\\protoss.grp", __FILE__, __LINE__);
        g_current_music_id = 7;
    } break;
    default: // Temporary default for Askosi etc
    case RaceId::Terran: {
        helper::dialog_grp_construct("dlgs\\terran.grp", __FILE__, __LINE__);
        g_current_music_id = 4;
    } break;
    }

    u32 music_offset = 0;

    if (g_campaign_index && g_console_index <= 2) {
        const auto& mission_table = (g_campaign_index < 0x20)
            ? g_campaign_first_mission
            : g_campaign_first_mission_bw;

        auto first_mission = mission_table[g_console_index];
        music_offset = (g_campaign_index - first_mission) % 3;
    } else {
        if (g_race_ui_music_offset == -1) {
            auto random_value = helper::rand_func();
            g_race_ui_music_offset = random_value % 3;
        }
    }
    g_race_ui_music_offset = (g_race_ui_music_offset + 1) % 3;
    if (g_race_ui_music_offset == 0) {
        return;
    }

    music_offset = g_race_ui_music_offset;
    g_current_music_id += music_offset;
}

static void console_find_vertical_bounds(Bitmap* console) {
    if (!console || !console->buffer) return;

    auto buffer = console->buffer;
    auto width = console->width;
    auto height = console->height;

    g_console_first_content_row = 0;
    for (auto row = 0; row < height; row++) {
        auto row_start = buffer + row * width;
        auto row_end = row_start + width;

        if (std::any_of(row_start, row_end, [](char c) { return c != '\0'; })) {
            break;
        }
        g_console_first_content_row++;
    }

    g_console_last_content_row = g_console_first_content_row;
    for (auto row = g_console_first_content_row; row < height; row++) {
        auto row_start = buffer + row * width;
        auto row_end = row_start + width;

        if (std::any_of(row_start, row_end, [](char c) { return c != '\0'; })) {
            g_console_last_content_row = row + 1;
        }
    }
}

static SurfaceContext* console_create_surface(u32 offset_x, u32 offset_y, Bitmap* bitmap) {
    void* surface_handle;
    SurfaceContext* surface_out{};

    // This returns a Bool but it's not used in the original call
    // TODO: Do we need error handling?
    STransCreateE(bitmap->buffer, bitmap->width, bitmap->height, 8, 0, 0x1000000, &surface_handle);

    auto* surface_node = static_cast<SurfaceContext*>(SMemAlloc(sizeof(SurfaceContext), "", 0, '\b'));
    if (surface_node) {
        surface_node->next = nullptr;
        surface_node->prev = nullptr;
        surface_out = surface_node;
    } else {
        surface_node = reinterpret_cast<SurfaceContext*>(&g_surface_list_head);
    }

    if (surface_node->next) {
        surface_node->next->prev = surface_node->prev;
        if (surface_node->prev) {
            surface_node->prev->next = surface_node->next;
        }
        surface_node->next = nullptr;
        surface_node->prev = nullptr;
    }

    surface_node->next = g_surface_list_head;
    if (g_surface_list_head) {
        surface_node->prev = g_surface_list_head->prev;
        g_surface_list_head->prev = surface_out;
    }
    g_surface_list_head = surface_node;

    if (surface_out) {  // Only store if we have a valid allocated node
        surface_out->surface = surface_handle;
        surface_out->left = offset_x;
        surface_out->top = offset_y;
        surface_out->right = offset_x + bitmap->width;
        surface_out->bottom = offset_y + bitmap->height;
    }

    helper::console_update_surface_masks();
    return surface_out;
}

void load_console_image() {
    auto race_char = g_is_in_replay ? 'n' : get_race_initial((RaceId::Enum)g_console_index).value_or('t');
    auto filename = std::format("game\\{}console.pcx", race_char);

    // Load bitmap from file
    u8* buffer_out;
    u32 width_out = 0;
    u32 height_out = 0;

    b32 success = SBmpAllocLoadImage(filename.c_str(), 0, &buffer_out, &width_out, &height_out, 0, 0, helper::generic_malloc_wrapper);

    if (!success) {
        auto error = SErrGetLastError();
        helper::fatal_file_not_found(error, filename.c_str());
    }

    g_console_image.width = static_cast<short>(width_out);
    g_console_image.height = static_cast<short>(height_out);
    g_console_image.buffer = buffer_out;

    console_find_vertical_bounds(&g_console_image);
    g_console_surface = console_create_surface(0, 0, &g_console_image);
}

static void load_progressbar(BinDlg* dialog, BinDlgProc update_function, Bitmap& empty, Bitmap& full, RaceId::Enum race) {
    dialog->fxn_update = update_function;
    dialog->graphic = 0;
    auto initial = get_race_initial(race).value_or('t');
    if (!empty.buffer) {
        auto filename = std::format("game\\{}pbrempt.pcx", initial);
        load_bitmap(filename.c_str(), empty, nullptr);
    }
    if (!full.buffer) {
        auto filename = std::format("game\\{}pbrfull.pcx", initial);
        load_bitmap(filename.c_str(), full, nullptr);
    }
}

void load_command_buttons() { // 00459b90
    g_active_portrait_buttonset_id = UnitId::Buttons_Blank;
    g_buttonset_special_buttonset_id = UnitId::Buttons_Blank;
    g_current_buttonset_id = UnitId::Buttons_Blank;
    g_highlighted_icon = -1;

    auto initial = get_race_initial((RaceId::Enum)g_console_index).value_or('t');
    auto filename = std::format("unit\\cmdbtns\\{}cmdbtns.grp", initial);
    g_cmdbtns_grp = sfile::load_file(filename.c_str()).unsafe_cast<GrpHead>().release();
    g_cmdicons_grp = sfile::load_file("unit\\cmdbtns\\cmdicons.grp").unsafe_cast<GrpHead>().release();

    if (!SBmpLoadImage("unit\\cmdbtns\\ticon.pcx", 0, (u8*)&g_new_cmdbtn_palette, sizeof(g_new_cmdbtn_palette), 0, 0, 0)) {
        helper::fatal_file_not_found(SErrGetLastError(), "unit\\cmdbtns\\ticon.pcx");
    };
    if (g_is_in_replay) {
        initial = 'n';
    }

    filename = std::format("rez\\statbtn{}.bin", initial);
    g_buttonset_dialog = load_bin_file(filename.c_str()).release();
    initialize_dialog(g_buttonset_dialog, helper::statbtn_dlgproc);
}

void load_textbox() { //004f37c0
    auto initial = get_race_initial((RaceId::Enum)g_console_index).value_or('t');
    auto filename = std::format("rez\\{}textbox.bin", initial);
    g_textbox_dialog = load_bin_file(filename.c_str()).release();
    initialize_dialog(g_textbox_dialog, helper::textbox_dlgproc);
}

void load_statlb() { // 004bed70
    bool is_standard_melee = (g_game_data.victory_condition == VictoryCondition::Default &&
        g_game_data.starting_units == StartingUnits::MapUnits &&
        g_game_data.tournament_mode_enabled == 0);

    bool should_show_statlb = !g_multiplayer_mode ||
        g_game_data.victory_condition == VictoryCondition::Greed ||
        g_game_data.victory_condition == VictoryCondition::Slaughter ||
        is_standard_melee ||
        g_is_in_replay;

    if (!should_show_statlb) {
        return;
    }

    g_statlb_unk = 0;
    g_show_statlb = true;

    g_statlb_dialog = load_bin_file("rez\\statlb.bin").release();
    initialize_dialog(g_statlb_dialog, helper::statlb_dlgproc);
}

void load_wireframe_palette() { // 00456a90
    g_wirefram_grp = sfile::load_file("unit\\wirefram\\wirefram.grp").unsafe_cast<GrpHead>().release();
    if (!SBmpLoadImage("game\\twire.pcx", 0, &g_wireframe_colors[0], sizeof(g_wireframe_colors), 0, 0, 0)) {
        helper::fatal_file_not_found(SErrGetLastError(), "game\\twire.pcx");
    };
}

void load_statf10() { // 004f5300
    g_stat_f10_dialog = load_bin_file("rez\\stat_f10.bin").release();
    initialize_dialog(g_stat_f10_dialog, helper::stat_f10_dlgproc);
}

void load_console() { // 004c3bb0
    load_console_image();
    if (!SBmpLoadImage("game\\thpbar.pcx", 0, &g_thpbar_colors[0], sizeof(g_thpbar_colors), 0, 0, 0)) {
        helper::fatal_file_not_found(SErrGetLastError(), "game\\thpbar.pcx");
    };

    load_statfluf_mouse_regions();
    helper::load_statport();
    load_statlb();
    load_statres();
    helper::load_stat_data();
    load_wireframe_palette();
    load_command_buttons();
    load_statf10();
    load_textbox();
    helper::load_minimap();

    if (g_console_image.buffer) {
        SMemFree(g_console_image.buffer);
        g_console_image.buffer = 0;
    }

    *g_client_selection_group = {};
    g_client_selection_count = 0;
    *g_active_portrait_unit = 0;
    g_control_under_mouse = 0;
    g_control_under_mouse_value = 0;
    g_force_button_refresh = 1;
    g_portrait_redraw = 1;
    g_status_screen_redraw = 1;
}

static std::pair<MousePixelmaskRegion*, size_t> get_mouse_regions() {
    if (g_is_in_replay)
        return {g_statfluf_mouse_regions_replay.data(), g_statfluf_mouse_regions_replay.size()};
    switch (g_console_index) {
    case RaceId::Zerg:
        return {g_statfluf_mouse_regions_z.data(), g_statfluf_mouse_regions_z.size()};
    case RaceId::Terran:
        return {g_statfluf_mouse_regions_t.data(), g_statfluf_mouse_regions_t.size()};
    case RaceId::Protoss:
        return {g_statfluf_mouse_regions_p.data(), g_statfluf_mouse_regions_p.size()};
    }
    return {g_statfluf_mouse_regions_t.data(), g_statfluf_mouse_regions_t.size()};
}

// Each console has an array of BoundedBinDlg that box regions of the console outside the main 3 boxes
// Custom consoles should provide an array of these to load here
void load_statfluf_mouse_regions() {
    auto [mouse_regions, region_count] = get_mouse_regions();

    for (size_t i = 0; i < region_count; ++i) {
        auto& region = mouse_regions[i];

        auto dialog = load_bin_file("rez\\statfluf.bin").release();
        region.dialog = dialog;

        dialog->bounds.x = region.x;
        dialog->bounds.y = region.y;
        dialog->bounds.w = region.w - 1;
        dialog->bounds.h = region.h - 1;

        dialog->DialogBoxDC_0x36.bitmap.width = region.w;
        dialog->DialogBoxDC_0x36.bitmap.height = region.h;

        initialize_dialog(dialog, helper::statfluf_dlgproc);
    }
}

void unload_statfluf_mouse_regions() {
    auto [mouse_regions, region_count] = get_mouse_regions();

    for (size_t i = 0; i < region_count; ++i) {
        auto& region = mouse_regions[i];
        if (region.dialog) {
            helper::destroy_dialog(region.dialog);
        }
    }
}

void __stdcall conover_filename(char(&buffer)[260]) {
    RaceId::Enum race = static_cast<RaceId::Enum>(g_console_index);
    auto race_char = get_race_initial(race).value_or('t');
    std::string filename = std::format("game\\{}conover.pcx", race_char);

    strncpy_s(buffer, filename.c_str(), 259);
    buffer[260] = '\0';
    g_conover_image = {};
}

void set_text_string(BinDlg* dialog) // 004263E0
{
    const char* string = "";
    Unit* active_unit = *g_active_portrait_unit;
    u32 active_id = active_unit->id;
    s32 max_hp_real = units_dat::max_hitpoints[active_id] >> 8;
    if ((max_hp_real || (max_hp_real = (active_unit->hitpoints + 255) >> 8) != 0) && max_hp_real > 9999
    ||   units_dat::max_shield_points[active_id] > 9999
    ||   active_unit->status & UnitStatus::IsHallucination)
    {
        string = 0;
    } else {
        s32 max_hp_raw = helper::get_max_hp(active_unit);
        u8 hp_state = 0;
        if (helper::unit_hp_below_33_percent(*g_active_portrait_unit))
        {
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
}