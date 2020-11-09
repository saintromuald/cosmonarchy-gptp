#include "config.h"
#include "yaml-cpp/yaml.h"
#include <fstream>
#include "config_utility.h"

Config config;

bool ConfigExists() {
	return std::ifstream(CONFIG_FILENAME).good();
}

void LoadConfig() {
	if (ConfigExists()) {
		YAML::Node configNode = YAML::LoadFile(CONFIG_FILENAME);

		LoadField(configNode, "disable-wmode-prompt", config.disable_wmode_prompt);
		LoadField(configNode, "prefer-wmode", config.prefer_wmode);

		LoadField(configNode, "show-timer", config.show_timer);
		LoadField(configNode, "extended-camera-hotkeys", config.extended_camera_hotkeys);

		auto gridNode = configNode["grid-hotkeys"];
		if (gridNode) {
			LoadField(gridNode, "enabled", config.grid_hotkeys);
			LoadField(gridNode, "layout", config.layout);
		}
	}
	SaveConfig();
}

void SaveConfig() {
	YAML::Emitter out;
	out << YAML::BeginMap;
	WriteField(out, "disable-wmode-prompt", config.disable_wmode_prompt);
	WriteField(out, "prefer-wmode", config.prefer_wmode);

	WriteField(out, "show-timer", config.show_timer);
	WriteField(out, "extended-camera-hotkeys", config.extended_camera_hotkeys);

	WriteField(out, "grid-hotkeys", YAML::BeginMap);
	WriteField(out, "enabled", config.grid_hotkeys);
	WriteField(out, "layout", YAML::BeginSeq);
	for (const std::string& str : config.layout) {
		out << str;
	}
	out << YAML::EndSeq << YAML::EndMap;
	out << YAML::EndMap;

	std::ofstream configFile(CONFIG_FILENAME);
	configFile << out.c_str();
}