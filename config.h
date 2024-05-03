#pragma once
#include <string>
#include <array>
#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "trigger_system/trigger_system.h"
#include "config_utils.h"

inline const std::filesystem::path g_config_path = bw::get_starcraft_path() / "cosmonarchy.yml";

struct AdvisorSetting {
	std::vector<std::string> strings;
	std::vector<AdvisorType> types;
};

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
	UiBool show_rally_lines = true;
	UiBool show_order_lines = false;
	UiBool show_queued_order_lines = false;
	UiBool show_unique_gas_order_line_color = false;
	UiBool show_aura_circles = false;
	UiBool show_ability_ranges = true;
	UiBool always_show_condensed_selection = false;
	UiBool use_alt_for_selection_ui = false;
	
	// Minimap UI
	UiBool show_minimap_cameras = true;

	// Team UI
	bool show_team_ui = true;
	bool show_team_ui_apm = true;
	bool show_team_ui_names = false;
	bool show_team_ui_self = false;

	// Witness and replay UI
	bool show_replay_progress_bar = true;

	// Scrolling
//	bool use_spacebar_for_scrolling = false;
	bool use_pan_scrolling = false;
	f32 scroll_sensitivity = 1.0;

	// Audiovisuals
	AdvisorSetting common_advisors{
		{ advisor_symbol_names[AdvisorType_Default] },
	};
	AdvisorSetting terran_advisors{
		{ advisor_symbol_names[AdvisorType_Terran] },
	};
	AdvisorSetting protoss_advisors{
		{ advisor_symbol_names[AdvisorType_Protoss] },
	};
	AdvisorSetting zerg_advisors{
		{ advisor_symbol_names[AdvisorType_Zerg] },
	};
	bool celebrations_enabled = true;

	// Hotkeys
	bool quick_casting = false;
	bool quick_building = false;
	bool grid_hotkeys = true;
	std::array<std::string, 3> grid_layout = {"QWE", "ASD", "ZXC"};
	bool extended_control_groups = true;
	bool default_to_steal_hotkeys = false;
	bool tilde_control_group = true;

	// Player Color
	std::vector<PlayerColor> preferred_colors;

	// Debug
	bool verbose = false;

	// Map directory
	std::string default_map_directory = "CMBW-Root\\CMBW-maps";
	bool auto_save_replays = true;
};

inline Config g_config{};

void load_config();
void save_config();