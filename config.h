#pragma once
#include <string>
#include <array>

const std::string CONFIG_FILENAME = "config.yml";
struct Config {
	// UI
	bool show_timer = true;
	bool extended_camera_hotkeys = true;

	// Hotkeys
	bool grid_hotkeys = true;
	std::array<std::string, 3> layout = { "QWE", "ASD", "ZXC" };

	// Debug
	/*bool dump_ai_info = false;
	bool dump_desync_game_data_arrays = false;
	bool dump_desync_animation_info = false;
	bool dump_crash_info = false;*/
};
extern Config config;

extern bool ConfigExists();
extern void LoadConfig();
extern void SaveConfig();