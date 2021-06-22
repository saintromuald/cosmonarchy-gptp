#include "config.h"
#include "yaml-cpp/yaml.h"
#include <fstream>
#include "config_utility.h"
#include <logger.h>

Config config;

// NOTE: this code is pretty gross, the yaml library is pretty bad, so not sure what to do about it
// it would probably be better to have a custom config format

bool FileExists(std::string file)
{
	return std::ifstream(file).good();
}

void LoadConfig()
{
	if (FileExists(CONFIG_FILENAME))
	{
		YAML::Node configNode = YAML::LoadFile(CONFIG_FILENAME);

		LoadField(configNode, "disable-wmode-prompt", config.disable_wmode_prompt);
		LoadField(configNode, "prefer-wmode", config.prefer_wmode);

		LoadField(configNode, "show-timer", config.show_timer);
		LoadField(configNode, "extended-camera-hotkeys", config.extended_camera_hotkeys);
		LoadField(configNode, "always-show-extended-tooltips", config.always_show_extended_tooltips);

		LoadField(configNode, "verbose", config.verbose);
		GPTP::doLogging = config.verbose;

		auto gridNode = configNode["grid-hotkeys"];
		if (gridNode)
		{
			LoadField(gridNode, "enabled", config.grid_hotkeys);
			LoadField(gridNode, "layout", config.layout);
		}
	}
	SaveConfig();
}

void SaveConfig()
{
	YAML::Emitter out;
	out << YAML::BeginMap;

	out << YAML::Comment("Cosmonarchy config");
	out << YAML::Newline << YAML::Newline;
	out << YAML::Comment("Launch");
	WriteField(out, "disable-wmode-prompt", config.disable_wmode_prompt);
	WriteField(out, "prefer-wmode", config.prefer_wmode);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("UI");
	WriteField(out, "show-timer", config.show_timer);
	WriteField(out, "extended-camera-hotkeys", config.extended_camera_hotkeys);
	WriteField(out, "always-show-extended-tooltips", config.always_show_extended_tooltips);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Debug");
	WriteField(out, "verbose", config.verbose);
	out << YAML::Newline << YAML::Newline;

	out << YAML::Comment("Hotkeys");
	WriteField(out, "grid-hotkeys", YAML::BeginMap);
	WriteField(out, "enabled", config.grid_hotkeys);
	WriteField(out, "layout", YAML::BeginSeq);
	for (const std::string& str : config.layout)
	{
		out << str;
	}
	out << YAML::EndSeq << YAML::EndMap;

	out << YAML::EndMap;
	std::ofstream configFile(CONFIG_FILENAME);
	configFile << out.c_str();
}