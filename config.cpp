#include "config.h"
#include "yaml-cpp/yaml.h"
#include <fstream>
#include "config_utility.h"
#include <logger.h>

Config config;

// This code is pretty gross, the yaml library is pretty bad. It would probably be better to have a custom config format. -Veeq7

bool file_exists(std::string file) {
	return std::ifstream(file).good();
}

void load_config() {
	if (file_exists(CONFIG_FILENAME)) {
		YAML::Node config_node = YAML::LoadFile(CONFIG_FILENAME);

		load_field(config_node, "disable-wmode-prompt", config.disable_wmode_prompt);
		load_field(config_node, "prefer-wmode", config.prefer_wmode);

		load_field(config_node, "extended-camera-hotkeys", config.extended_camera_hotkeys);
		load_field(config_node, "show-timer", config.show_timer);
		load_field(config_node, "show-apm", config.show_apm);
		load_field(config_node, "always-show-extended-tooltips", config.always_show_extended_tooltips);

		load_field(config_node, "show-rally-lines", config.show_rally_lines);
		load_field(config_node, "show-order-lines", config.show_order_lines);
		load_field(config_node, "show-queued-order-lines", config.show_queued_order_lines);
		load_field(config_node, "show-unique-gas-order-line-color", config.show_unique_gas_order_line_color);
		load_field(config_node, "show-aura-circles", config.show_aura_circles);

		load_field(config_node, "witness-show-apm", config.witness_show_apm);
		load_field(config_node, "witness-show-order-lines", config.witness_show_order_lines);
		load_field(config_node, "show-replay-progress-bar", config.show_replay_progress_bar);

		load_field(config_node, "show-team-camera", config.show_team_camera);
		load_field(config_node, "show-team-ui", config.show_team_ui);
		load_field(config_node, "show-team-ui-apm", config.show_team_ui_apm);
		load_field(config_node, "show-team-ui-names", config.show_team_ui_names);
		load_field(config_node, "show-team-ui-self", config.show_team_ui_self);

		load_field(config_node, "quick-casting", config.quick_casting);
		load_field(config_node, "quick-building", config.quick_building);

		load_field(config_node, "celebrations-enabled", config.celebrations_enabled);
		load_field(config_node, "verbose", config.verbose);
		GPTP::doLogging = config.verbose;

		load_field(config_node, "default-map-directory", config.default_map_directory);
		load_field(config_node, "auto-save-replays", config.auto_save_replays);

		auto grid_node = config_node["grid-hotkeys"];
		if (grid_node) {
			load_field(grid_node, "enabled", config.grid_hotkeys);
			load_field(grid_node, "layout", config.layout);
		}
	}
	save_config();
}

void save_config() {
	YAML::Emitter out;
	out << YAML::BeginMap;

	out << YAML::Comment("Cosmonarchy Config");
	out << YAML::Newline << YAML::Newline;
	out << YAML::Comment("Launch");
	write_field(out, "disable-wmode-prompt", config.disable_wmode_prompt);
	write_field(out, "prefer-wmode", config.prefer_wmode);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("In-Game UI");
	write_field(out, "extended-camera-hotkeys", config.extended_camera_hotkeys);
	write_field(out, "show-timer", config.show_timer);
	write_field(out, "show-apm", config.show_apm);
	write_field(out, "always-show-extended-tooltips", config.always_show_extended_tooltips);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Selection UI");
	write_field(out, "show-rally-lines", config.show_rally_lines);
	write_field(out, "show-order-lines", config.show_order_lines);
	write_field(out, "show-queued-order-lines", config.show_queued_order_lines);
	write_field(out, "show-unique-gas-order-line-color", config.show_unique_gas_order_line_color);
	write_field(out, "show-aura-circles", config.show_aura_circles);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Witness UI");
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

	out << YAML::Comment("Visuals");
	write_field(out, "celebrations-enabled", config.celebrations_enabled);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Debug");
	write_field(out, "verbose", config.verbose);
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