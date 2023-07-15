#pragma once
#include <string>
#include <array>

const std::string CONFIG_FILENAME = "config.yml";
struct Config {
	// Launch Settings
	bool disable_wmode_prompt = false;
	bool prefer_wmode = true;

	// UI
	bool extended_camera_hotkeys = true;
	bool show_timer = true;
	bool show_apm = true;
	bool always_show_extended_tooltips = false;

	bool show_rally_lines = true;
	bool show_order_lines = false;
	bool show_queued_order_lines = false;
	bool show_unique_gas_order_line_color = false;
	bool show_aura_circles = false;

	bool witness_show_apm = true;
	bool witness_show_order_lines = true;
	bool show_replay_progress_bar = true;

	bool show_team_camera = true;
	bool show_team_ui = true;
	bool show_team_ui_apm = true;
	bool show_team_ui_names = false;
	bool show_team_ui_self = false;

	// Visuals
	bool celebrations_enabled = true;

	// Hotkeys
	bool quick_casting = false;
	bool quick_building = false;
	bool grid_hotkeys = false;
	std::array<std::string, 3> layout = {"QWE", "ASD", "ZXC"};

	// Debug
	bool verbose = false;

	// Map directory
	std::string default_map_directory = "BroodWar"; // std::string is awful, but the yaml library requires it for text... -Veeq7
	bool auto_save_replays = true;
};
extern Config config;

bool file_exists(std::string file);
void load_config();
void save_config();