#pragma once
#include <string>
#include <array>

const std::string CONFIG_FILENAME = "config.yml";
struct Config {
	// Launch Settings
	bool disable_wmode_prompt = false;
	bool prefer_wmode = true;

	// UI
	bool show_timer = true;
	bool extended_camera_hotkeys = true;
	bool always_show_extended_tooltips = false;

	// Hotkeys
	bool grid_hotkeys = false;
	std::array<std::string, 3> layout = {"QWE", "ASD", "ZXC"};

	// Debug
	bool verbose = false;
	/*bool dump_ai_info = false;
	bool dump_desync_game_data_arrays = false;
	bool dump_desync_animation_info = false;
	bool dump_crash_info = false;*/
};
extern Config config;

bool file_exists(std::string file);
void load_config();
void save_config();