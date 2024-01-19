#include "config.h"
#include "yaml-cpp/yaml.h"
#include <fstream>
#include "config_utility.h"
#include <logger.h>

#include <format>

Config config;

// This code is pretty gross, the yaml library is pretty bad. It would probably be better to have a custom config format. -Veeq7

bool file_exists(std::string file) {
	return std::ifstream(file).good();
}

static void config_error(const std::string_view message, const auto&... args) {
	MessageBoxA(0, std::format(message, args...).c_str(), "Invalid config.yml!", MB_OK | MB_ICONERROR);
	exit(0);
}

static AdvisorType parse_advisor_name(const char* name) {
	Parser parser = {};
	parser.at = (char*)name;
	AdvisorType advisor = (AdvisorType)parser_get_enum(&parser, advisor_symbol_names, ARRAY_SIZE(advisor_symbol_names), CHAR_MAX);
	if (advisor == CHAR_MAX) {
		config_error("Advisor \"{}\" does not exist! Please remove it from your config.yml.", name);
		return (AdvisorType) CHAR_MAX;
	}
	return advisor;
}

static void load_advisors(const YAML::Node& config_node, const char* name, AdvisorSetting* advisor) {
	load_field(config_node, name, advisor->strings);
	advisor->types.clear();
	for (const std::string& name : advisor->strings) {
		advisor->types.push_back(parse_advisor_name(name.c_str()));
	}
}

static void validate_config() {
	if (config.extended_control_groups) {
		if (!config.grid_hotkeys) {
			config.extended_control_groups = false;
			config_error("Extended control groups must be used together with grid hotkeys!");
		}
		for (u32 i = 0; i < 3; i++) {
			for (u32 j = 0; j < 3; j++) {
				char hotkey = config.layout[i][j];

				for (u32 k = 0; k < 6; k++) {
					static const char* valid_hotkeys = "RFVTGB";
					char valid_ch = valid_hotkeys[k];
					if (hotkey == valid_ch) {
						config.extended_control_groups = false;
						config_error("Grid hotkey \"{}\" collides with extended control group \"{}\"! Please resolve the collision in config.yml.", hotkey, valid_ch);
					}
				}
			}
		}
	}
}

void load_config() {
	if (file_exists(CONFIG_FILENAME)) {
		YAML::Node config_node = YAML::LoadFile(CONFIG_FILENAME);

		// Launch settings
		load_field(config_node, "disable-wmode-prompt", config.disable_wmode_prompt);
		load_field(config_node, "prefer-wmode", config.prefer_wmode);

		// General UI
		load_field(config_node, "extended-camera-hotkeys", config.extended_camera_hotkeys);
		load_field(config_node, "show-timer", config.show_timer);
		load_field(config_node, "show-apm", config.show_apm);
		load_field(config_node, "always-show-extended-tooltips", config.always_show_extended_tooltips);
		load_field(config_node, "require-control-for-console-hotkey", config.require_control_for_console_hotkey);

		// Selection UI
		load_field(config_node, "show-rally-lines", config.show_rally_lines);
		load_field(config_node, "show-order-lines", config.show_order_lines);
		load_field(config_node, "show-queued-order-lines", config.show_queued_order_lines);
		load_field(config_node, "show-unique-gas-order-line-color", config.show_unique_gas_order_line_color);
		load_field(config_node, "show-aura-circles", config.show_aura_circles);
		load_field(config_node, "show-ability-ranges", config.show_ability_ranges);
		load_field(config_node, "always-show-condensed-selection", config.always_show_condensed_selection);

		// Witness and replay UI
		load_field(config_node, "witness-show-apm", config.witness_show_apm);
		load_field(config_node, "witness-show-order-lines", config.witness_show_order_lines);
		load_field(config_node, "show-replay-progress-bar", config.show_replay_progress_bar);

		// Team UI
		load_field(config_node, "show-team-camera", config.show_team_camera);
		load_field(config_node, "show-team-ui", config.show_team_ui);
		load_field(config_node, "show-team-ui-apm", config.show_team_ui_apm);
		load_field(config_node, "show-team-ui-names", config.show_team_ui_names);
		load_field(config_node, "show-team-ui-self", config.show_team_ui_self);

		// Scrolling
	//	load_field(config_node, "use-spacebar-for-scrolling", config.use_spacebar_for_scrolling);
		load_field(config_node, "use-pan-scrolling", config.use_pan_scrolling);
		load_field(config_node, "scroll-sensitivity", config.scroll_sensitivity);

		// Audiovisuals
		load_advisors(config_node, "common-advisors", &config.common_advisors);
		load_advisors(config_node, "terran-advisors", &config.terran_advisors);
		load_advisors(config_node, "protoss-advisors", &config.protoss_advisors);
		load_advisors(config_node, "zerg-advisors", &config.zerg_advisors);
		load_field(config_node, "celebrations-enabled", config.celebrations_enabled);

		// Hotkeys
		load_field(config_node, "quick-casting", config.quick_casting);
		load_field(config_node, "quick-building", config.quick_building);
		auto grid_node = config_node["grid-hotkeys"];
		if (grid_node) {
			load_field(grid_node, "enabled", config.grid_hotkeys);
			load_field(grid_node, "layout", config.layout);
		}
		load_field(config_node, "extended-control-groups", config.extended_control_groups);
		load_field(config_node, "default-to-steal-hotkeys", config.default_to_steal_hotkeys);

		// Debug
		load_field(config_node, "verbose", config.verbose);
		GPTP::logger.set_enabled(config.verbose);

		// Map directory
		load_field(config_node, "default-map-directory", config.default_map_directory);
		load_field(config_node, "auto-save-replays", config.auto_save_replays);
	}
	validate_config();
	save_config();
}

void save_config() {
	YAML::Emitter out;
	out << YAML::BeginMap;

	out << YAML::Comment("Cosmonarchy BW Config");
	out << YAML::Newline << YAML::Newline;
	out << YAML::Comment("Launch settings");
	write_field(out, "disable-wmode-prompt", config.disable_wmode_prompt);
	write_field(out, "prefer-wmode", config.prefer_wmode);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("General UI");
	write_field(out, "extended-camera-hotkeys", config.extended_camera_hotkeys);
	write_field(out, "show-timer", config.show_timer);
	write_field(out, "show-apm", config.show_apm);
	write_field(out, "always-show-extended-tooltips", config.always_show_extended_tooltips);
	write_field(out, "require-control-for-console-hotkey", config.require_control_for_console_hotkey);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Selection UI");
	write_field(out, "show-rally-lines", config.show_rally_lines);
	write_field(out, "show-order-lines", config.show_order_lines);
	write_field(out, "show-queued-order-lines", config.show_queued_order_lines);
	write_field(out, "show-unique-gas-order-line-color", config.show_unique_gas_order_line_color);
	write_field(out, "show-aura-circles", config.show_aura_circles);
	write_field(out, "show-ability-ranges", config.show_ability_ranges);
	write_field(out, "always-show-condensed-selection", config.always_show_condensed_selection);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Witness and replay UI");
	write_field(out, "witness-show-apm", config.witness_show_apm);
	write_field(out, "witness-show-order-lines", config.witness_show_order_lines);
	write_field(out, "show-replay-progress-bar", config.show_replay_progress_bar);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Team UI");
	write_field(out, "show-team-camera", config.show_team_camera);
	write_field(out, "show-team-ui", config.show_team_ui);
	write_field(out, "show-team-ui-apm", config.show_team_ui_apm);
	write_field(out, "show-team-ui-names", config.show_team_ui_names);
	write_field(out, "show-team-ui-self", config.show_team_ui_self);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Scrolling");
//	write_field(out, "use-spacebar-for-scrolling", config.use_spacebar_for_scrolling);
	write_field(out, "use-pan-scrolling", config.use_pan_scrolling);
	write_field(out, "scroll-sensitivity", config.scroll_sensitivity);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Audiovisuals");
	std::stringstream available_advisors;
	available_advisors << "Available Advisors are: ";
	for (u32 i = 0; i < AdvisorType_Count; ++i) {
		available_advisors << advisor_symbol_names[i];
		if (i != AdvisorType_Count - 1) {
			available_advisors << ", ";
		}
	}
	out << YAML::Newline << YAML::Comment("Advisors will be randomized from the common list combined with the list of the race you are playing.");
	out << YAML::Newline << YAML::Comment("You can use [] to make a list empty. e.g. terran-advisors: []");
	out << YAML::Newline << YAML::Comment("Which is probably what you will want to do, if you want to just use common-advisors.");
	out << YAML::Newline << YAML::Comment(available_advisors.str());
	write_field(out, "common-advisors", config.common_advisors.strings);
	write_field(out, "terran-advisors", config.terran_advisors.strings);
	write_field(out, "protoss-advisors", config.protoss_advisors.strings);
	write_field(out, "zerg-advisors", config.zerg_advisors.strings);
	out << YAML::Newline << YAML::Newline;
	write_field(out, "celebrations-enabled", config.celebrations_enabled);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Hotkeys");
	write_field(out, "quick-casting", config.quick_casting);
	write_field(out, "quick-building", config.quick_building);
	write_field(out, "grid-hotkeys", YAML::BeginMap);
	write_field(out, "enabled", config.grid_hotkeys);
	write_field(out, "layout", YAML::BeginSeq);
	for (const std::string& str : config.layout) {
		out << str;
	}
	out << YAML::EndSeq << YAML::EndMap;
	out << YAML::Newline << YAML::Comment("Adds extra control groups at R, F, V, T, G, B. Must be used with grid hotkeys!");
	write_field (out, "extended-control-groups", config.extended_control_groups);
	write_field (out, "default-to-steal-hotkeys", config.default_to_steal_hotkeys);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Debug");
	write_field(out, "verbose", config.verbose);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Maps");
	out << YAML::Newline << YAML::Comment("Use single backslashes for paths!");
	out << YAML::Newline << YAML::Comment("The map directory must be within Starcraft's 'maps' folder.");
	out << YAML::Newline << YAML::Comment("Example: BroodWar\\CMBW-maps");
	write_field(out, "default-map-directory", config.default_map_directory);
	write_field(out, "auto-save-replays", config.auto_save_replays);
	out << YAML::Newline << YAML::Newline;

	out << YAML::EndMap;
	std::ofstream config_file(CONFIG_FILENAME);
	config_file << out.c_str();
}