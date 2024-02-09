#pragma once
#include <string>
#include <array>
#include "trigger_system/trigger_system.h"

struct AdvisorSetting {
	std::vector<std::string> strings;
	std::vector<AdvisorType> types;
};

const std::string CONFIG_FILENAME = "config.yml";
struct Config {
	// Launch settings
	bool disable_wmode_prompt = true;
	bool prefer_wmode = false;

	// General UI
	bool extended_camera_hotkeys = true;
	bool show_timer = true;
	bool show_apm = true;
	bool always_show_extended_tooltips = false;

	// Selection UI
	bool show_rally_lines = true;
	bool show_order_lines = false;
	bool show_queued_order_lines = false;
	bool show_unique_gas_order_line_color = false;
	bool show_aura_circles = false;
	bool show_ability_ranges = true;
	bool always_show_condensed_selection = false;

	// Witness and replay UI
	bool witness_show_apm = true;
	bool witness_show_order_lines = true;
	bool show_replay_progress_bar = true;

	// Team UI
	bool show_team_camera = true;
	bool show_team_ui = true;
	bool show_team_ui_apm = true;
	bool show_team_ui_names = false;
	bool show_team_ui_self = false;

	// Scrolling
//	bool use_spacebar_for_scrolling = false;
	bool use_pan_scrolling = false;
	f32 scroll_sensitivity = 1.0;

	// Audiovisuals
	AdvisorSetting common_advisors = {
		{ advisor_symbol_names[AdvisorType_Default] },
	};
	AdvisorSetting terran_advisors = {
		{ advisor_symbol_names[AdvisorType_Terran] },
	};
	AdvisorSetting protoss_advisors = {
		{ advisor_symbol_names[AdvisorType_Protoss] },
	};
	AdvisorSetting zerg_advisors = {
		{ advisor_symbol_names[AdvisorType_Zerg] },
	};
	bool celebrations_enabled = true;

	// Hotkeys
	bool quick_casting = false;
	bool quick_building = false;
	bool grid_hotkeys = false;
	std::array<std::string, 3> layout = {"QWE", "ASD", "ZXC"};
	bool extended_control_groups = false;
	bool default_to_steal_hotkeys = false;
	bool tilde_control_group = true;

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