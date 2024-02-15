#include "config.h"
#include <fstream>
#include <format>
#include <yaml-cpp/yaml.h>
#include "logger.h"

Config config;

// Have to use std::vformat and no `const auto&...`, because of some obscure VS issue that makes it not compile for Pr0nogo.
// It's sad, because it will make std::string allocate a copy, but it is what it is, we only do it once on a fail path anyways. -Veeq7
static void config_error(const std::string_view message, auto... args) {
    MessageBoxA(0, std::vformat(message, std::make_format_args(args...)).c_str(), "Invalid config.yml!", MB_OK | MB_ICONERROR);
    exit(0);
}

template<typename T>
static void load_field(const YAML::Node& node, const std::string& key, T& variable) {
	if (node[key]) {
		try {
			variable = node[key].as<T>();
		} catch (YAML::Exception) {
			config_error("Invalid value for variable \"{}\"! Please correct your config.yml.", key);
		}
	} else {
		config_error("Missing variable \"{}\"! Please correct your config.yml.", key);
	}
}

template<typename T>
static void write_field(YAML::Emitter& emitter, const std::string& key, const T& value) {
	emitter << YAML::Key << key << YAML::Value << value;
}

static AdvisorType parse_advisor_name(const char* name) {
	Parser parser = new_parser(name);
	AdvisorType advisor = (AdvisorType)parser_get_enum(&parser, advisor_symbol_names, ARRAY_SIZE(advisor_symbol_names), CHAR_MAX);
	if (advisor == CHAR_MAX) {
		config_error("Advisor \"{}\" does not exist! Please remove it from your config.yml.", name);
	}
	return advisor;
}

static void load_advisors(const YAML::Node& yaml, const char* name, AdvisorSetting* advisor) {
	load_field(yaml, name, advisor->strings);
	advisor->types.clear();
	for (const std::string& name : advisor->strings) {
		advisor->types.push_back(parse_advisor_name(name.c_str()));
	}
}

static void validate_config() {
	if (config.extended_control_groups) {
		if (!config.grid_hotkeys) {
			config_error("Extended control groups must be used together with grid hotkeys!");
		}
		for (u32 i = 0; i < 3; i++) {
			for (u32 j = 0; j < 3; j++) {
				char& hotkey = config.layout[i][j];
				if (hotkey < 'A' || hotkey > 'Z') {
					if (hotkey >= 'a' && hotkey <= 'z') {
						hotkey = toupper(hotkey);
					} else {
						config_error("Grid hotkey '{}' is not valid! Please correct your config.yml to use hotkeys from the A-Z range.", hotkey);
					}
				}
				for (u32 k = 0; k < 6; k++) {
					// Note: We don't need to validate against tilde, since not only it's not included in A-Z range, neither '`' nor '~' maps to the correct VK code anyways.
					static const char* invalid_hotkeys = "RFVTGB";
					char invalid_ch = invalid_hotkeys[k];
					if (hotkey == invalid_ch) {
						config_error("Grid hotkey \"{}\" collides with extended control group \"{}\"! Please resolve the collision in config.yml.", hotkey, invalid_ch);
					}
				}
			}
		}
	}
}

void load_config() {
	if (!file_exists(CONFIG_FILENAME)) {
		save_config();
		return;
	}

	YAML::Node yaml = YAML::LoadFile(CONFIG_FILENAME);

	// Launch settings
	load_field(yaml, "disable-wmode-prompt", config.disable_wmode_prompt);
	load_field(yaml, "prefer-wmode", config.prefer_wmode);

	// General UI
	load_field(yaml, "extended-camera-hotkeys", config.extended_camera_hotkeys);
	load_field(yaml, "show-timer", config.show_timer);
	load_field(yaml, "show-apm", config.show_apm);
	load_field(yaml, "always-show-extended-tooltips", config.always_show_extended_tooltips);

	// Selection UI
	load_field(yaml, "show-rally-lines", config.show_rally_lines);
	load_field(yaml, "show-order-lines", config.show_order_lines);
	load_field(yaml, "show-queued-order-lines", config.show_queued_order_lines);
	load_field(yaml, "show-unique-gas-order-line-color", config.show_unique_gas_order_line_color);
	load_field(yaml, "show-aura-circles", config.show_aura_circles);
	load_field(yaml, "show-ability-ranges", config.show_ability_ranges);
	load_field(yaml, "always-show-condensed-selection", config.always_show_condensed_selection);

	// Witness and replay UI
	load_field(yaml, "witness-show-apm", config.witness_show_apm);
	load_field(yaml, "witness-show-order-lines", config.witness_show_order_lines);
	load_field(yaml, "show-replay-progress-bar", config.show_replay_progress_bar);

	// Team UI
	load_field(yaml, "show-team-camera", config.show_team_camera);
	load_field(yaml, "show-team-ui", config.show_team_ui);
	load_field(yaml, "show-team-ui-apm", config.show_team_ui_apm);
	load_field(yaml, "show-team-ui-names", config.show_team_ui_names);
	load_field(yaml, "show-team-ui-self", config.show_team_ui_self);

	// Scrolling
//	load_field(yaml, "use-spacebar-for-scrolling", config.use_spacebar_for_scrolling);
	load_field(yaml, "use-pan-scrolling", config.use_pan_scrolling);
	load_field(yaml, "scroll-sensitivity", config.scroll_sensitivity);

	// Audiovisuals
	load_advisors(yaml, "common-advisors", &config.common_advisors);
	load_advisors(yaml, "terran-advisors", &config.terran_advisors);
	load_advisors(yaml, "protoss-advisors", &config.protoss_advisors);
	load_advisors(yaml, "zerg-advisors", &config.zerg_advisors);
	load_field(yaml, "celebrations-enabled", config.celebrations_enabled);

	// Hotkeys
	load_field(yaml, "quick-casting", config.quick_casting);
	load_field(yaml, "quick-building", config.quick_building);
	auto yaml_grid = yaml["grid-hotkeys"];
	if (yaml_grid) {
		load_field(yaml_grid, "enabled", config.grid_hotkeys);
		load_field(yaml_grid, "layout", config.layout);
	}
	load_field(yaml, "extended-control-groups", config.extended_control_groups);
	load_field(yaml, "default-to-steal-hotkeys", config.default_to_steal_hotkeys);
	load_field(yaml, "tilde-control-group", config.tilde_control_group);

	// Debug
	load_field(yaml, "verbose", config.verbose);
	GPTP::logger.set_enabled(config.verbose);

	// Map directory
	load_field(yaml, "default-map-directory", config.default_map_directory);
	load_field(yaml, "auto-save-replays", config.auto_save_replays);

	validate_config();
	save_config();
}

void save_config() {
	YAML::Emitter yaml;
	yaml << YAML::BeginMap;

	yaml << YAML::Comment("Cosmonarchy BW Config");
	yaml << YAML::Newline << YAML::Newline;
	yaml << YAML::Comment("Launch settings");
	write_field(yaml, "disable-wmode-prompt", config.disable_wmode_prompt);
	write_field(yaml, "prefer-wmode", config.prefer_wmode);
	yaml << YAML::Newline << YAML::Newline;

	yaml << YAML::Comment("General UI");
	write_field(yaml, "extended-camera-hotkeys", config.extended_camera_hotkeys);
	write_field(yaml, "show-timer", config.show_timer);
	write_field(yaml, "show-apm", config.show_apm);
	write_field(yaml, "always-show-extended-tooltips", config.always_show_extended_tooltips);
	yaml << YAML::Newline << YAML::Newline;

	yaml << YAML::Comment("Selection UI");
	write_field(yaml, "show-rally-lines", config.show_rally_lines);
	write_field(yaml, "show-order-lines", config.show_order_lines);
	write_field(yaml, "show-queued-order-lines", config.show_queued_order_lines);
	write_field(yaml, "show-unique-gas-order-line-color", config.show_unique_gas_order_line_color);
	write_field(yaml, "show-aura-circles", config.show_aura_circles);
	write_field(yaml, "show-ability-ranges", config.show_ability_ranges);
	write_field(yaml, "always-show-condensed-selection", config.always_show_condensed_selection);
	yaml << YAML::Newline << YAML::Newline;

	yaml << YAML::Comment("Witness and replay UI");
	write_field(yaml, "witness-show-apm", config.witness_show_apm);
	write_field(yaml, "witness-show-order-lines", config.witness_show_order_lines);
	write_field(yaml, "show-replay-progress-bar", config.show_replay_progress_bar);
	yaml << YAML::Newline << YAML::Newline;

	yaml << YAML::Comment("Team UI");
	write_field(yaml, "show-team-camera", config.show_team_camera);
	write_field(yaml, "show-team-ui", config.show_team_ui);
	write_field(yaml, "show-team-ui-apm", config.show_team_ui_apm);
	write_field(yaml, "show-team-ui-names", config.show_team_ui_names);
	write_field(yaml, "show-team-ui-self", config.show_team_ui_self);
	yaml << YAML::Newline << YAML::Newline;

	yaml << YAML::Comment("Scrolling");
//	write_field(yaml, "use-spacebar-for-scrolling", config.use_spacebar_for_scrolling);
	write_field(yaml, "use-pan-scrolling", config.use_pan_scrolling);
	write_field(yaml, "scroll-sensitivity", config.scroll_sensitivity);
	yaml << YAML::Newline << YAML::Newline;

	yaml << YAML::Comment("Audiovisuals");
	std::stringstream available_advisors;
	available_advisors << "Available Advisors are: ";
	for (u32 i = 0; i < AdvisorType_Count; ++i) {
		available_advisors << advisor_symbol_names[i];
		if (i != AdvisorType_Count - 1) {
			available_advisors << ", ";
		}
	}
	yaml << YAML::Newline << YAML::Comment("Advisors will be randomized from the common list combined with the list of the race you are playing.");
	yaml << YAML::Newline << YAML::Comment("You can use [] to make a list empty. e.g. terran-advisors: []");
	yaml << YAML::Newline << YAML::Comment("Which is probably what you will want to do, if you want to just use common-advisors.");
	yaml << YAML::Newline << YAML::Comment(available_advisors.str());
	write_field(yaml, "common-advisors", config.common_advisors.strings);
	write_field(yaml, "terran-advisors", config.terran_advisors.strings);
	write_field(yaml, "protoss-advisors", config.protoss_advisors.strings);
	write_field(yaml, "zerg-advisors", config.zerg_advisors.strings);
	yaml << YAML::Newline << YAML::Newline;
	write_field(yaml, "celebrations-enabled", config.celebrations_enabled);
	yaml << YAML::Newline << YAML::Newline;

	yaml << YAML::Comment("Hotkeys");
	write_field(yaml, "quick-casting", config.quick_casting);
	write_field(yaml, "quick-building", config.quick_building);
	write_field(yaml, "grid-hotkeys", YAML::BeginMap);
	write_field(yaml, "enabled", config.grid_hotkeys);
	write_field(yaml, "layout", YAML::BeginSeq);
	for (const std::string& str : config.layout) {
		yaml << str;
	}
	yaml << YAML::EndSeq << YAML::EndMap;
	yaml << YAML::Newline << YAML::Comment("Adds extra control groups at R, F, V, T, G, B. Must be used with grid hotkeys!");
	write_field (yaml, "extended-control-groups", config.extended_control_groups);
	write_field (yaml, "default-to-steal-hotkeys", config.default_to_steal_hotkeys);
	yaml << YAML::Newline << YAML::Comment("Replaces tilde with an additional control group, moves console to F12");
	write_field(yaml, "tilde-control-group", config.tilde_control_group);
	yaml << YAML::Newline << YAML::Newline;

	yaml << YAML::Comment("Debug");
	write_field(yaml, "verbose", config.verbose);
	yaml << YAML::Newline << YAML::Newline;

	yaml << YAML::Comment("Maps");
	yaml << YAML::Newline << YAML::Comment("Use single backslashes for paths!");
	yaml << YAML::Newline << YAML::Comment("The map directory must be within Starcraft's 'maps' folder.");
	yaml << YAML::Newline << YAML::Comment("Example: BroodWar\\CMBW-maps");
	write_field(yaml, "default-map-directory", config.default_map_directory);
	write_field(yaml, "auto-save-replays", config.auto_save_replays);
	yaml << YAML::Newline << YAML::Newline;

	yaml << YAML::EndMap;
	std::ofstream config_file(CONFIG_FILENAME);
	config_file << yaml.c_str();
}

bool file_exists(const std::string& file) {
	return std::ifstream(file).good();
}
