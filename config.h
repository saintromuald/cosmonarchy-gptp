#pragma once
#include "config_utils.h"

inline const std::filesystem::path g_config_path = bw::get_starcraft_path() / "cosmonarchy.yml";

struct Config {
    // Random settings
    std::vector<std::string> random_races{"Zerg", "Terran", "Protoss", "Askosi"};

    // Launch settings
    bool disable_wmode_prompt = true;
    bool prefer_wmode = false;
    bool allow_multiple_instances = false;

    // General UI
    bool extended_camera_hotkeys = true;
    bool show_timer = true;
    bool show_apm = true;
    bool always_show_extended_tooltips = false;
    bool suppress_windows_key = false;

    // Selection UI
    UiBool show_rally_lines = true;
    UiBool show_order_lines = false;
    UiBool show_queued_order_lines = false;
    UiBool show_unique_gas_order_line_color = false;
    UiBool show_aura_circles = false;
    UiBool show_ability_ranges = true;
    UiBool always_show_condensed_selection = false;
    UiBool use_alt_for_selection_ui = false;
    
    // Minimap UI
    UiBool show_minimap_cameras = true;
    UiBool can_toggle_minimap_terrain = true;

    // Team UI
    bool show_team_ui = true;
    bool show_team_ui_apm = true;
    bool show_team_ui_names = false;
    bool show_team_ui_self = false;
    bool show_ingame_global_queue = false;

    // Witness and replay UI
    bool show_replay_progress_bar = true;

    // Scrolling
//	bool use_spacebar_for_scrolling = false;
    bool use_pan_scrolling = false;
    f32 scroll_sensitivity = 1.0;

    // Audiovisuals
    std::vector<Advisor> common_advisors{ Advisor::Default };
    std::vector<Advisor> terran_advisors{};
    std::vector<Advisor> protoss_advisors{};
    std::vector<Advisor> zerg_advisors{};
    std::vector<Advisor> askosi_advisors{};
    bool celebrations_enabled = true;
    bool flash_window_lobby_join = true;
    bool play_sound_in_background = false;
    bool play_music_in_background = false;

    // Hotkeys
    bool quick_casting = false;
    bool quick_building = false;
    bool grid_hotkeys = true;
    std::array<std::string, 3> grid_layout{"QWE", "ASD", "ZXC"};
    bool extended_control_groups = true;
    bool default_to_steal_hotkeys = false;
    bool tilde_control_group = true;
    bool flip_attack_and_patrol = true;

    // Player Color
    std::vector<PlayerColor> common_preferred_colors{};
    std::vector<PlayerColor> terran_preferred_colors{};
    std::vector<PlayerColor> protoss_preferred_colors{};
    std::vector<PlayerColor> zerg_preferred_colors{};
    std::vector<PlayerColor> askosi_preferred_colors{};
    bool shuffle_preferred_colors = false;

    // Debug
    bool verbose = false;
    bool log_randoms = false;

    // Map directory
    std::string default_map_directory = "CMBW-Root\\CMBW-maps";

    // Replays
    bool auto_save_replays = true;
    bool sanitize_replay_chat = false;
};

inline Config g_config{};

void load_config();
void save_config();