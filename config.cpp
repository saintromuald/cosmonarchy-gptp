#include "config.h"
#include "yaml-cpp/yaml.h"
#include <fstream>
#include "config_utility.h"

Config config;

bool ConfigExists() {
	return std::ifstream(CONFIG_FILENAME).good();
}

void LoadConfig() {
	if (!ConfigExists()) { 
		SaveConfig();
	}
	else {
		YAML::Node configNode = YAML::LoadFile(CONFIG_FILENAME);

		LoadField(configNode, config.show_timer, "show-timer");
		LoadField(configNode, config.extended_camera_hotkeys, "extended-camera-hotkeys");

		auto gridNode = configNode["grid-hotkeys"];
		if (gridNode) {
			LoadField(gridNode, config.grid_hotkeys, "enabled");
			LoadField(gridNode, config.layout, "layout");
		}
	}
}

void SaveConfig() {
	YAML::Emitter out;
	out << YAML::BeginMap;
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