#include "config.h"
#include <fstream>
#include <format>
#include "logger.h"
#include "player_color.h"

#define NOMINMAX
#include <Windows.h>

#include <algorithm>
#include <ranges>
#include <format>

// Have to use std::vformat and no `const auto&...`, because of some obscure VS issue that makes it not compile for Pr0nogo.
// It's sad, because it will make std::string allocate a copy, but it is what it is, we only do it once on a fail path anyways. -Veeq7
static void config_error(const std::string_view message, auto&&... args) {
    MessageBoxA(0, std::vformat(message, std::make_format_args(args...)).c_str(), "Invalid cosmonarchy.yml!", MB_OK | MB_ICONERROR);
    exit(-1);
}

template <typename T>
static void load_field(const YAML::Node& node, const std::string& key, T& variable) {
    if (node[key]) {
        try {
            variable = node[key].as<T>();
        } catch (YAML::Exception exception) {
            config_error("Invalid value for variable \"{}\"! Please correct your cosmonarchy.yml.\n\n{}", key, exception.what());
        }
    }
}

template <typename T>
static void write_field(YAML::Emitter& emitter, const std::string& key, const T& value) {
    emitter << YAML::Key << key << YAML::Value << value;
}

static void validate_config() {
    if (g_config.extended_control_groups) {
        if (!g_config.grid_hotkeys) {
            config_error("Extended control groups must be used together with grid hotkeys!");
        }
        for (u32 i = 0; i < 3; i++) {
            for (u32 j = 0; j < 3; j++) {
                char& hotkey = g_config.grid_layout[i][j];
                if (hotkey < 'A' || hotkey > 'Z') {
                    if (hotkey >= 'a' && hotkey <= 'z') {
                        hotkey = toupper(hotkey);
                    } else {
                        config_error("Grid hotkey '{}' is not valid! Please correct your cosmonarchy.yml to use hotkeys from the A-Z range.", hotkey);
                    }
                }
                for (u32 k = 0; k < 6; k++) {
                    // Note: We don't need to validate against tilde, since not only it's not included in A-Z range, neither '`' nor '~' maps to the correct VK code anyways.
                    static const char* invalid_hotkeys = "RFVTGB";
                    char invalid_ch = invalid_hotkeys[k];
                    if (hotkey == invalid_ch) {
                        config_error("Grid hotkey \"{}\" collides with extended control group \"{}\"! Please resolve the collision in cosmonarchy.yml.", hotkey, invalid_ch);
                    }
                }
            }
        }
    }
}

void load_config() {
    if (!std::filesystem::exists(g_config_path)) {
        save_config();
        return;
    }

    YAML::Node yaml = YAML::LoadFile(g_config_path.string());
    // Random settings
    load_field(yaml, "random-races", g_config.random_races);

    // Launch settings
    load_field(yaml, "disable-wmode-prompt", g_config.disable_wmode_prompt);
    load_field(yaml, "prefer-wmode", g_config.prefer_wmode);
    load_field(yaml, "allow-multiple-instances", g_config.allow_multiple_instances);

    // General UI
    load_field(yaml, "extended-camera-hotkeys", g_config.extended_camera_hotkeys);
    load_field(yaml, "show-timer", g_config.show_timer);
    load_field(yaml, "show-apm", g_config.show_apm);
    load_field(yaml, "always-show-extended-tooltips", g_config.always_show_extended_tooltips);
    load_field(yaml, "suppress-windows-key", g_config.suppress_windows_key);

    // Selection UI
    load_field(yaml, "show-rally-lines", g_config.show_rally_lines);
    load_field(yaml, "show-order-lines", g_config.show_order_lines);
    load_field(yaml, "show-queued-order-lines", g_config.show_queued_order_lines);
    load_field(yaml, "show-unique-gas-order-line-color", g_config.show_unique_gas_order_line_color);
    load_field(yaml, "show-aura-circles", g_config.show_aura_circles);
    load_field(yaml, "show-ability-ranges", g_config.show_ability_ranges);
    load_field(yaml, "always-show-condensed-selection", g_config.always_show_condensed_selection);
    load_field(yaml, "use-alt-for-selection-ui", g_config.use_alt_for_selection_ui);

    // Minimap UI
    load_field(yaml, "show-minimap-cameras", g_config.show_minimap_cameras);
    load_field(yaml, "allow-minimap-terrain-toggle", g_config.can_toggle_minimap_terrain);

    // Team UI
    load_field(yaml, "show-team-ui", g_config.show_team_ui);
    load_field(yaml, "show-team-ui-apm", g_config.show_team_ui_apm);
    load_field(yaml, "show-team-ui-names", g_config.show_team_ui_names);
    load_field(yaml, "show-team-ui-self", g_config.show_team_ui_self);
    load_field(yaml, "show-ingame-global-queue", g_config.show_ingame_global_queue);

    // Witness and replay UI
    load_field(yaml, "show-replay-progress-bar", g_config.show_replay_progress_bar);

    // Scrolling
//	load_field(yaml, "use-spacebar-for-scrolling", config.use_spacebar_for_scrolling);
    load_field(yaml, "use-pan-scrolling", g_config.use_pan_scrolling);
    load_field(yaml, "scroll-sensitivity", g_config.scroll_sensitivity);

    // Audiovisuals
    load_field(yaml, "celebrations-enabled", g_config.celebrations_enabled);
    if (auto yaml_advisors = yaml["advisors"]) {
        load_field(yaml_advisors, "common", g_config.common_advisors);
        load_field(yaml_advisors, "terran", g_config.terran_advisors);
        load_field(yaml_advisors, "protoss", g_config.protoss_advisors);
        load_field(yaml_advisors, "zerg", g_config.zerg_advisors);
        load_field(yaml_advisors, "askosi", g_config.askosi_advisors);
    }

    // Hotkeys
    load_field(yaml, "quick-casting", g_config.quick_casting);
    load_field(yaml, "quick-building", g_config.quick_building);
    auto yaml_grid = yaml["grid-hotkeys"];
    if (yaml_grid) {
        load_field(yaml_grid, "enabled", g_config.grid_hotkeys);
        load_field(yaml_grid, "layout", g_config.grid_layout);
    }
    load_field(yaml, "extended-control-groups", g_config.extended_control_groups);
    load_field(yaml, "default-to-steal-hotkeys", g_config.default_to_steal_hotkeys);
    load_field(yaml, "tilde-control-group", g_config.tilde_control_group);
    load_field(yaml, "flip-attack-and-patrol", g_config.flip_attack_and_patrol);

    // Player Color
    if (auto yaml_colors = yaml["preferred-colors"]) {
        load_field(yaml_colors, "common", g_config.common_preferred_colors);
        load_field(yaml_colors, "terran", g_config.terran_preferred_colors);
        load_field(yaml_colors, "protoss", g_config.protoss_preferred_colors);
        load_field(yaml_colors, "zerg", g_config.zerg_preferred_colors);
        load_field(yaml_colors, "shuffle", g_config.shuffle_preferred_colors);
    }

    // Debug
    load_field(yaml, "verbose", g_config.verbose);
    gptp::g_logger.set_enabled(g_config.verbose);
    load_field(yaml, "log-randoms", g_config.log_randoms);
    gptp::g_local_random_logger.set_enabled(g_config.log_randoms);
    gptp::g_sync_random_logger.set_enabled(g_config.log_randoms);

    // Map directory
    load_field(yaml, "default-map-directory", g_config.default_map_directory);

    // Replays
    load_field(yaml, "auto-save-replays", g_config.auto_save_replays);
    load_field(yaml, "sanitize-replay-chat", g_config.sanitize_replay_chat);

    validate_config();
    save_config();
}

static std::string make_valid_colors_string() {
    std::stringstream ss;
    ss << "Valid colors are:\n";

    bool first = true;
    size_t i = 0;
    size_t line_w = 0;
    for (const auto& color : player_color_identifiers) {
        if (is_color_banned((PlayerColor)i++)) {
            continue;
        }

        if (first) {
            first = false;
        } else {
            ss << ", ";
            line_w += 2;
        }
        if (line_w > 80) {
            ss << "\n";
            line_w = 0;
        }

        ss << color;
        line_w += color.size();
    }

    return ss.str();
}

void save_config() {
    YAML::Emitter yaml{};
    yaml << YAML::BeginMap;

    yaml << YAML::Comment("Cosmonarchy config");
    yaml << YAML::Newline << YAML::Newline;
    
    yaml << YAML::Comment("Random settings");
    yaml << YAML::Newline << YAML::Comment("Possible values: Zerg, Terran, Protoss, Askosi, All, Original");
    yaml << YAML::Newline << YAML::Comment("You must select at least two values or All will be used");
    write_field(yaml, "random-races", g_config.random_races);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Launch settings");
    write_field(yaml, "disable-wmode-prompt", g_config.disable_wmode_prompt);
    write_field(yaml, "prefer-wmode", g_config.prefer_wmode);
    write_field(yaml, "allow-multiple-instances", g_config.allow_multiple_instances);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("General UI");
    write_field(yaml, "extended-camera-hotkeys", g_config.extended_camera_hotkeys);
    write_field(yaml, "show-timer", g_config.show_timer);
    write_field(yaml, "show-apm", g_config.show_apm);
    write_field(yaml, "always-show-extended-tooltips", g_config.always_show_extended_tooltips);
    write_field(yaml, "suppress-windows-key", g_config.suppress_windows_key);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Selection UI");
    yaml << YAML::Newline << YAML::Comment("Possible values: true, ingame, witness, false");
    write_field(yaml, "show-rally-lines", g_config.show_rally_lines);
    write_field(yaml, "show-order-lines", g_config.show_order_lines);
    write_field(yaml, "show-queued-order-lines", g_config.show_queued_order_lines);
    write_field(yaml, "show-unique-gas-order-line-color", g_config.show_unique_gas_order_line_color);
    write_field(yaml, "show-aura-circles", g_config.show_aura_circles);
    write_field(yaml, "show-ability-ranges", g_config.show_ability_ranges);
    write_field(yaml, "always-show-condensed-selection", g_config.always_show_condensed_selection);
    write_field(yaml, "use-alt-for-selection-ui", g_config.use_alt_for_selection_ui);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Minimap UI");
    yaml << YAML::Newline << YAML::Comment("Possible values: true, ingame, witness, false");
    write_field(yaml, "show-minimap-cameras", g_config.show_minimap_cameras);
    write_field(yaml, "allow-minimap-terrain-toggle", g_config.can_toggle_minimap_terrain);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Team UI");
    write_field(yaml, "show-team-ui", g_config.show_team_ui);
    write_field(yaml, "show-team-ui-apm", g_config.show_team_ui_apm);
    write_field(yaml, "show-team-ui-names", g_config.show_team_ui_names);
    write_field(yaml, "show-team-ui-self", g_config.show_team_ui_self);
    write_field(yaml, "show-ingame-global-queue", g_config.show_ingame_global_queue);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Witness and replay UI");
    write_field(yaml, "show-replay-progress-bar", g_config.show_replay_progress_bar);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Scrolling");
//	write_field(yaml, "use-spacebar-for-scrolling", config.use_spacebar_for_scrolling);
    write_field(yaml, "use-pan-scrolling", g_config.use_pan_scrolling);
    write_field(yaml, "scroll-sensitivity", g_config.scroll_sensitivity);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Audiovisuals");
    write_field(yaml, "celebrations-enabled", g_config.celebrations_enabled);
    {
        std::stringstream available_advisors;
        available_advisors << "Available Advisors are: ";
        for (u32 i = 0; i < (u32)Advisor::Count; ++i) {
            if (i > 0) {
                available_advisors << ", ";
            }
            available_advisors << advisor_names[i];
        }
        yaml << YAML::Newline << YAML::Comment("Advisors will be randomized from the common list combined with the list of the race you are playing.");
        yaml << YAML::Newline << YAML::Comment("You can use [] to make a list empty. e.g.\nterran:\n  []");
        yaml << YAML::Newline << YAML::Comment("Which is probably what you will want to do, if you want to just use the same advisors for every race.");
        yaml << YAML::Newline << YAML::Comment(available_advisors.str());
        write_field(yaml, "advisors", YAML::BeginMap);
        write_field(yaml, "common", g_config.common_advisors);
        write_field(yaml, "terran", g_config.terran_advisors);
        write_field(yaml, "protoss", g_config.protoss_advisors);
        write_field(yaml, "zerg", g_config.zerg_advisors);
        write_field(yaml, "askosi", g_config.askosi_advisors);
        yaml << YAML::EndMap;
    }
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Hotkeys");
    write_field(yaml, "quick-casting", g_config.quick_casting);
    write_field(yaml, "quick-building", g_config.quick_building);
    {
        write_field(yaml, "grid-hotkeys", YAML::BeginMap);
        write_field(yaml, "enabled", g_config.grid_hotkeys);
        write_field(yaml, "layout", YAML::BeginSeq);
        for (const std::string& str : g_config.grid_layout) {
            yaml << str;
        }
        yaml << YAML::EndSeq << YAML::EndMap;
    }
    yaml << YAML::Newline << YAML::Comment("Adds extra control groups at R, F, V, T, G, B. Must be used with grid hotkeys!");
    write_field(yaml, "extended-control-groups", g_config.extended_control_groups);
    write_field(yaml, "default-to-steal-hotkeys", g_config.default_to_steal_hotkeys);
    yaml << YAML::Newline << YAML::Comment("Replaces tilde with an additional control group, moves console to F12");
    write_field(yaml, "tilde-control-group", g_config.tilde_control_group);
    write_field(yaml, "flip-attack-and-patrol", g_config.flip_attack_and_patrol);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Player Color");
    {
        yaml << YAML::Newline << YAML::Comment("");
        yaml << YAML::Newline << YAML::Comment(make_valid_colors_string());
        write_field(yaml, "preferred-colors", YAML::BeginMap);
        write_field(yaml, "common", g_config.common_preferred_colors);
        write_field(yaml, "terran", g_config.terran_preferred_colors);
        write_field(yaml, "protoss", g_config.protoss_preferred_colors);
        write_field(yaml, "zerg", g_config.zerg_preferred_colors);
        write_field(yaml, "shuffle", g_config.shuffle_preferred_colors);
        yaml << YAML::EndMap;
    }
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Debug");
    write_field(yaml, "verbose", g_config.verbose);
    write_field(yaml, "log-randoms", g_config.log_randoms);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Maps");
    yaml << YAML::Newline << YAML::Comment("Use single backslashes for paths!");
    yaml << YAML::Newline << YAML::Comment("The map directory must be within Starcraft's 'maps' folder.");
    yaml << YAML::Newline << YAML::Comment("Example: BroodWar\\CMBW-maps");
    write_field(yaml, "default-map-directory", g_config.default_map_directory);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::Comment("Replays");
    write_field(yaml, "auto-save-replays", g_config.auto_save_replays);
    write_field(yaml, "sanitize-replay-chat", g_config.sanitize_replay_chat);
    yaml << YAML::Newline << YAML::Newline;

    yaml << YAML::EndMap;

    std::ofstream config_file{g_config_path.string()};
    config_file << yaml.c_str();
}

bool file_exists(const std::string& file) {
    return std::ifstream(file).good();
}
