#pragma once
#include "../definitions.h"
#include "structures.h"
#include "scbwhelper.h"
#pragma pack(1)

// Contains the definitions of various data structures, arrays, pointers and internal constants in StarCraft.exe.

#define SCBW_DATA(type, name, offset) type const name = (type)offset;		// Designed as const to make sure programmers don't read from or write to the address itself.
#define SCBW_DATA_REF(type, name, offset)  extern type& name;				// Contains the definitions of various data structures, arrays, pointers and internal constants in StarCraft.exe.

//////////////////////////////////////////////////////////////// @}
/// @name Linked list nodes
//////////////////////////////////////////////////////////////// @{

/// The first nodes in StarCraft's internal linked lists (taken from BWAPI's Offsets.h)
SCBW_DATA(Unit**,			g_first_visible_unit,			0x00628430);
SCBW_DATA(Unit**,			g_first_hidden_unit,			0x006283EC);	// Includes units in transports/bunkers, worker inside gas buildings, nukes in silos, and units in production.
SCBW_DATA(Unit**,			g_last_hidden_unit,				0x00628428);	// Same as above.
SCBW_DATA(Bullet**,			g_first_active_bullet,			0x0064DEC4);
SCBW_DATA(Unit**,			first_scanner_sweep,			0x006283F4);
SCBW_DATA(Unit**,			last_scanner_sweep,				0x00628434);
SCBW_DATA(Unit**,			first_psi_field_provider,		0x0063FF54);
SCBW_DATA(u32*,				psi_field_grp_width,			0x0063FF4C);
SCBW_DATA(u32*,				psi_field_grp_height,			0x0063FF58);
SCBW_DATA(u8**,				psi_field_mask,					0x006D5BD8);
SCBW_DATA(u32*,				psi_unknown,					0x0063FF48);
SCBW_DATA(u32*,				g_player_visions,				0x0057F0B0);
SCBW_DATA(Bullet**,			first_bullet,					0x0064DEC4);
SCBW_DATA(Bullet**,			last_bullet,					0x0064DEAC);
SCBW_DATA(Bullet**,			first_free_bullet,				0x0064EED8);
SCBW_DATA(Bullet**,			last_free_bullet,				0x0064EEDC);
SCBW_DATA(u32*,				bullet_count,					0x0064DEBC);

struct Units12 {
	Unit* unit[12];
};
SCBW_DATA(Units12*, first_player_unit, 0x006283F8); // Indexed by player Id

//////////////////////////////////////////////////////////////// @}
/// @name Structure tables
//////////////////////////////////////////////////////////////// @{

SCBW_DATA(Units12*,			g_completed_factory_counts,		0x0068A9D8);
SCBW_DATA(Units12*,			g_trigger_factory_counts,		0x0068A9A8);

SCBW_DATA(Unit*,			g_unit_table,					0x0059CCA8);
SCBW_DATA(u32*,				g_unit_count,					0x006283F0);
SCBW_DATA(Bullet*,			g_bullet_table,					0x0064B2E8);
SCBW_DATA_REF(UnitDeaths*,	g_unit_deaths_table,			0x0058A364);
SCBW_DATA(Sprite*,			g_sprite_table,					0x00629D98);
SCBW_DATA(Image*,			g_image_table,					0x0052F568);
SCBW_DATA_REF(UnitDeaths*,	g_unit_killed_table,			0x005878A4);

SCBW_DATA_REF(UnitDeaths*,	g_completed_unit_count_table,	0x00584DE4);

SCBW_DATA_REF(UnitDeaths*,	g_all_unit_count_table,			0x00582324);
SCBW_DATA(UnitDeaths*,		g_all_unit_count_table_original,0x00582324);

SCBW_DATA(Thingy**,			first_lone_sprite,				0x00654874);

// From locations.cpp
SCBW_DATA(MapSize*,			g_map_tile_size,				0x0057F1D4);
SCBW_DATA(Location*,		g_location_table,				0x0058DC60);

// For buttonsets
SCBW_DATA_REF(ButtonSet*,	g_buttonset_table,				0x005187E8);
SCBW_DATA_REF(UnitStatFuncArrayEntry*, unitStatFuncs,		0x005193A0);

SCBW_DATA(ActionPointer*,	g_action_table,					0x00512800); // From triggers.cpp
SCBW_DATA(ActionPointer*,	g_briefing_action_table,		0x00512800);
SCBW_DATA(char**,			g_chk_string_table,				0x005993D4);
SCBW_DATA(HWND*,			g_main_window,					0x0051BFB0);
SCBW_DATA(s32*,				g_music_volume,					0x006D5BB4);
SCBW_DATA(b8*,				g_music_playing,				0x006D5BBC);

//////////////////////////////////////////////////////////////// @}
/// @name Data Variables and Constants
//////////////////////////////////////////////////////////////// @{

// From resources.cpp
struct Resources {
	s32 minerals[12];
	s32 gas[12];
	s32 cumulative_gas[12];
	s32 cumulative_minerals[12];
};
SCBW_DATA(Resources*, resources, 0x0057F0F0);

struct cv5 {
	u16 index;
	/*
	u8 unk : 4;
	u8 flags : 4;
	u8 ground_height_old : 4;
	u8 unk2 : 4;
	*/
	u16 flags;
	u16 left_edge;
	u16 top_edge;
	u16 right_edge;
	u16 bottom_edge;
	u16 unk3;
	u16 unk4;
	u16 unk5;
	u16 unk6;
	u16 megatiles[16];
};

SCBW_DATA(cv5**, tileGroup, 0x006D5EC8);

/// TBL files
SCBW_DATA(const u16*,		g_stat_txt_tbl,		0x006D1238);
SCBW_DATA(const StringTbl*, g_map_string_tbl,	0x005993D4);

SCBW_DATA(const char*,		g_string_empty,		0x00501B7D);

/// Units that are selected by the current player (or the player viewing the replay).
struct UnitsSel {
	Unit* unit[SelectionCap];
};
SCBW_DATA(UnitsSel*,		g_client_selection_group,		0x00597208);
SCBW_DATA(Unit*,			g_client_selection_group_end,	0x00597238);
SCBW_DATA(u8*,				g_client_selection_count,		0x0059723D);

// Screen position relative to the map
SCBW_DATA(s32*,				g_screen_x,				0x00628448);
SCBW_DATA(s32*,				g_screen_y,				0x00628470);

// To use with a player ID
SCBW_DATA(Point16*,			g_start_positions,		0x0058D720);

// Mouse position relative to the screen
SCBW_DATA(const Vector32*,	g_mouse,				0x006CDDC4);

// Booleans indicating which keyboard buttons are held
// use g_is_keycode_used[VK_param] where VK_param is one of the Virtual-Key Codes of Windows as seen in msdn here: https://msdn.microsoft.com/fr-fr/library/windows/desktop/dd375731%28v=vs.85%29.aspx
SCBW_DATA(const b8*, g_is_keycode_used,		0x00596A18);
SCBW_DATA(const b8*, g_is_holding_shift,	0x00596A28);
SCBW_DATA(const b8*, g_is_holding_ctrl,		0x00596A29);
SCBW_DATA(const b8*, g_is_holding_alt,		0x00596A2A);

// Maybe used by Map triggers to init a progressive movement toward the corresponding location, see GameImpl::setScreenPosition of BWAPI
SCBW_DATA(u32*,		g_move_to_x,			0x0062848C);
SCBW_DATA(u32*,		g_move_to_y,			0x006284A8);

// Contains various information (names, player types, race types, and associated forces) of each player in the current game
SCBW_DATA(Player*,	g_player_table,			0x0057EEE0);

struct SupplyData {
	u32 provided[PLAYER_COUNT];
	u32 used[PLAYER_COUNT];
	u32 max[PLAYER_COUNT];
};
SCBW_DATA(SupplyData*,	g_race_supply,			0x00582144);  //Array; Use Unit::get_race() to get the index.

SCBW_DATA(u32*,		g_elapsed_time_frames,		0x0057F23C);
SCBW_DATA(u32*,		g_countdown_timer,			0x0058D6F4);	// Vanilla trigger actions/conditions (From BWAPI Offsets.h)
SCBW_DATA(u32*,		g_elapsed_time_seconds,		0x0058D6F8);
SCBW_DATA(u8*,		g_game_type,				0x00596820);	// Compare with GameType::Enum
SCBW_DATA(s32*,		g_active_nation_id,			0x00512678);
SCBW_DATA(s32*,		g_active_player_id,			0x0051267C);	// Used by get_active_player_next_selection
SCBW_DATA(s32*,		g_local_nation_id,			0x00512684);	// Actually stores the player ID
SCBW_DATA(s32*,		g_local_human_id,			0x00512688);	// Invalid in replay games
SCBW_DATA(u8*,		g_current_tileset,			0x00596828);	// Tileset of current map - compare with TilesetType::Enum
SCBW_DATA(s32*,		g_provider_max_buffer,		0x0057F0D8);
SCBW_DATA(u32*,		g_provider_latency_calls,	0x0057F090);
SCBW_DATA(u32*,		g_command_queue_size,		0x00654AA0);
SCBW_DATA(char*,	g_turn_buffer,				0x00654880);

// For reset_tooltip_info, set_tooltip_info, set_tooltip_info_unit_case5, etc
SCBW_DATA(u16*,		tooltip_ore_cost,			0x0068C1D0);
SCBW_DATA(u16*,		tooltip_gas_cost,			0x0068C1D2);
SCBW_DATA(u16*,		tooltip_energy_cost,		0x0068C1D4); // Used to be Protoss energy
SCBW_DATA(u16*,		tooltip_unused_cost,		0x0068C1D6); // Used to be Terran energy
SCBW_DATA(u16*,		tooltip_time_cost,			0x0068C1D8); // Used to be Zerg energy
SCBW_DATA(u16*,		tooltip_p_supply_cost,		0x0068C1DA);
SCBW_DATA(u16*,		tooltip_t_supply_cost,		0x0068C1DC);
SCBW_DATA(u16*,		tooltip_z_supply_cost,		0x0068C1DE);

struct Event;
typedef bool __fastcall generic_interact_function(BinDlg*, Event*);
SCBW_DATA(generic_interact_function**, generic_interact_functions, 0x005014AC);

// Added from BWAPI Offsets.h
SCBW_DATA(char*,		current_map_filename,	0x0057FD3C);
SCBW_DATA(char*,		current_map_name,		0x0057FE40);
SCBW_DATA_REF(_uavail*, g_unit_availability,	0x0057F27C);

// Ping data
SCBW_DATA(Ping*, ping_array, 0x0059C1B8); // 16

	// -- Added for various hooks used by Cosmonarchy BW! -- \\

// For race_dropdown
SCBW_DATA(StringMap_b*,	single_race_select,			0x0051AAA0);
SCBW_DATA(StringMap_w*,	single_type_select,			0x0051AAA8);
SCBW_DATA(u16*,			g_network_table,			0x006D1220);

// For init_singleplayer_custom
SCBW_DATA(u32*,			selected_game_type,			0x0059BA64);
SCBW_DATA(u16*,			selected_game_type_param,	0x0059BA5C);
SCBW_DATA(char*,		selected_singleplayer_race,	0x0057F266);
SCBW_DATA(char*,		custom_singleplayer,		0x0057F1E3);

// For Lobotomy Mine
SCBW_DATA(char**,		g_status_screen_info_button_unk,	0x00519F40);
SCBW_DATA(GrpHead**,	g_cmdicons_grp,						0x0068C1E0);

// For init_game_load
SCBW_DATA(void*,		scenario_chk,				0x006D0F24);
SCBW_DATA(ForceFlags*,	player_force_flags,			0x0058D5B8); // array of 0x4
SCBW_DATA(u8*,			player_force,				0x0059BDA8); // array of 0x8
SCBW_DATA(u8*,			player_force_replay_sth,	0x006D11A1); // array of 0x8
SCBW_DATA(FILE*,		load_game_file_handle,		0x006D1218);
SCBW_DATA(u8*,			multiplayer_mode,			0x0057F0B4);
SCBW_DATA(u32*,			initial_seed,				0x0051CA10);
SCBW_DATA(u32*,			replay_seed,				0x006D0F38);
SCBW_DATA(u32*,			random_counts,				0x0051C610); // count of 0x400
SCBW_DATA(u32*,			random_counts_total,		0x0051CA18);
SCBW_DATA(u16*,			campaign_index,				0x0057F244);
SCBW_DATA(void**,		map_archive_handle,			0x00596BC4);
SCBW_DATA(GameData*,	game_data,					0x005967F8);
SCBW_DATA(u8*,			map_started,				0x006D121C);
SCBW_DATA(u8*,			player_has_left,			0x00581D62); // array of 0x8
SCBW_DATA(u8*,			loss_type,					0x00581D61);
SCBW_DATA(u8*,			unk_victory_variable,		0x00581D66);
SCBW_DATA(u32*,			save_load_success,			0x006D5A68);
SCBW_DATA(u32*,			elapsed_time_modified,		0x006D5BCC);
SCBW_DATA(u32*,			saved_elapsed_seconds,		0x0057F240);
SCBW_DATA(Options*,		g_options_list,				0x006CDFD4);
SCBW_DATA(ReplayData*,	replay_data,				0x00596BBC);
SCBW_DATA(u32*,			g_replay_visions,			0x006D0F18);
SCBW_DATA(u32*,			player_visions,				0x0057F0B0);
SCBW_DATA(u32*,			g_replay_reveal_all,		0x006D0F1C);
SCBW_DATA(u32*,			next_replay_command_frame,	0x005136C8);
SCBW_DATA(u32*,			player_explored_visions,	0x0057EEB8);
SCBW_DATA(s32*,			sub_two_check_two,			0x004EE0F0);

// For get_response_sfx
SCBW_DATA(u32*,			selection_sound_counter,	0x00640548);
SCBW_DATA(u32*,			g_last_random_sfx_id,		0x006D5BE0);

// For load_race_ui
SCBW_DATA(u8*,			console_index,				0x0057F1E2);
SCBW_DATA(u16*,			current_music_id,			0x0057F1DE);
SCBW_DATA(char*,		campaign_first_mission_bw,	0x005122C0);
SCBW_DATA(char*,		campaign_first_mission,		0x005122BC);
SCBW_DATA(u32*,			console_race_specific,		0x0051A3B4);

// For load_game_create
SCBW_DATA(u32*,			is_host,					0x00596888);
SCBW_DATA(u8*,			out_of_game,				0x006D5A10);
SCBW_DATA(u32*,			sc_main_state,				0x00596904);
SCBW_DATA(u8*,			game_active,				0x006D11EC);
SCBW_DATA(u16*,			next_sc_main_state,			0x0051CE90);
SCBW_DATA(u32*,			replay_end_frame,			0x006D0F31);

// For level_cheat_create
SCBW_DATA(u32*,			level_cheat_unlocked,		0x006D11C0);
SCBW_DATA(char*,		player_name,				0x0057EEB5);

// For update_placement_state_for_placement_box
SCBW_DATA(u8*,		placing_player,					0x0064088C);
SCBW_DATA(u16*,		place_building_x,				0x00640890);
SCBW_DATA(u16*,		place_building_y,				0x00640892);
SCBW_DATA(u32*,		is_placing_addon,				0x00640884);
SCBW_DATA(u16*,		place_building_unit_id,			0x0064088A);
SCBW_DATA(u32*,		place_building_base_type,		0x00640888);

SCBW_DATA(u16*,		move_to_tile_x,					0x0057F1D0);
SCBW_DATA(u16*,		move_to_tile_y,					0x0057F1D2);

// For increment_unit_scores_ex
SCBW_DATA(u32*,		factories_owned,				0x005820B4); // Array of 12	btw
SCBW_DATA(u32*,		units_produced,					0x00581E14); // Array of 12	btw
SCBW_DATA(u32*,		structures_built,				0x00581F64); // Array of 12	btw
SCBW_DATA(u32*,		unit_score,						0x00581ED4); // Array of 12	btw
SCBW_DATA(u32*,		building_score,					0x00582024); // Array of 12	btw
SCBW_DATA(u32*,		units_owned,					0x00581E44); // Array of 12	btw
SCBW_DATA(u32*,		buildings_owned,				0x00581F94); // Array of 12	btw

// For increment_unit_scores
SCBW_DATA(u32*,		factories_produced_owned_total,	0x00582084); // Array of 12	btw
SCBW_DATA(u32*,		structures_produced_owned_total,0x00581F34); // Array of 12	btw
SCBW_DATA(u32*,		units_produced_owned_total,		0x00581DE4); // Array of 12	btw

// For refresh_screen
SCBW_DATA(u32*,		g_place_building_message_id,	0x00640958);

// For targeted_order
SCBW_DATA(u32*,		g_order_count,					0x00641698);

// For create_upgrade_level_tooltip
SCBW_DATA(u16*,		g_upgrade_level_tooltip,		0x0068C1CC);
SCBW_DATA(u16**,	g_stat_txt_buffer,				0x006D1238);

struct BufferStr {
	char buf[260];
};

// For drawUnitInfo
SCBW_DATA(u16*,			g_supplies_available_temp,	0x006CA93C);
SCBW_DATA(u8*,			g_status_screen_func,		0x0068C1E5);
SCBW_DATA(BufferStr*,	g_struct_char,				0x006CA9F8);

// For draw_supply_unit_info
SCBW_DATA(BufferStr*,	g_struct_char_2,			0x006CAB08);
SCBW_DATA(char*,		g_unk_string_char,			0x006CA668);

// For draw_wireframe_shield
SCBW_DATA(char*,		g_char_buffer,				0x0068C208);
SCBW_DATA(char*,		g_byte_shield_string,		0x0068C20A); // count of 22
SCBW_DATA(u8*,			g_wireframe_constants,		0x00515280); // count of 40
SCBW_DATA(char**,		g_image_draw_palette_index,	0x0050CEC1); // count of 256
SCBW_DATA(u8*,			off_50CE81,					0x0050CE81);
SCBW_DATA(u8*,			off_50CE82,					0x0050CE82);

// For draw_building_info
SCBW_DATA(s32*,			g_selection_group_hp,		0x006CA94C);
SCBW_DATA(s16*,			g_ui_energy,				0x006CAC0C);
SCBW_DATA(s32*,			g_ui_shields,				0x006CA9EC);
SCBW_DATA(char*,		g_redraw_transport_ui,		0x006CA9F0);

// For AI_EMPShieldRequirementsProc
SCBW_DATA(s32*,			g_temp_hp_shields,			0x00695798);
SCBW_DATA(s32*,			g_temp_weapon,				0x00695604);

// For drawImageHPBar
SCBW_DATA(unk_61*,		struct_5140C4,				0x005140C4);
SCBW_DATA(unk_61*,		struct_5140D8,				0x005140D8);
SCBW_DATA(unk_62*,		struct_5140E8,				0x005140E8);

// For getUnitBulletDamage
SCBW_DATA(char*,		g_upgrades_level_sc,		0x0058D2B0);
SCBW_DATA(char*,		g_upgrades_level_bw,		0x0058F2FE);

// For find_next_bounce_target_proc
SCBW_DATA(Unit**,	g_bullet_bounce_missile_source,	0x0064EECC);
SCBW_DATA(Unit**,	g_bullet_bounce_missile_target,	0x0064EED0);
SCBW_DATA(Unit**,	g_bullet_bounce_missile_next,	0x0064EED4);

// For setUnderDisruptionWeb
SCBW_DATA(void**,	unk_array_59CB64,				0x0059CB64);	// count of 252
SCBW_DATA(Unit*,	g_unit_reference,				0x0059CB58);

// For startGame
SCBW_DATA(s8*,		g_game_state,					0x0066FBFA); // 0x4 Idling in lobby | 0x5 Game starting | 0x6 Slots locked | 0x7 Sending final commands | 0x8 Waiting for save data | 0x9 End lobby
SCBW_DATA(Player*,	g_player_replay_watchers,		0x00640970);

SCBW_DATA(s8*,		g_countdown_timer_interval,		0x0066FBFC);
SCBW_DATA(s32*,		g_countdown_timer_tick_count,	0x0059CC7C);
SCBW_DATA(s32*,		g_countdown_timer_tick_count_2,	0x0059CC88);
SCBW_DATA(s32*,		g_is_in_game,					0x006D5BC8);
SCBW_DATA(s32*,		g_countdown_time_remaining,		0x0059CC78);

// For air_splash
SCBW_DATA(Unit**,	g_air_splash_list,				0x0064EEE0);	// count of 1700
SCBW_DATA(s32*,		g_air_splash_list_count,		0x0046DEB4);
SCBW_DATA(s32*,		g_is_in_ground_splash_proc,		0x0064DEB8);

// For ai_add_military
SCBW_DATA(RegionArray**,	g_sai_paths,				0x006D5BFC);
SCBW_DATA(AiTown*,			g_ai_town_array,			0x006AA090);
SCBW_DATA(AiTownList*,		g_ai_town_list,				0x006AA050); // sizeof 0x64[0x8]

// For cmdrecv_hold_position
SCBW_DATA(s16*,		g_map_width_pixels,					0x00628450);
SCBW_DATA(s16*,		g_map_height_pixels,				0x006284B4);
SCBW_DATA(u32*,		g_order_type_4_position,			0x006D5C20);
SCBW_DATA(u32*,		g_order_type_4_target,				0x006D5C24);

// For openMapFile
SCBW_DATA(char*,	g_maps_folder,			0x0066FC08);
SCBW_DATA(s32*,		dword_5994DC,			0x005994DC);

// For loadMap
SCBW_DATA(ScenarioFormat*, stru_500574,		0x00500574);
SCBW_DATA(ScenarioFormat*, stru_50057C,		0x0050057C);
SCBW_DATA(s32*, scenarioCHKSize,			0x006D0F20);

// For game_loop
SCBW_DATA(s32*, visionUpdateCount,			0x0051CE98);
SCBW_DATA(s8*, visionUpdated,				0x0051CE9C);

// X405: For displayEstablishing Shot
SCBW_DATA(CampaignMission*, campaignMissions, 0x004FFF60);

// For stats_weapon_level / stats_armor_level
SCBW_DATA(s16*, g_unk_upgrade_count,			0x006CA940); // Seems to be unused. Only ever written to.

// For isUnitCloaked
SCBW_DATA(Unit**,	first_invisible_unit,		0x0063FF5C);
SCBW_DATA(s32*,		force_button_refresh,		0x0068C1B0);
SCBW_DATA(s8*,		portrait_redraw,			0x0068AC74); // sizeof 4
SCBW_DATA(s8*,		status_screen_redraw,		0x0068C1F8);
SCBW_DATA(BinDlg**,	control_under_mouse,		0x0068C1E8);
SCBW_DATA(BinDlg**,	control_under_mouse_value,	0x0068C1EC); // void* in Neiv's list

// For update_units
SCBW_DATA(s32*,		g_interceptor_focused_this_frame,		0x006957D0);
SCBW_DATA(s8*,		g_is_targeting,							0x00641694);
SCBW_DATA(s32*,		g_order_timer_reassign_count,			0x0059CCA4);
SCBW_DATA(s32*,		g_secondary_order_timer_reassign_count,	0x006283E8);
SCBW_DATA(Unit**,	g_active_iscript_entity,				0x006D11F4);
SCBW_DATA(Bullet**,	g_active_iscript_bullet,				0x006D11F8);
SCBW_DATA(Unit**,	g_active_iscript_unit,					0x006D11FC);
SCBW_DATA(Unit**,	g_first_dying_unit,						0x0062842C);
SCBW_DATA(Unit**,	g_lurker_splash_list,					0x0064DEC8); // sizeof 400
SCBW_DATA(s32*,		g_lurker_splash_list_index,				0x0064EEC8);
SCBW_DATA(s32*,		g_last_bullet_index,					0x0064DEA8);

// For order_allowed
SCBW_DATA(s32*,		g_last_internal_error,			0x0066FF60);
SCBW_DATA_REF(u16*,	g_orders_requirement_tree,		0x00514CF8);

// For larva_creep_spawn
SCBW_DATA(s16*,		g_larva_counter,				0x0067D3F8);

// For update_building_placement_state_units
SCBW_DATA(s8*,		g_building_placement_state,		0x006408F8);

// For progress_unit_frame
SCBW_DATA(s8*,		g_selection_circle_color,		0x00581D6A);

// For initializeBulet
SCBW_DATA(Unit**,	g_last_bullet_creator,			0x0064DEB0);
SCBW_DATA(s32*,		g_launch_spin_random,			0x0064DEC0);
SCBW_DATA(Point16*, g_attack_curve,					0x00513650);

// For ai_progress_build
SCBW_DATA(s32*,		g_game_minerals,				0x0057F0F0);
SCBW_DATA(s32*,		g_game_vespene,					0x0057F120);
SCBW_DATA(s32*,		g_ai_minerals_reserve,			0x006CA51C);
SCBW_DATA(s32*,		g_ai_vespene_reserve,			0x006CA4EC);

// For updateBriefingRoomObjectives
SCBW_DATA(BinDlg**, g_ready_room_dialog,				0x0066FF6C);
SCBW_DATA(s32*,		g_mission_objectives_string_index,	0x0058D6C4);

// For ai_manage_building_placement
SCBW_DATA(Unit**,	g_disabled_unit_array,			0x00692660);
SCBW_DATA(Unit**,	g_ai_builder_unk,				0x006D5DCC);

// For get_supplies_available
SCBW_DATA(s32*,		g_supplies_available_protoss,	0x00582264);
SCBW_DATA(s32*,		g_supplies_max_protoss,			0x005822C4);
SCBW_DATA(s32*,		g_supplies_available_terran,	0x005821D4);
SCBW_DATA(s32*,		g_supplies_max_terran,			0x00582234);
SCBW_DATA(s32*,		g_supplies_available_zerg,		0x00582144);
SCBW_DATA(s32*,		g_supplies_max_zerg,			0x005821A4);

// For get_supplies_used_no_args
SCBW_DATA(s32*,		g_supplies_used_protoss,		0x00582294);
SCBW_DATA(s32*,		g_supplies_used_terran,			0x00582204);
SCBW_DATA(s32*,		g_supplies_used_zerg,			0x00582174);

// For initialize_unit_counts
SCBW_DATA(s32*,		g_unit_count_score,				0x00581DE4);
SCBW_DATA(s8*,		unknown_57F41CB,				0x0057F1CB);

// For stat_res_main_proc
SCBW_DATA(s32*,		unknown_player_5153F8,			0x005153F8);
SCBW_DATA(s32*,		unknown_68C22C,					0x0068C22C);
SCBW_DATA(s32*,		unknown_68C230,					0x0068C230);

// For get_supplies_temp
SCBW_DATA(s16*,		g_supplies_used_temp,			0x006CA93E);

// For draw_supply_unit_status
SCBW_DATA(char*,	g_supply_available_buffer,		0x006CA668);
SCBW_DATA(char*,	g_supply_used_buffer,			0x006CA9F8);
SCBW_DATA(char*,	g_supply_max_buffer,			0x006CAB08);
SCBW_DATA(char*,	g_supply_provided_buffer,		0x006CAC78);

SCBW_DATA(char*,	g_kills_buffer,					0x006CAC78);

// For give_unit_to_player
SCBW_DATA(s32*,		g_current_trigger_player_id,	0x006509B0);

// For delete_ai_town_if_needed
SCBW_DATA(ResourceArea*, g_base_array,				0x00692688);

// For get_bunker_placement_position
SCBW_DATA(u32*,		g_region_ids_count,				0x0069265C);
SCBW_DATA(u16*,		g_region_ids,					0x0069262C);

SCBW_DATA(u8*,		unk_order_array,				0x0068C300);

SCBW_DATA(u32**,	g_active_tile_array_int,		0x006D1260);

SCBW_DATA(void**,	g_get_right_click_order_array,	0x005153FC);

// For prepare_unit_move_custom
SCBW_DATA(BinDlg*,	g_portrait,						0x0068AC98);

// For fog of war changes by DarkenedFantasies
SCBW_DATA(FogMask*,	g_fog_of_war_mask,				0x00657AA0);

// For score screen player name overrides, from DarkenedFantasies
SCBW_DATA(const StringTbl*,	g_network_table_DF,		0x006D1220);
SCBW_DATA(ColorTable*,		g_color_table,			0x0058F442);
SCBW_DATA(PlayerNames*,		g_player_name_list,		0x0059B3D8);

// For soundThread
SCBW_DATA(HANDLE*,		g_sound_thread_event,		0x006D5A00);
SCBW_DATA(s32*,			g_unk_sound_core,			0x006D5A08);
SCBW_DATA(s32*,			g_sound_volume,				0x00599900);
SCBW_DATA(s32*,			g_volume_reduction,			0x006D5A0C);
SCBW_DATA(DSound*,		g_direct_sound_interface,	0x006D59F4);
SCBW_DATA(s32*,			g_unk_sound_core_2,			0x005999C0);
SCBW_DATA(CritSect*,	g_sound_lock,				0x006D5F4C);

// For soundInit
SCBW_DATA(SoundChannel*,g_sound_channels,			0x005998F0);
SCBW_DATA(HMODULE*,		g_hd_sound_module,			0x006D59F0);
SCBW_DATA(s8*,			g_dda_initialized,			0x006D1265);
SCBW_DATA(s8*,			g_vid_initialized,			0x006D1266);
SCBW_DATA(s32*,			g_sound_memory_cache,		0x005129B8);
SCBW_DATA(HANDLE*,		g_sound_handle,				0x006D5A04);
SCBW_DATA(s32*,			g_unk_sound_core_3,			0x005998E4);
SCBW_DATA(s8*,			g_unk_sound_bool_core,		0x006D1264);
SCBW_DATA(char**,		g_current_sound_memory,		0x006D59FC);

SCBW_DATA(void*,		g_sfx_tbl_handle,			0x005999B0);

// For handle_window_messages
typedef void __fastcall InputProcsFunc(s32 arg);
struct InputProcsArray {
	InputProcsFunc* funcs[19];
};
SCBW_DATA(InputProcsArray*,	g_input_procs,		0x005968A0);

// For get_standard_attack_target
SCBW_DATA(u32*,		g_min_range,				0x006955B8); // sizeof 10
SCBW_DATA(s32*,		g_max_range_in_px,			0x006957A4);

// For show_right_click_error_if_needed
SCBW_DATA(u32*,		use_quiet_sounds,			0x0064086C);
SCBW_DATA(char*,	draw_out_text_left,			0x00641598);
SCBW_DATA(char*,	text_bytes,					0x00641674); // sizeof 13
SCBW_DATA(s32*,		unit_stat_update_timer,		0x00640B54);

// For cmdact_game_speed
SCBW_DATA(s32*,		last_control_index,			0x006D051C);
SCBW_DATA(b32*,		cpu_throttle,				0x006D5C00);
SCBW_DATA(b32*,		wants_redraw,				0x005968EC);
SCBW_DATA(s32*,		temp_cpu_throttle,			0x00655C3C);
SCBW_DATA(char*,	bin_interact,				0x006D1224);
SCBW_DATA(void**,	last_bindlg_menu_function,	0x006D1234);

//////////////////////////////////////////////////////////////// @}
/// @name *.DAT File Data
//////////////////////////////////////////////////////////////// @{

// The following code is based on BWAPI's BW/Offsets.h
// you can get the other addresses in http://code.google.com/p/bwapi/source/browse/trunk/bwapi/BWAPI/Source/BW/Offsets.h
// Since StarCraft is already running when the plugin is loaded, the DatLoad
// structs are guaranteed to have the corrent offsets.

struct DatLoad {
	void* address; // a real pointer
	u32 length;
	u32 entries;
};

namespace units_dat {
//	SCBW_DATA(const DatLoad*,	unitsDat,	0x00513C30); // now in scbwdata.cpp

	SCBW_DATA_REF(u16*,			graphic, unitsDat[0].address);							// 006644F8
	SCBW_DATA_REF(u16*,			subunit, unitsDat[1].address);							// 006607C0
	SCBW_DATA_REF(u16*,			subunit_2,	unitsDat[2].address);						// 00660C38
	SCBW_DATA_REF(u16*,			infested_unit_partial, unitsDat[3].address);			// 00664980 // (Id - UnitId::TerranCommandCenter) for it to work, last valid id is UnitId::Special_OvermindCocoon by default
	SCBW_DATA_REF(u32*,			construction_graphic, unitsDat[4].address);				// 006610B0
	SCBW_DATA_REF(u8*,			start_direction, unitsDat[5].address);					// 006605F0 // Direction unit will face after it is created. From 0 (top) to 31 (before top turning clockwise). 32 means random
	SCBW_DATA_REF(u8*,			shields_enabled, unitsDat[6].address);					// 006647B0
	SCBW_DATA_REF(u16*,			max_shield_points, unitsDat[7].address);				// 00660E00
	SCBW_DATA_REF(s32*,			max_hitpoints, unitsDat[8].address);					// 00662350
	SCBW_DATA_REF(u8*,			elevation, unitsDat[9].address);						// 00663150
	SCBW_DATA_REF(u8*,			movement_flags, unitsDat[10].address);					// 00660FC8
	SCBW_DATA_REF(u8*,			rank, unitsDat[11].address);							// 00663DD0
	SCBW_DATA_REF(u8*,			computer_idle_order, unitsDat[12].address);				// 00662EA0
	SCBW_DATA_REF(u8*,			human_idle_order, unitsDat[13].address);				// 00662268
	SCBW_DATA_REF(u8*,			return_to_idle_order, unitsDat[14].address);			// 00664898
	SCBW_DATA_REF(u8*,			attack_unit_order, unitsDat[15].address);				// 00663320
	SCBW_DATA_REF(u8*,			attack_move_order, unitsDat[16].address);				// 00663A50
	SCBW_DATA_REF(u8*,			ground_weapon, unitsDat[17].address);					// 006636B8
	SCBW_DATA_REF(u8*,			max_ground_hits, unitsDat[18].address);					// 006645E0
	SCBW_DATA_REF(u8*,			air_weapon, unitsDat[19].address);						// 006616E0
	SCBW_DATA_REF(u8*,			max_air_hits, unitsDat[20].address);					// 0065FC18
	SCBW_DATA_REF(u8*,			computer_ai_internal, unitsDat[21].address);			// 00660178
	SCBW_DATA_REF(u32*,			base_property, unitsDat[22].address);					// 00664080 // AKA SpecialAbilityFlags, UnitProperty, etc. Use with UnitProperty::Enum.
	SCBW_DATA_REF(u8*,			seek_range, unitsDat[23].address);						// 00662DB8 // AKA Target Acquisition Range, Acquire Target Range, etc.
	SCBW_DATA_REF(u8*,			sight_range, unitsDat[24].address);						// 00663238
	SCBW_DATA_REF(u8*,			armor_upgrade, unitsDat[25].address);					// 006635D0
	SCBW_DATA_REF(u8*,			size_type, unitsDat[26].address);						// 00662180
	SCBW_DATA_REF(u8*,			armor_amount, unitsDat[27].address);					// 0065FEC8
	SCBW_DATA_REF(u8*,			right_click_action, unitsDat[28].address);				// 00662098
	SCBW_DATA_REF(u16*,			sound_ready, unitsDat[29].address);						// 00661FC0
	SCBW_DATA_REF(u16*,			sound_what_first, unitsDat[30].address);				// 0065FFB0
	SCBW_DATA_REF(u16*,			sound_what_last, unitsDat[31].address);					// 00662BF0
	SCBW_DATA_REF(u16*,			sound_annoyed_first, unitsDat[32].address);				// 00663B38
	SCBW_DATA_REF(u16*,			sound_annoyed_last, unitsDat[33].address);				// 00661EE8
	SCBW_DATA_REF(u16*,			sound_yes_first, unitsDat[34].address);					// 00663C10
	SCBW_DATA_REF(u16*,			sound_yes_last, unitsDat[35].address);					// 00661440
	SCBW_DATA_REF(Point16*,		building_dimensions, unitsDat[36].address);				// 00662860	// Placement Box in PyDAT
	SCBW_DATA_REF(Point16*,		addon_placement, unitsDat[37].address);					// 006626E0 // Addon Position in PyDAT
	SCBW_DATA_REF(Box16*,		unit_bounds, unitsDat[38].address);						// 006617C8	// Unit Dimensions in PyDAT
	SCBW_DATA_REF(u16*,			portrait_id, unitsDat[39].address);						// 00662F88
	SCBW_DATA_REF(u16*,			mineral_cost, unitsDat[40].address);					// 00663888
	SCBW_DATA_REF(u16*,			vespene_cost, unitsDat[41].address);					// 0065FD00
	SCBW_DATA_REF(u16*,			time_cost, unitsDat[42].address);						// 00660428
	SCBW_DATA_REF(u16*,			requirement_offsets, unitsDat[43].address);				// 00660A70 // "Restriction Address" according to EUDDB
	SCBW_DATA_REF(GroupFlag*,	group_flags, unitsDat[44].address);						// 006637A0
	SCBW_DATA_REF(u8*,			supply_provided, unitsDat[45].address);					// 006646C8 // Supply shown in-game is (true supply) / 2; e.g. Zerglings have 1 true supply, Marines have 2 true supply, etc.
	SCBW_DATA_REF(u8*,			supply_required, unitsDat[46].address);					// 00663CE8
	SCBW_DATA_REF(u8*,			space_required, unitsDat[47].address);					// 00664410
	SCBW_DATA_REF(u8*,			space_provided, unitsDat[48].address);					// 00660988
	SCBW_DATA_REF(u16*,			build_score, unitsDat[49].address);						// 00663408
	SCBW_DATA_REF(u16*,			destroy_score, unitsDat[50].address);					// 00663EB8
	SCBW_DATA_REF(u16*,			map_string_id, unitsDat[51].address);					// 00660260
	SCBW_DATA_REF(u8*,			broodwar_only, unitsDat[52].address);					// 006606D8
	SCBW_DATA_REF(u16*,			staredit_avail_flags, unitsDat[53].address);			// 00661518
} // (last entry at 0x00513EAC)

namespace flingy_dat {
//	SCBW_DATA(const DatLoad*,	flingyDat,	0x00515A38); // now in scbwdata.cpp

	SCBW_DATA_REF(u16*,	sprite_id,			flingyDat[0].address);	// 006CA318
	SCBW_DATA_REF(u32*,	top_speed,			flingyDat[1].address);	// 006C9EF8
	SCBW_DATA_REF(u16*,	acceleration,		flingyDat[2].address);	// 006C9C78
	SCBW_DATA_REF(u32*,	halt_distance,		flingyDat[3].address);	// 006C9930
	SCBW_DATA_REF(u8*,	turn_speed,			flingyDat[4].address);	// 006C9E20 Incorrectly known as "Turn Radius"
//	SCBW_DATA_REF(u8*,	???,				flingyDat[5].address);	// 006CA240
	SCBW_DATA_REF(u8*,	movement_control,	flingyDat[6].address);	// 006C9858
} // (last entry at 0x00515A80)

// incorrect, must be clarified

// Farty sound extender
/*
namespace sfx_dat {
//	SCBW_DATA(const	DatLoad*,	sfxDat,		0x00515498); // now in scbwdata.cpp
	
	SCBW_DATA_REF(u32*,	sound_pointer,		sfxDat[0].address);	// 0068DAA0
	SCBW_DATA_REF(u8*,	priority,			sfxDat[1].address);	// 0068D628
	SCBW_DATA_REF(u8*,	sound_flags,		sfxDat[2].address);	// 0068C8C0
	SCBW_DATA_REF(u16*,	portrait_duration,	sfxDat[3].address);	// 0068CD38
	SCBW_DATA_REF(u8*,	volume,				sfxDat[4].address);	// 0068C448
}
*/

namespace weapons_dat {
//	SCBW_DATA(const DatLoad*,	weaponsDat, 0x00513868);

	SCBW_DATA_REF(u16*,			label,					weaponsDat[0].address);		// 006572E0
	SCBW_DATA_REF(u32*,			flingy_id,				weaponsDat[1].address);		// 00656CA8
	SCBW_DATA_REF(u8*,			special_attack,			weaponsDat[2].address);		// 006573E8
	SCBW_DATA_REF(TargetFlag*,	target_flags,			weaponsDat[3].address);		// 00657998
	SCBW_DATA_REF(u32*,			min_range,				weaponsDat[4].address);		// 00656A18
	SCBW_DATA_REF(u32*,			max_range,				weaponsDat[5].address);		// 00657470
	SCBW_DATA_REF(u8*,			damage_upgrade,			weaponsDat[6].address);		// 006571D0
	SCBW_DATA_REF(u8*,			damage_type,			weaponsDat[7].address);		// 00657258 Normal, Explosive, etc.. Use with damage_type::Enum.
	SCBW_DATA_REF(u8*,			behavior,				weaponsDat[8].address);		// 00656670 Fly and follow target, appear on target unit, etc. Use with WeaponBehavior::Enum.
	SCBW_DATA_REF(u8*,			remove_after,			weaponsDat[9].address);		// 00657040
	SCBW_DATA_REF(u8*,			explosion_type,			weaponsDat[10].address);	// 006566F8 NormalHit, RadialSplash, EnemySplash, etc. Use with WeaponEffect::Enum.
	SCBW_DATA_REF(u16*,			inner_splash_radius,	weaponsDat[11].address);	// 00656888
	SCBW_DATA_REF(u16*,			medium_splash_radius,	weaponsDat[12].address);	// 006570C8
	SCBW_DATA_REF(u16*,			outer_splash_radius,	weaponsDat[13].address);	// 00657780
	SCBW_DATA_REF(u16*,			damage_amount,			weaponsDat[14].address);	// 00656EB0
	SCBW_DATA_REF(u16*,			armor_pen,				weaponsDat[15].address);	// 00657678
	SCBW_DATA_REF(u8*,			cooldown,				weaponsDat[16].address);	// 00656FB8
	SCBW_DATA_REF(u8*,			damage_factor,			weaponsDat[17].address);	// 006564E0
	SCBW_DATA_REF(u8*,			attack_angle,			weaponsDat[18].address);	// 00656990
	SCBW_DATA_REF(u8*,			launch_spin,			weaponsDat[19].address);	// 00657888
	SCBW_DATA_REF(u8*,			forward_offset,			weaponsDat[20].address);	// 00657910
	SCBW_DATA_REF(u8*,			vertical_offset,		weaponsDat[21].address);	// 00656C20
	SCBW_DATA_REF(u16*,			error_message,			weaponsDat[22].address);	// 00656568
	SCBW_DATA_REF(u16*,			weapon_icon,			weaponsDat[23].address);	// 00656780
} //weapons_dat (last entry at 0x0051397C)

namespace upgrades_dat {
//	SCBW_DATA(const DatLoad*,	upgradesDat,	0x005136E0); // now in scbwdata.cpp

	SCBW_DATA_REF(u16*,		mineral_cost_base,			upgradesDat[0].address);	// 00656740
	SCBW_DATA_REF(u16*,		mineral_cost_factor,		upgradesDat[1].address);	// 006559C0
	SCBW_DATA_REF(u16*,		vespene_cost_base,			upgradesDat[2].address);	// 00655840
	SCBW_DATA_REF(u16*,		vespene_cost_factor,		upgradesDat[3].address);	// 006557C0
	SCBW_DATA_REF(u16*,		time_cost_base,				upgradesDat[4].address);	// 00655B80
	SCBW_DATA_REF(u16*,		time_cost_factor,			upgradesDat[5].address);	// 00655940
	SCBW_DATA_REF(u16*,		requirement_offsets,		upgradesDat[6].address);	// 006558C0	// Restriction sound_flags according to EUDDB
	SCBW_DATA_REF(u16*,		upgrade_icon,				upgradesDat[7].address);	// 00655AC0
	SCBW_DATA_REF(u16*,		label,						upgradesDat[8].address);	// 00655A40
	SCBW_DATA_REF(u8*,		race,						upgradesDat[9].address);	// 00655BFC
	SCBW_DATA_REF(u8*,		max_repeats,				upgradesDat[10].address);	// 00655700
//	SCBW_DATA_REF(u8*,		???,						upgradesDat[11].address);	// 00655B3C	// Brood War Flag according to EUDDB
} //upgrades_dat (last entry at 0x00513764)

namespace techdata_dat {
//	SCBW_DATA(const DatLoad*,	techdataDat,	0x005137D8); // now in scbwdata.cpp

	SCBW_DATA_REF(u16*,		mineral_cost,				techdataDat[0].address);	// 00656248
	SCBW_DATA_REF(u16*,		vespene_cost,					techdataDat[1].address);	// 006561F0
	SCBW_DATA_REF(u16*,		time_cost,					techdataDat[2].address);	// 006563D8
	SCBW_DATA_REF(u16*,		energy_cost,				techdataDat[3].address);	// 00656380
//	SCBW_DATA_REF(u16*,		???,						techdataDat[4].address);	// 00656198
	SCBW_DATA_REF(u16*,		requirement_offsets,		techdataDat[5].address);	// 006562F8
	SCBW_DATA_REF(u16*,		tech_icon,					techdataDat[6].address);	// 00656430
	SCBW_DATA_REF(u16*,		label,						techdataDat[7].address);	// 006562A0
//	SCBW_DATA_REF(u8*,		???,						techdataDat[8].address);	// 00656488	// race according to EUDDB
//	SCBW_DATA_REF(u8*,		???,						techdataDat[9].address);	// 00656350	// "Researched" according to EUDDB
//	SCBW_DATA_REF(u8*,		???,						techdataDat[10].address);	// 006564B4	// Brood War Flag according to EUDDB
} // last entry at 0x00513850

namespace orders_dat {
//	SCBW_DATA(const DatLoad*,	ordersDat,	0x00513EC8); // now in scbwdata.cpp

	SCBW_DATA_REF(u16*,		label,						ordersDat[0].address);	// 00665280
	SCBW_DATA_REF(u8*,		use_weapon_targeting,		ordersDat[1].address);	// 00664B00
//	SCBW_DATA_REF(u8*,		???,						ordersDat[2].address);	// 00665940 // May be "secondary order" used for debug/assert only / "Order goes in bSecondaryOrderID instead of bMainOrderID." according to EUDDB
	SCBW_DATA_REF(u8*,		Unused3,					ordersDat[3].address);	// 00665A00 // May be "Non-subunit" used for debug only
//	SCBW_DATA_REF(u8*,		???,						ordersDat[4].address);	// 00664A40
//	SCBW_DATA_REF(u8*,		???,						ordersDat[5].address);	// 006657C0 // May be "Subunit can use" used for debug only
	SCBW_DATA_REF(b8*,		can_be_interrupted,			ordersDat[6].address);	// 00665040
	SCBW_DATA_REF(u8*,		Unknown7,					ordersDat[7].address);	// 00665100
	SCBW_DATA_REF(u8*,		can_be_queued,				ordersDat[8].address);	// 00665700
	SCBW_DATA_REF(u8*,		disabling_keeps_target,		ordersDat[9].address);	// 006651C0
	SCBW_DATA_REF(u8*,		can_be_obstructed,			ordersDat[10].address);	// 006654C0
//	SCBW_DATA_REF(u8*,		???,						ordersDat[11].address);	// 00664C80
//	SCBW_DATA_REF(u8*,		???,						ordersDat[12].address);	// 00664BC0 // May be "Order requires moving"
	SCBW_DATA_REF(u8*,		order_weapon_id,			ordersDat[13].address);	// 00665880
	SCBW_DATA_REF(u8*,		tech_used,					ordersDat[14].address);	// 00664E00
	SCBW_DATA_REF(u8*,		order_animation,			ordersDat[15].address);	// 00664D40 // Animation (IscriptAnimation) according to EUDDB
	SCBW_DATA_REF(s16*,		button_icon,				ordersDat[16].address);	// 00664EC0
	SCBW_DATA_REF(u16*,		req_offset,					ordersDat[17].address);	// 00665580 // based on context
	SCBW_DATA_REF(u8*,		obscured_order,				ordersDat[18].address);	// 00665400
} //orders_dat (last entry at 0x00513FA0)

namespace sprites_dat {
//	SCBW_DATA(const DatLoad*,	spritesDat,		0x00513FB8); // now in scbwdata.cpp

	SCBW_DATA_REF(u16*,		image_id,					spritesDat[0].address);		//00666160
	SCBW_DATA_REF(s8*,		hp_bar_size,				spritesDat[1].address);		//00665E50
//	SCBW_DATA_REF(s8*,		???,						spritesDat[2].address);		//00666570
	SCBW_DATA_REF(u8*,		is_visible,					spritesDat[3].address);		//00665C48
//	SCBW_DATA_REF(s8*,		???,						spritesDat[4].address);		//00665AC0 // "Selection Circle" according to EUDDB, not starting at 0
//	SCBW_DATA_REF(s8*,		???,						spritesDat[5].address);		//00665FD8 // "Selection Vertical Offset" according to EUDDB
} //sprites_dat (last entry at 0x00513FF4)

namespace images_dat {
//	SCBW_DATA(const DatLoad*,	imagesDat,	0x00514010); // now in scbwdata.cpp

	SCBW_DATA_REF(const u32*,		image_file,			imagesDat[0].address);	// 00668AA0
	SCBW_DATA_REF(const u8*,		is_turnable,		imagesDat[1].address);	// 0066E860
	SCBW_DATA_REF(const u8*,		is_clickable,		imagesDat[2].address);	// 0066C150
	SCBW_DATA_REF(const u8*,		use_full_iscript,	imagesDat[3].address);	// 0066D4D8
	SCBW_DATA_REF(const u8*,		draw_if_cloaked,	imagesDat[4].address);	// 00667718
	SCBW_DATA_REF(const u8*,		rle_function,		imagesDat[5].address);	// 00669E28
	SCBW_DATA_REF(const u8*,		remapping,			imagesDat[6].address);	// 00669A40
	SCBW_DATA_REF(const u32*,		iscript_id,			imagesDat[7].address);	// 0066EC48
	SCBW_DATA_REF(const u32*,		lo_shields,			imagesDat[8].address);	// 0066C538
	SCBW_DATA_REF(const u32*,		lo_attack,			imagesDat[9].address);	// 0066B1B0
	SCBW_DATA_REF(const u32*,		lo_damage,			imagesDat[10].address);	// 0066A210
	SCBW_DATA_REF(const u32*,		lo_special,			imagesDat[11].address);	// 00667B00
	SCBW_DATA_REF(const u32*,		lo_landing,			imagesDat[12].address);	// 00666778
	SCBW_DATA_REF(const u32*,		lo_liftoff,			imagesDat[13].address);	// 0066D8C0
} //images_dat (last entry at 0x005140AC)

SCBW_DATA_REF(u8*,	g_is_valid_sprite,			0x00629A88);

SCBW_DATA(u32*,		g_image_overlay_attack,		0x0066B1B0);
SCBW_DATA(u32*,		g_image_overlay_special,	0x00667B00);

//////////////////////////////////////////////////////////////// @}

/// @name Reserved data
/// Warning: The following section contains data reserved for internal use only.
//////////////////////////////////////////////////////////////// @{

SCBW_DATA(u8*, unit_iterator_pos, 0x006284B6);
SCBW_DATA(UnitsSel*, active_player_selection, 0x006284B8);
typedef Unit* (__cdecl* GetActivePlayerNextSelectionFunc)();
SCBW_DATA(GetActivePlayerNextSelectionFunc, get_active_player_next_selection, 0x0049A850);

/// Selections of all players
struct PlayersSel {
	Unit* unit[PLAYABLE_PLAYER_COUNT][SelectionCap];
};
SCBW_DATA(PlayersSel*, players_selections, 0x006284E8);
struct PlayerSel {
	Unit* unit[SelectionCap];
};
SCBW_DATA(const PlayerSel*, current_unit_selection, 0x006284B8);

typedef void(__fastcall* PrepareForNextOrderFunc)(Unit*);
SCBW_DATA(PrepareForNextOrderFunc, prepare_for_next_order, 0x00475000);

// Contains various tile info.
SCBW_DATA(ActiveTile**,		activeTileArray,		0x006D1260);
SCBW_DATA(u16**,			tileIdArray,			0x005993C4);
SCBW_DATA(u16**,			megatileArray,			0x00628494);
//SCBW_DATA(u16**,			megatileExtArrayFlags,	0x00628494+0x20000); // extender only

// Stores an ordered list of all units currently in the game.
SCBW_DATA(UnitFinderData*,	g_unit_ordering_x,		0x0066FF78);
SCBW_DATA(UnitFinderData*,	g_unit_ordering_y,		0x006769B8);
SCBW_DATA(u32*,				g_unit_ordering_count,	0x0066FF74);

SCBW_DATA(Vector32*,		g_angle_distance,		0x00512D28);
SCBW_DATA(u16*,				g_megatile_count,		0x005998E0);

// Used internally by Starcraft to manage temporary units lists
SCBW_DATA(Unit***, g_temp_units_lists_array, 0x006BB938);
SCBW_DATA(u32*, g_temp_units_list_current_array_count, 0x006BEE64);
SCBW_DATA(u32*, g_temp_units_list_arrays_counts_list_last_index, 0x006BEE6C);
SCBW_DATA(u32*, g_temp_units_list_arrays_counts_list, 0x006BEE70);

// Font & Drawing
namespace graphics {
	struct Font; struct Bitmap;
}
SCBW_DATA(graphics::Font**,		font_base,				0x006CE0F4);
SCBW_DATA(graphics::Bitmap*,	g_game_screen_buffer,	0x006CEFF0);
SCBW_DATA(u8*,					g_refresh_regions,		0x006CEFF8); // 640 x 480 divided into 1200 squares of 16x16
SCBW_DATA(Layers*,				g_screen_layers,		0x006CEF50);
typedef void(__stdcall* DrawGameProc)(graphics::Bitmap* surface, Bounds* bounds);
static DrawGameProc const g_old_draw_game_proc = (DrawGameProc)0x004BD580;

// Array used in multiplayer to randomize factions used by unit.get_faction_color() and player.get_faction_color()
// Max index is PLAYABLE_PLAYER_COUNT-1
SCBW_DATA(s32*, factionsColorsOrdering, 0x0057F21C);

// Access to true player colors that differ from standard versions from graphics.h
// This is PlayerColors from BWAPI
// Max index is PLAYER_COUNT-1
SCBW_DATA(u8*, g_player_minimap_colors,	0x00581DD6);
SCBW_DATA(u8*, g_player_scorescreen_colors, 0x0058F442);
SCBW_DATA(u8*, g_player_in_game_colors, 0x00581D76);

const ListExtern<Image, &Image::link> unusedImages(0x0057EB68, 0x0057EB70);
SCBW_DATA(List<Sprite>*, g_free_sprites, 0x0063FE30);

struct SpriteTileData {
	Sprite* tails[256];
	Sprite* heads[256];
};
SCBW_DATA(SpriteTileData*, spritesOnTileRow, 0x00629288);

// Image-related
struct ImagesDatExtraOverlayLO_Files {
	LO_Header** attack_overlays;
	LO_Header** damage_overlays;
	LO_Header** special_overlays;
	LO_Header** landing_dust_overlays;
	LO_Header** liftoff_dust_overlays;
};
SCBW_DATA_REF(ImagesDatExtraOverlayLO_Files*,		lo_files,				0x0051F2A8);
SCBW_DATA_REF(const LO_Header* const*,				g_shield_overlays,		0x0052E5C8);

SCBW_DATA(const ColorShiftData*,					g_color_shift,			0x005128F8); // Use ColorRemapping::Enum as index
SCBW_DATA_REF(GrpHead* const*,						g_image_grp_graphics,	0x0051CED0);

template <typename T>
struct PlayerFlags {
	T flags[PLAYER_COUNT];
};
SCBW_DATA(PlayerFlags<u8>*,		player_alliance,	0x0058D634); // See scbw::is_allied_to()
SCBW_DATA(PlayerFlags<u32>*,	player_vision,		0x0057F1EC);

SCBW_DATA(Unit**,		g_active_portrait_unit,		0x00597248);

SCBW_DATA_REF(AI_Main*,		g_aiscript_controller,		0x0068FEE8);
SCBW_DATA(AiRegion**,		g_ai_region_captains,		0x0069A604);
SCBW_DATA(u32*,				g_ai_supply_reserved,		0x006CA4BC);
SCBW_DATA(AiScript**,		g_first_active_aiscript,	0x0068C100);
SCBW_DATA(AiScriptArray*,	g_aiscript_array,			0x0068C0FC);

SCBW_DATA(ResourceAreaArray*, g_resource_areas,		0x00692688);

//-------- Internal constants --------//

SCBW_DATA(b32*, g_is_game_paused,				0x006509C4);  // See scbw::g_is_game_paused()
SCBW_DATA(b8*,  g_is_brood_war,					0x0058F440);  // See scbw::is_brood_war_mode()
SCBW_DATA(u32*, g_cheat_state,					0x006D5A6C);  // See scbw::is_cheat_enabled()
SCBW_DATA(b32*, g_is_in_replay,					0x006D0F14);  // See scbw::is_in_replay()
SCBW_DATA(b32*, g_is_replay_paused,				0x006D11B0);
SCBW_DATA(s32*, g_max_unit_width,				0x006BEE68);
SCBW_DATA(s32*, g_max_unit_height,				0x006BB930);
SCBW_DATA(b32*, g_is_in_game_loop,				0x006D11C8);
SCBW_DATA(u32*, g_last_random_number,			0x0051CA14);  // See scbw::random(), scbw::rand_between()
SCBW_DATA(b32*, g_can_update_powered_status,	0x0063FF44);
SCBW_DATA(b32*, g_is_placing_building,			0x00640880);

//-------- Tech levels --------//

struct _scTechs {
	u8 is_enabled[PLAYER_COUNT][24];		// 0x0058CE24
	u8 is_researched[PLAYER_COUNT][24];		// 0x0058CF44
};
struct _bwTechs {
	u8* is_enabled[PLAYER_COUNT];			// 0x0058F050
	u8* is_researched[PLAYER_COUNT];			// 0x0058F140
};

// Available and Researched techs are exactly next to each other, that's why with 2 entries structures, only 2 addresses are needed here when BWAPI got 4 of them.
SCBW_DATA(_scTechs*, TechSc, 0x0058CE24); //Use with ScTech::Enum
SCBW_DATA_REF(_bwTechs*, TechBw, 0x0058F050); //Use with BwTech::Enum

//-------- Upgrade levels --------//

struct _scUpgrs {
	u8 max_level[PLAYER_COUNT][46];		// 0x0058D088
	u8 current_level[PLAYER_COUNT][46];	// 0x0058D2B0
};
struct _bwUpgrs {
	u8* max_level[PLAYER_COUNT];			// 0x0058F278
	u8* current_level[PLAYER_COUNT];		// 0x0058F32C
};
SCBW_DATA(_scUpgrs*,		UpgradesSc, 0x0058D088); // Use with ScUpgrade::Enum
SCBW_DATA_REF(_bwUpgrs*,	UpgradesBw, 0x0058F278); // Use with BwUpgrade::Enum

// Requirement opcode lists
namespace requirements
{
	SCBW_DATA_REF(u16*, units,		0x00514178);
	SCBW_DATA_REF(u16*, orders,		0x00514CF8);
	SCBW_DATA_REF(u16*, research,	0x00514908);
	SCBW_DATA_REF(u16*, upgrade,	0x005145C0);
	SCBW_DATA_REF(u16*, tech,		0x00514A48);
}

SCBW_DATA(GuardAiArray*,       g_guard_array,			0x0067D400);
SCBW_DATA(GuardAiListPlayers*, g_player_guard_list,		0x00685108);

SCBW_DATA(u32*,			g_ai_update_cooldown,			0x0059CC94);

// For update_creep_disappearance_unit
SCBW_DATA(u32*,			g_map_creep_width,						0x006D0F08);
SCBW_DATA(u32*,			g_dying_unit_creep_disappearance_count, 0x00658AE4);
SCBW_DATA(u32**,		g_current_tiles,						0x006D0E84);

// For draw_tooltip_with_icon
SCBW_DATA(char*,		grp_stat_res_icon_mapping,	0x00515228); // sizeof 8
SCBW_DATA(GrpHead*,		icons_grp,					0x0068C238);
SCBW_DATA(Box16*,		tooltip_required_area,		0x006CE0D8);

// For do_attack_notify_unit
SCBW_DATA(char*,		g_palette_index_reference,	0x006CEB20); // sizeof at least 20
SCBW_DATA(u32*,			notify_cooldown,			0x00640870);
//SCBW_DATA(SoundData*,	g_sound_data,				0x006D1270); // THIS IS NOT EXTENDED!!!

// For stat_data_unit_wireframe_select_update
SCBW_DATA(char**,	off_68C1FC,						0x0068C1FC);
SCBW_DATA(char**,	default_grp_remap,				0x0050CDC1);

// For kill_units_at_location_proc
SCBW_DATA(u32*,		trig_remove_unit_active,		0x005971DC);
SCBW_DATA(u32*,		trig_killing_additional_unit,	0x00596CD4);
SCBW_DATA(u32*,		trig_kill_unit_count,			0x005971E0);

// For btnsact_rally_point
SCBW_DATA(char*,	g_order_id_ground,				0x00641691);
SCBW_DATA(char*,	g_order_id_obstructed,			0x00641693);
SCBW_DATA(char*,	g_order_id_unit,				0x00641692);
SCBW_DATA(u16*,		orders_buttonset,				0x0068C1C8);

// For select_multiple_units_from_unit_list
SCBW_DATA(char*,	client_selection_changed,		0x0059723C);

// For set_ingame_input_procs
SCBW_DATA(HACCEL*,				common_accelerator,		0x006D120C);
SCBW_DATA(HACCEL*,				accelerator_tables,		0x005968F0);
SCBW_DATA(InputProcsFunc**,		left_click_proc,		0x006556EC);
SCBW_DATA(InputProcsFunc**,		right_click_proc,		0x006556F0);

SCBW_DATA(u16*,					g_minimap_cursor,		0x006556FC);
SCBW_DATA(RECT*,				g_minimap_rect,			0x00512D00);

// For process_right_click_action
SCBW_DATA(u16*,			is_unit_in_point_x_pos,			0x006509CC);
SCBW_DATA(u16*,			is_unit_in_point_y_pos,			0x006509CE);

// For input_game_right_mouse_click
SCBW_DATA(Unit**,		local_selection_group_2,		0x0059724C);

// For init_damage_overlay_counts
SCBW_DATA_REF(u8*,		g_building_overlay_state_max,	0x005240D0);

// To move in-game timer while leaderboard is active
SCBW_DATA(BinDlg**,		leaderboard_dialog,				0x00597284);
SCBW_DATA(char*,		leaderboard_type,				0x0058D70C);

// For begin_gas_building
SCBW_DATA(char*,		unk_sprite_related,				0x006D5BE4);

// For create_all_selection_circles
SCBW_DATA(char**,		game_result_text,				0x00628668);

// For find_fow_target
SCBW_DATA(Thingy**,		first_used_thingy,				0x00654868);

// For play_building_complete_sound
SCBW_DATA(u32*,			building_complete_something,	0x006D1AE8);

// For trigact_transmission
SCBW_DATA(char*,			g_is_executing_wait,		0x006509B8);
SCBW_DATA(TriggerVector**,	g_current_trigger_node,		0x006509AC);
SCBW_DATA(u32*,				g_trigger_wait,				0x00650980);

// For draw_minimap_box
SCBW_DATA(char*,			g_minimap_state,			0x006D5BBE);
SCBW_DATA(u32*,				g_minimap_box_checksum,		0x0059C1A8);
SCBW_DATA(u32*,				g_minimap_draw_neutrals,	0x005993AC);

// For draw_train_progress
SCBW_DATA(GrpHead**,		g_cmdbtns_handle,			0x0068C1C0);

// For draw_all_minimap_boxes
SCBW_DATA(Bitmap**,			g_current_surface,			0x006CF4A8);
SCBW_DATA(Bitmap*,			g_minimap_surface_2,		0x0059C194);
SCBW_DATA(u32*,				g_minimap_unit_box_count,	0x0059C2B8);

// For execute_trigger_conditions
SCBW_DATA(void**,			g_trigger_condition_list,	0x00515A98);

// For play_sound trigger
SCBW_DATA(s32*,				g_big_volume_array,			0x005008F0);

SCBW_DATA(u32*,				g_game_speed_replay_modifiers,	0x005124D8);

// For play_yes_unit_responses
SCBW_DATA(Unit**,			g_place_building_parent_unit,	0x00640874);

// For sprite_to_iscript_loop
SCBW_DATA(u16*,			g_map_tile_height,					0x0057F1D6);
SCBW_DATA(Sprite**,		g_horizontal_sprite_lines_begin,	0x00629688);
SCBW_DATA(Sprite**,		g_horizontal_sprite_lines_end,		0x00629288);

// For get_cursor_type
SCBW_DATA(RECT*,		g_cursor_limit_rect,				0x005993B0);
SCBW_DATA(char*,		g_is_drag_selecting,				0x0066FF5C);

// For stats_panel_display hooks
SCBW_DATA(char**,		g_queues_buffers,					0x00519F40);

// For adding implicit ESC to submenus
SCBW_DATA(u32*,			g_active_portrait_buttonset_id,		0x0068C14C);
SCBW_DATA(u32*,			g_current_buttonset_id,				0x0068C1C4);

// For request_military_to
SCBW_DATA_REF(u32*,		ai_strength_air,					0x006BB210);
SCBW_DATA_REF(u32*,		ai_strength_ground,					0x006BB5A0);

// For ground_attack_init
SCBW_DATA(Thingy**,		g_first_used_thingy_1,				0x00652918);
SCBW_DATA(u8*,			g_can_draw_thingy,					0x00652920);
SCBW_DATA(char**,		g_iscript_data,						0x006D1200);

// For play_warp_in_overlay
SCBW_DATA(UpdateFunction*,	g_sg_update_funcs,				0x00512510);
SCBW_DATA(DrawFunction*,	g_sg_draw_funcs,				0x005125A0);

// For iscript_opcode_cases
SCBW_DATA(char*,		cbullet_damage,						0x0050434C);

// For create_unit_building_sprite_validity_array
SCBW_DATA(u32*,			g_current_visibility_hash,			0x0063FD30);
SCBW_DATA(u32*,			g_want_visibility_hash_update,		0x00629D90);
SCBW_DATA(char*,		g_region_vision_hash,				0x0063FE38);
SCBW_DATA(u32*,			g_current_hash_offset,				0x0063FD28);

SCBW_DATA(ControlGroupPlayerArray*, control_group_array,	0x0057FE60);

// For remove_healthbar
SCBW_DATA(Layer*,		g_game_layer,						0x006CEFB4);
SCBW_DATA(Image**,		g_first_free_healthbar,				0x0057EB6C);
SCBW_DATA(Image**,		g_last_free_healthbar,				0x005254B8);

SCBW_DATA(Image**,		g_first_free_selection_circle,		0x0052E4C0);
SCBW_DATA(Image**,		g_last_free_selection_circle,		0x0052F564);

SCBW_DATA(u16*,			g_temp_iscript_header,				0x006C2310);
SCBW_DATA(u16*,			g_iscript_speed_something,			0x006C2314);

SCBW_DATA(char*,		g_draw_color_index,					0x006CF4AC);

SCBW_DATA(void**,		g_repulse_handle,					0x006D5CD8);

//-------- Format strings used by Starcraft implementations of printf --------//

SCBW_DATA(const char*, FORMATSTRING_TEXT_SPACE_TEXT, 0x00503614); //"%s %s"
SCBW_DATA(const char*, FORMATSTRING_COLORYELLOW_VALUE_COLORCYAN_SPACE, 0x00505708); //"\x03%d\x01 "
SCBW_DATA(const char*, FORMATSTRING_VALUE_SPACE, 0x00505710); //"%d "
SCBW_DATA(const char*, FORMATSTRING_CURRENTVALUE1_MAXVALUE1_COLOR_CURRENTVALUE2_MAXVALUE2, 0x00505714); //"%d/%d %c%d/%d"

//////////////////////////////////////////////////////////////// @}

#undef SCBW_DATA
#undef SCBW_DATA_REF
#pragma pack()