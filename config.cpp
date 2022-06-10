#include "config.h"
#include "yaml-cpp/yaml.h"
#include <fstream>
#include "config_utility.h"
#include <logger.h>

Config config;

// NOTE: this code is pretty gross, the yaml library is pretty bad, so not sure what to do about it
// it would probably be better to have a custom config format

bool file_exists(std::string file) {
	return std::ifstream(file).good();
}

void load_config() {
	if (file_exists(CONFIG_FILENAME))
	{
		YAML::Node config_node = YAML::LoadFile(CONFIG_FILENAME);

		LoadField(config_node, "disable-wmode-prompt", config.disable_wmode_prompt);
		LoadField(config_node, "prefer-wmode", config.prefer_wmode);

		LoadField(config_node, "show-timer", config.show_timer);
		LoadField(config_node, "extended-camera-hotkeys", config.extended_camera_hotkeys);
		LoadField(config_node, "always-show-extended-tooltips", config.always_show_extended_tooltips);
		LoadField(config_node, "quick-casting", config.quick_casting);
		LoadField(config_node, "quick-building", config.quick_building);

		LoadField(config_node, "verbose", config.verbose);
		GPTP::doLogging = config.verbose;

		auto grid_node = config_node["grid-hotkeys"];
		if (grid_node) {
			LoadField(grid_node, "enabled", config.grid_hotkeys);
			LoadField(grid_node, "layout", config.layout);
		}
	}
	save_config();
}

void save_config() {
	YAML::Emitter out;
	out << YAML::BeginMap;

	out << YAML::Comment("Cosmonarchy config");
	out << YAML::Newline << YAML::Newline;
	out << YAML::Comment("Launch");
	write_field(out, "disable-wmode-prompt", config.disable_wmode_prompt);
	write_field(out, "prefer-wmode", config.prefer_wmode);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("UI");
	write_field(out, "show-timer", config.show_timer);
	write_field(out, "extended-camera-hotkeys", config.extended_camera_hotkeys);
	write_field(out, "always-show-extended-tooltips", config.always_show_extended_tooltips);
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

	out << YAML::EndMap;
	std::ofstream config_file(CONFIG_FILENAME);
	config_file << out.c_str();
}