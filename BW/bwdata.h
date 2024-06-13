#pragma once
#include "../definitions.h"
#include "structures.h"
#include "BWhelper.h"
#include "bullet_extender.h"
#include "hooks/limits/ccmu.h"
#include "graphics\Font.h"
#pragma pack(1)

// Contains the definitions of various data structures, arrays, pointers and internal constants in StarCraft.exe.

#define BW_DATA(type, name, offset) type const name = (type)offset;		// Designed as const to make sure programmers don't read from or write to the address itself.
#define BW_DATA_REF(type, name, offset)  extern type& name;				// Contains the definitions of various data structures, arrays, pointers and internal constants in StarCraft.exe.

// TODO: Replace BW_DATA to those new macros (which are harder to misuse and simpler to define)
//       Or potentially make Neiv's style templated classes, like BwOffset<T> and BwArray<T> that would have either T& conversion operator or operator[]
//                                                                                                                                                -Veeq7
#define BW_REF(type, name, offset) inline type& name = *(type*)offset;
#define BW_ARRAY(type, name, offset) type const* name = (type*)offset;

//////////////////////////////////////////////////////////////// @}
/// @name Linked list nodes
//////////////////////////////////////////////////////////////// @{

/// The first nodes in StarCraft's internal linked lists (taken from BWAPI's Offsets.h)
BW_REF(Unit*,			g_first_visible_unit,			0x00628430);
BW_DATA(Unit**,			g_first_hidden_unit,			0x006283EC);	// Includes units in transports/bunkers, worker inside gas buildings, nukes in silos, and units in production.
BW_DATA(Unit**,			g_last_hidden_unit,				0x00628428);	// Same as above.
BW_DATA(Bullet**,		g_first_active_bullet,			0x0064DEC4);
BW_DATA(Unit**,			g_first_scanner_sweep,			0x006283F4);
BW_DATA(Unit**,			g_last_scanner_sweep,			0x00628434);
BW_DATA(Unit**,			first_psi_field_provider,		0x0063FF54);
BW_DATA(u32*,			psi_field_grp_width,			0x0063FF4C);
BW_DATA(u32*,			psi_field_grp_height,			0x0063FF58);
BW_DATA(u8**,			psi_field_mask,					0x006D5BD8);
BW_DATA(u32*,			g_psi_aura_is_drawn,			0x0063FF48);
BW_DATA(u32*,			g_player_visions,				0x0057F0B0);
BW_DATA(Bullet**,		first_bullet,					0x0064DEC4);
BW_DATA(Bullet**,		last_bullet,					0x0064DEAC);
BW_DATA(Bullet**,		first_free_bullet,				0x0064EED8);
BW_DATA(Bullet**,		last_free_bullet,				0x0064EEDC);
BW_DATA(u32*,			bullet_count,					0x0064DEBC);

struct Units12 {
	Unit* unit[12];
};
BW_DATA(Units12*, first_player_unit, 0x006283F8); // Indexed by player Id

//////////////////////////////////////////////////////////////// @}
/// @name Structure tables
//////////////////////////////////////////////////////////////// @{

BW_DATA(Units12*,			g_completed_factory_counts,		0x0068A9D8);
BW_DATA(Units12*,			g_trigger_factory_counts,		0x0068A9A8);

BW_DATA(Thingy*,			g_thingy_table,					ccmu_extender.thingy_table); // lone sprites, not fow sprites!

BW_DATA(Unit*,			g_unit_table,					ccmu_extender.unit_table);
BW_DATA(Order*,			g_order_table,					ccmu_extender.order_table);
BW_DATA(u32*,				g_unit_count,					0x006283F0);
BW_DATA(Bullet*,			g_bullet_table,					new_bullet_table);				// previously 0064B2E8
BW_DATA_REF(UnitDeaths*,	g_unit_deaths_table,			0x0058A364);
BW_DATA(Sprite*,			g_sprite_table,					ccmu_extender.sprite_table);	// previously 00629D98
BW_DATA(u32*,				g_sprite_count,					0x006C4A2C);
BW_DATA(Sprite**,			g_sprite_refs,					ccmu_extender.sprite_list_pointers);
BW_DATA(Image*,			g_image_table,					ccmu_extender.image_table);
BW_DATA_REF(UnitDeaths*,	g_unit_killed_table,			0x005878A4);

BW_DATA_REF(UnitDeaths*,	g_completed_unit_count_table,	0x00584DE4);

BW_DATA_REF(UnitDeaths*,	g_all_unit_count_table,			0x00582324);
BW_DATA(UnitDeaths*,		g_all_unit_count_table_original,0x00582324);

BW_DATA(Thingy**,			g_first_thingy,					0x00654874);
BW_DATA(Thingy**,			g_last_thingy,					0x0065291C);

// From locations.cpp
BW_DATA(MapSize*,			g_map_tile_size,				0x0057F1D4);
BW_DATA(Location*,		g_location_table,				0x0058DC60);

// For buttonsets
BW_DATA_REF(ButtonSet*,	g_buttonset_table,				0x005187E8);
BW_DATA_REF(UnitStatFuncArrayEntry*, unitStatFuncs,		0x005193A0);

BW_DATA(ActionPointer*,	g_action_table,					0x00512800); // From triggers.cpp
BW_DATA(ActionPointer*,	g_briefing_action_table,		0x00512800);
BW_DATA(char**,			g_chk_string_table,				0x005993D4);
BW_DATA(HWND*,			g_main_window,					0x0051BFB0);
BW_REF(s32,				g_music_volume,					0x006D5BB4);
BW_REF(b8,				g_music_playing,				0x006D5BBC);

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
BW_DATA(Resources*, resources, 0x0057F0F0);

BW_DATA(cv5**, tileGroup, 0x006D5EC8);

/// TBL files
BW_DATA(const u16*,		g_stat_txt_tbl,		0x006D1238);
BW_DATA(const StringTbl*, g_map_string_tbl,	0x005993D4);

BW_DATA(const char*,		g_string_empty,		0x00501B7D);

/// Units that are selected by the current player (or the player viewing the replay).
struct UnitsSel {
	Unit* unit[SelectionCap];
};
BW_DATA(UnitsSel*,		g_client_selection_group,		ccmu_extender.new_client_selection_group);
BW_DATA(Unit*,			g_client_selection_group_end,	0x00597238);
BW_DATA(u8*,				g_client_selection_count,		0x0059723D);
BW_DATA(Point16*,			g_selection_start,				0x0066FF4C);
BW_DATA(Box16*,			g_selection_box,				0x0066FF50);

// Screen position relative to the map
BW_DATA(s32*,				g_screen_x,				0x00628448);
BW_DATA(s32*,				g_screen_y,				0x00628470);

// To use with a player ID
BW_DATA(Point16*,			g_start_positions,		0x0058D720);

// Mouse position relative to the screen
BW_DATA(const Vector32*,	g_mouse,				0x006CDDC4);

BW_DATA(u32*,				g_mmb_pos_x,            0x006556F8);
BW_DATA(u32*,				g_mmb_pos_y,            0x006556F4);

// Booleans indicating which keyboard buttons are held
// use g_is_keycode_used[VK_param] where VK_param is one of the Virtual-Key Codes of Windows as seen in msdn here: https://msdn.microsoft.com/fr-fr/library/windows/desktop/dd375731%28v=vs.85%29.aspx
BW_DATA(const b8*, g_is_keycode_used,		0x00596A18);
BW_DATA(const b8*, g_is_holding_shift,	0x00596A28);
BW_DATA(const b8*, g_is_holding_ctrl,		0x00596A29);
BW_DATA(const b8*, g_is_holding_alt,		0x00596A2A);

// Maybe used by Map triggers to init a progressive movement toward the corresponding location, see GameImpl::setScreenPosition of BWAPI
BW_DATA(u32*,		g_move_to_x,			0x0062848C);
BW_DATA(u32*,		g_move_to_y,			0x006284A8);

// Contains various information (names, player types, race types, and associated forces) of each player in the current game
BW_DATA(Player*,	g_player_table,			0x0057EEE0);

struct SupplyData {
	u32 provided[PLAYER_COUNT];
	u32 used[PLAYER_COUNT];
	u32 max[PLAYER_COUNT];
};
BW_DATA(SupplyData*,	g_race_supply,			0x00582144);	// Array; Use Unit::get_race() to get the index.

BW_REF(u32,			g_elapsed_time_frames,		0x0057F23C);
BW_DATA(u32*,		g_countdown_timer,			0x0058D6F4);	// Vanilla trigger actions/conditions (From BWAPI Offsets.h)
BW_DATA(u32*,		g_elapsed_time_seconds,		0x0058D6F8);
BW_DATA(u8*,		g_game_type,				0x00596820);	// Compare with GameType::Enum
BW_DATA(s32*,		g_active_nation_id,			0x00512678);
BW_DATA(s32*,		g_active_player_id,			0x0051267C);	// Used by get_active_player_next_selection
BW_DATA(s32*,		g_local_nation_id,			0x00512684);	// Actually stores the player ID, shared in team games
BW_DATA(s32*,		g_local_human_id,			0x00512688);	// Invalid in replays, unique even in team games
BW_DATA(u8*,		g_current_tileset,			0x00596828);	// Tileset of current map - compare with TilesetType::Enum
BW_DATA(s32*,		g_provider_max_buffer,		0x0057F0D8);
BW_DATA(u32*,		g_provider_latency_calls,	0x0057F090);
BW_DATA(u32*,		g_command_queue_size,		0x00654AA0);
BW_DATA(char*,		g_turn_buffer,				0x00654880);

// For reset_tooltip_info, set_tooltip_info, set_tooltip_info_unit_case5, etc
BW_DATA(u16*,		tooltip_ore_cost,			0x0068C1D0);
BW_DATA(u16*,		tooltip_gas_cost,			0x0068C1D2);
BW_DATA(u16*,		tooltip_energy_cost,		0x0068C1D4); // Used to be Protoss energy
BW_DATA(u16*,		tooltip_unused_cost,		0x0068C1D6); // Used to be Terran energy
BW_DATA(u16*,		tooltip_time_cost,			0x0068C1D8); // Used to be Zerg energy
BW_DATA(u16*,		tooltip_p_supply_cost,		0x0068C1DA);
BW_DATA(u16*,		tooltip_t_supply_cost,		0x0068C1DC);
BW_DATA(u16*,		tooltip_z_supply_cost,		0x0068C1DE);

struct Event;
typedef bool __fastcall generic_interact_function(BinDlg*, Event*);
BW_DATA(generic_interact_function**, generic_interact_functions, 0x005014AC);

// Added from BWAPI Offsets.h
BW_DATA(char*,		g_current_map_filename,	0x0057FD3C);
BW_DATA(char*,		g_current_map_name,		0x0057FE40);
BW_DATA_REF(_uavail*, g_unit_availability,	0x0057F27C);

// Ping data
BW_DATA(Ping*,			ping_array,					0x0059C1B8); // 16
BW_DATA(PingArray*,		minimap_ping_array,			0x0059C1B8); // 16

	// -- Added for various hooks used by Cosmonarchy -- \\

// For race_dropdown
BW_DATA(StringMap_b*,	single_race_select,			0x0051AAA0);
BW_DATA(StringMap_w*,	single_type_select,			0x0051AAA8);
BW_DATA(u16*,			g_network_table,			0x006D1220);
BW_DATA(StringTbl*,		g_network_table_df,			0x006D1220);

// For init_singleplayer_custom
BW_DATA(u32*,			g_selected_game_type,			0x0059BA64);
BW_DATA(u16*,			g_selected_game_type_param,		0x0059BA5C);
BW_DATA(char*,			g_selected_singleplayer_race,	0x0057F266);
BW_DATA(char*,			g_custom_singleplayer,			0x0057F1E3);

// For Lobotomy Mine
BW_DATA(char**,			g_status_screen_info_button_unk,	0x00519F40);
BW_DATA(GrpHead**,		g_cmdicons_grp,						0x0068C1E0);

// For init_game_load
BW_DATA(void*,			scenario_chk,				0x006D0F24);
BW_DATA(ForceFlags*,	g_player_force_flags,		0x0058D5B8); // array of 0x4
BW_DATA(u8*,			g_player_force,				0x0059BDA8); // array of 0x8
BW_DATA(u8*,			g_player_force_replay_sth,	0x006D11A1); // array of 0x8
BW_DATA(FILE*,			load_game_file_handle,		0x006D1218);
BW_DATA(u8*,			g_multiplayer_mode,			0x0057F0B4);
BW_DATA(u32*,			g_initial_random_seed,		0x0051CA10);
BW_DATA(u32*,			g_replay_random_seed,		0x006D0F38);
BW_DATA(u32*,			g_random_counts,			0x0051C610); // count of 0x400
BW_DATA(u32*,			g_total_random_counts,		0x0051CA18);
BW_DATA(u16*,			campaign_index,				0x0057F244);
BW_DATA(void**,			map_archive_handle,			0x00596BC4);
BW_DATA(GameData*,		game_data,					0x005967F8);
BW_DATA(u8*,			map_started,				0x006D121C);
BW_DATA(u8*,			player_has_left,			0x00581D62); // array of 0x8
BW_DATA(u8*,			loss_type,					0x00581D61);
BW_DATA(u8*,			unk_victory_variable,		0x00581D66);
BW_DATA(u32*,			save_load_success,			0x006D5A68);
BW_DATA(u32*,			g_elapsed_time_modified,	0x006D5BCC);
BW_DATA(u32*,			saved_elapsed_seconds,		0x0057F240);
BW_DATA(Options*,		g_options_list,				0x006CDFD4);
BW_DATA(ReplayData**,	g_replay_data,				0x00596BBC);
BW_DATA(u32*,			g_turn_command_size,		0x005005F8);
BW_DATA(u32*,			g_replay_visions,			0x006D0F18);
BW_DATA(u32*,			player_visions,				0x0057F0B0);
BW_DATA(u32*,			g_replay_reveal_all,		0x006D0F1C);
BW_DATA(u32*,			next_replay_command_frame,	0x005136C8);
BW_DATA(u32*,			g_player_explored_visions,	0x0057EEB8);
BW_DATA(s32*,			sub_two_check_two,			0x004EE0F0);

enum MinimapColor {
	MinimapColor_Unexplored,
	MinimapColor_Explored,
	MinimapColor_Visible,
	MinimapColor_Visible2,

	MinimapColor_Count,
};

struct MinimapColors {
	u8 index[MinimapColor_Count][256];
};
BW_DATA(MinimapColors*,	minimap_colors, 0x0059C2C0);

struct DarkMap {
	u8 index[32][256];
};
BW_DATA(DarkMap*,			dark_pcx_image_map, 0x005973A0);
BW_DATA(u8*,				palette_index_ref,  0x006CEB20);

// For get_response_sfx
BW_DATA(u32*,			selection_sound_counter,	0x00640548);
BW_DATA(u32*,			g_last_random_sfx_id,		0x006D5BE0);

// For load_race_ui
BW_DATA(u8*,			g_console_index,				0x0057F1E2);
BW_DATA(u16*,			g_current_music_id,				0x0057F1DE);
BW_DATA(char*,		g_campaign_first_mission_bw,	0x005122C0);
BW_DATA(char*,		g_campaign_first_mission,		0x005122BC);
BW_DATA(u32*,			g_console_race_specific,		0x0051A3B4);

// For load_game_create
BW_DATA(u32*,			is_host,					0x00596888);
BW_DATA(u8*,			out_of_game,				0x006D5A10);
BW_DATA(u32*,			sc_main_state,				0x00596904);
BW_DATA(u8*,			g_game_active,				0x006D11EC);
BW_DATA(u16*,			next_sc_main_state,			0x0051CE90);
BW_DATA(u32*,			replay_end_frame,			0x006D0F31);

// For level_cheat_create
BW_DATA(u32*,			g_ophelia_enabled,			0x006D11C0);
BW_DATA(char*,		player_name,				0x0057EEB5);

// For update_placement_state_for_placement_box
BW_DATA(u8*,		g_placing_player,				0x0064088C);
BW_DATA(u16*,		place_building_x,				0x00640890);
BW_DATA(u16*,		place_building_y,				0x00640892);
BW_DATA(u32*,		g_is_placing_addon,				0x00640884);
BW_DATA(u16*,		g_place_building_unit_id,		0x0064088A);
BW_DATA(u32*,		g_place_building_base_type,		0x00640888);

BW_DATA(u16*,		g_move_to_tile_x,					0x0057F1D0);
BW_DATA(u16*,		g_move_to_tile_y,					0x0057F1D2);

// For increment_unit_scores_ex
BW_DATA(u32*,		factories_owned,				0x005820B4); // Array of 12	btw
BW_DATA(u32*,		units_produced,					0x00581E14); // Array of 12	btw
BW_DATA(u32*,		structures_built,				0x00581F64); // Array of 12	btw
BW_DATA(u32*,		unit_score,						0x00581ED4); // Array of 12	btw
BW_DATA(u32*,		building_score,					0x00582024); // Array of 12	btw
BW_DATA(u32*,		units_owned,					0x00581E44); // Array of 12	btw
BW_DATA(u32*,		buildings_owned,				0x00581F94); // Array of 12	btw

// For increment_unit_scores
BW_DATA(u32*,		factories_produced_owned_total,	0x00582084); // Array of 12	btw
BW_DATA(u32*,		structures_produced_owned_total,0x00581F34); // Array of 12	btw
BW_DATA(u32*,		units_produced_owned_total,		0x00581DE4); // Array of 12	btw

// For refresh_screen
BW_DATA(u32*,		g_place_building_message_id,	0x00640958);

// For targeted_order
BW_DATA(u32*,		g_order_count,					0x00641698);

// For create_upgrade_level_tooltip
BW_DATA(u16*,			g_upgrade_level_tooltip,	0x0068C1CC);
BW_DATA(StringTbl*,	g_stat_txt_buffer,			0x006D1238);

struct BufferStr {
	char buf[260];
};

// For draw_unit_info
BW_DATA(u16*,			g_supplies_available_temp,	0x006CA93C);
BW_DATA(u8*,			g_status_screen_func,		0x0068C1E5);
BW_DATA(BufferStr*,	g_struct_char,				0x006CA9F8);

// For draw_supply_unit_info
BW_DATA(BufferStr*,	g_struct_char_2,			0x006CAB08);
BW_DATA(char*,		g_unk_string_char,			0x006CA668);

// For draw_wireframe_shield
BW_DATA(char*,		g_char_buffer,				0x0068C208);
BW_DATA(char*,		g_byte_shield_string,		0x0068C20A); // count of 22
BW_DATA(u8*,			g_wireframe_constants,		0x00515280); // count of 40
BW_DATA(char**,		g_image_draw_palette_index,	0x0050CEC1); // count of 256
BW_DATA(u8*,			off_50CE81,					0x0050CE81);
BW_DATA(u8*,			off_50CE82,					0x0050CE82);

// For draw_building_info
BW_DATA(s32*,			g_selection_group_hp,		ccmu_extender.new_selGroupHp);
BW_DATA(s16*,			g_ui_energy,				0x006CAC0C);
BW_DATA(s32*,			g_ui_shields,				0x006CA9EC);
BW_DATA(char*,		g_redraw_transport_ui,		0x006CA9F0);

// For AI_EMPShieldRequirementsProc
BW_DATA(s32*,			g_temp_hp_shields,			0x00695798);
BW_DATA(s32*,			g_temp_weapon,				0x00695604);

// For draw_image_hp_bar
BW_DATA(unk_61*,		struct_5140C4,				0x005140C4);
BW_DATA(unk_61*,		struct_5140D8,				0x005140D8);
BW_DATA(unk_62*,		struct_5140E8,				0x005140E8);

// For get_unit_bullet_damage
BW_DATA(u8*,		g_upgrades_level_sc,		0x0058D2B0);
BW_DATA(u8*,		g_upgrades_level_bw,		0x0058F2FE);

// For find_next_bounce_target_proc
BW_DATA(Unit**,		g_bullet_bounce_missile_source,	0x0064EECC);
BW_DATA(Unit**,		g_bullet_bounce_missile_target,	0x0064EED0);
BW_DATA(Unit**,		g_bullet_bounce_missile_next,	0x0064EED4);

// For start_game
BW_DATA(s8*,		g_game_state,					0x0066FBFA); // 0x4 Idling in lobby | 0x5 Game starting | 0x6 Slots locked | 0x7 Sending final commands | 0x8 Waiting for save data | 0x9 End lobby
BW_DATA(Player*,	g_player_replay_watchers,		0x00640970);

BW_DATA(s8*,		g_countdown_timer_interval,		0x0066FBFC);
BW_DATA(s32*,		g_countdown_timer_tick_count,	0x0059CC7C);
BW_DATA(s32*,		g_countdown_timer_tick_count_2,	0x0059CC88);
BW_DATA(s32*,		g_is_in_game,					0x006D5BC8);
BW_DATA(s32*,		g_countdown_time_remaining,		0x0059CC78);

// For air_splash
BW_DATA(Unit**,		g_air_splash_list,				ccmu_extender.air_splash_list);
BW_REF(s32,			g_air_splash_list_count,		0x0046DEB4);
BW_REF(s32,			g_is_in_ground_splash_proc,		0x0064DEB8);

// For ai_add_military
BW_DATA(RegionArray**,		g_sai_paths,				0x006D5BFC);
BW_DATA(AiTown*,			g_ai_town_array,			0x006AA090);
BW_DATA(AiTownList*,		g_ai_town_list,				0x006AA050); // sizeof 0x64[0x8]

// For cmdrecv_hold_position
BW_DATA(s16*,		g_map_width_pixels,					0x00628450);
BW_DATA(s16*,		g_map_height_pixels,				0x006284B4);
BW_DATA(u32*,		g_order_type_4_position,			0x006D5C20);
BW_DATA(u32*,		g_order_type_4_target,				0x006D5C24);

// For openMapFile
BW_DATA(char*,		g_maps_folder,						0x0066FC08);
BW_DATA(s32*,		dword_5994DC,						0x005994DC);

// For loadMap
BW_DATA(ScenarioFormat*, stru_500574,					0x00500574);
BW_DATA(ScenarioFormat*, stru_50057C,					0x0050057C);
BW_DATA(s32*,		scenarioCHKSize,					0x006D0F20);

// For game_loop
BW_DATA(s32*,		visionUpdateCount,					0x0051CE98);
BW_DATA(s8*,		g_vision_updated,					0x0051CE9C);

// X405: For displayEstablishing Shot
BW_DATA(CampaignMission*, campaignMissions, 0x004FFF60);

// For stats_weapon_level / stats_armor_level
BW_DATA(s16*, g_unk_upgrade_count,			0x006CA940); // Seems to be unused. Only ever written to.

// For isUnitCloaked
BW_DATA(Unit**,		g_first_invisible_unit,			0x0063FF5C);
BW_REF(s32,			g_force_button_refresh,			0x0068C1B0);
BW_REF(s8,			g_portrait_redraw,				0x0068AC74); // sizeof 4
BW_REF(s8,			g_status_screen_redraw,			0x0068C1F8);
BW_REF(BinDlg*,		g_control_under_mouse,			0x0068C1E8);
BW_REF(BinDlg*,		g_control_under_mouse_value,	0x0068C1EC); // void* in Neiv's list

// For update_units
BW_DATA(s32*,		g_interceptor_focused_this_frame,		0x006957D0);
BW_REF(b8,			g_is_targeting,							0x00641694);
BW_DATA(s32*,		g_order_timer_reassign_count,			0x0059CCA4);
BW_DATA(s32*,		g_secondary_order_timer_reassign_count,	0x006283E8);
BW_DATA(Unit**,		g_active_iscript_entity,				0x006D11F4);
BW_DATA(Bullet**,	g_active_iscript_bullet,				0x006D11F8);
BW_DATA(Unit**,		g_active_iscript_unit,					0x006D11FC);
BW_DATA(Unit**,		g_first_dying_unit,						0x0062842C);
BW_DATA(Unit**,		g_lurker_splash_list,					0x0064DEC8); // sizeof 400
BW_DATA(s32*,		g_lurker_splash_list_index,				0x0064EEC8);
BW_DATA(s32*,		g_last_bullet_index,					0x0064DEA8);

// For order_allowed
BW_DATA(s32*,		g_last_internal_error,			0x0066FF60);
BW_DATA_REF(u16*,	g_orders_requirement_tree,		0x00514CF8);

// For larva_creep_spawn
BW_DATA(s16*,		g_larva_counter,				0x0067D3F8);

// For update_building_placement_state_units
BW_DATA(s8*,		g_building_placement_state,		0x006408F8);

// For progress_unit_frame
BW_DATA(s8*,		g_selection_circle_color,		0x00581D6A);

// For initializeBulet
BW_DATA(Unit**,	g_last_bullet_creator,			0x0064DEB0);
BW_DATA(s32*,		g_launch_spin_random,			0x0064DEC0);
BW_DATA(Point16*, g_attack_curve,					0x00513650);

// For ai_progress_build
BW_DATA(s32*,		g_game_minerals,				0x0057F0F0);
BW_DATA(s32*,		g_total_minerals,				0x0057F180);
BW_DATA(s32*,		g_game_vespene,					0x0057F120);
BW_DATA(s32*,		g_total_vespene,				0x0057F150);
BW_DATA(s32*,		g_ai_minerals_reserve,			0x006CA51C);
BW_DATA(s32*,		g_ai_vespene_reserve,			0x006CA4EC);

// For updateBriefingRoomObjectives
BW_DATA(BinDlg**, g_ready_room_dialog,				0x0066FF6C);
BW_DATA(s32*,		g_mission_objectives_string_index,	0x0058D6C4);

// For ai_manage_building_placement
BW_DATA(Unit**,	g_disabled_unit_array,			0x00692660);
BW_DATA(Unit**,	g_ai_builder_unk,				0x006D5DCC);

// For get_supplies_available
BW_DATA(s32*,		g_supplies_available_protoss,	0x00582264);
BW_DATA(s32*,		g_supplies_max_protoss,			0x005822C4);
BW_DATA(s32*,		g_supplies_available_terran,	0x005821D4);
BW_DATA(s32*,		g_supplies_max_terran,			0x00582234);
BW_DATA(s32*,		g_supplies_available_zerg,		0x00582144);
BW_DATA(s32*,		g_supplies_max_zerg,			0x005821A4);

// For get_supplies_used_no_args
BW_DATA(s32*,		g_supplies_used_protoss,		0x00582294);
BW_DATA(s32*,		g_supplies_used_terran,			0x00582204);
BW_DATA(s32*,		g_supplies_used_zerg,			0x00582174);

// For initialize_unit_counts
BW_DATA(s32*,		g_unit_count_score,				0x00581DE4);
BW_DATA(s8*,		unknown_57F41CB,				0x0057F1CB);

// For stat_res_main_proc
BW_DATA(s32*,		unknown_player_5153F8,			0x005153F8);
BW_DATA(u32*,		unknown_68C22C,					0x0068C22C);
BW_DATA(u32*,		unknown_68C230,					0x0068C230);

// For get_supplies_temp
BW_DATA(s16*,		g_supplies_used_temp,			0x006CA93E);

// For draw_supply_unit_status
BW_DATA(char*,	g_supply_available_buffer,		0x006CA668);
BW_DATA(char*,	g_supply_used_buffer,			0x006CA9F8);
BW_DATA(char*,	g_supply_max_buffer,			0x006CAB08);
BW_DATA(char*,	g_supply_provided_buffer,		0x006CAC78);

BW_DATA(char*,	g_kills_buffer,					0x006CAC78);

// For give_unit_to_player
BW_DATA(s32*,		g_current_trigger_player_id,	0x006509B0);

// For delete_ai_town_if_needed
BW_DATA(ResourceArea*, g_base_array,				0x00692688);

// For get_bunker_placement_position
BW_DATA(u32*,		g_region_ids_count,				0x0069265C);
BW_DATA(u16*,		g_region_ids,					0x0069262C);

BW_DATA(u8*,		unk_order_array,				0x0068C300);

BW_DATA(u32**,	g_active_tile_array_int,		0x006D1260);

BW_DATA(void**,	g_get_right_click_order_array,	0x005153FC);

// For prepare_unit_move_custom
BW_DATA(BinDlg**,	g_portrait,						0x0068AC98);

// For fog of war changes by DarkenedFantasies
BW_DATA(FogMask*,	g_fog_of_war_mask,				0x00657AA0);

// For score screen player name overrides, from DarkenedFantasies
BW_DATA(const StringTbl*,	g_network_table_DF,		0x006D1220);
BW_DATA(PlayerNames*,		g_player_name_list,		0x0059B3D8);

// For soundThread
BW_DATA(HANDLE*,		g_sound_thread_event,		0x006D5A00);
BW_DATA(s32*,			g_unk_sound_core,			0x006D5A08);
BW_DATA(s32*,			g_sound_volume,				0x00599900);
BW_DATA(s32*,			g_volume_reduction,			0x006D5A0C);
BW_DATA(DSound*,		g_direct_sound_interface,	0x006D59F4);
BW_DATA(s32*,			g_unk_sound_core_2,			0x005999C0);
BW_DATA(CritSect*,	g_sound_lock,				0x006D5F4C);

// For soundInit
BW_DATA(SoundChannel*,g_sound_channels,			0x005998F0);
BW_DATA(HMODULE*,		g_hd_sound_module,			0x006D59F0);
BW_DATA(s8*,			g_dda_initialized,			0x006D1265);
BW_DATA(s8*,			g_vid_initialized,			0x006D1266);
BW_DATA(s32*,			g_sound_memory_cache,		0x005129B8);
BW_DATA(HANDLE*,		g_sound_handle,				0x006D5A04);
BW_DATA(s32*,			g_unk_sound_core_3,			0x005998E4);
BW_DATA(s8*,			g_unk_sound_bool_core,		0x006D1264);
BW_DATA(char**,		g_current_sound_memory,		0x006D59FC);

BW_DATA(void*,		g_sfx_tbl_handle,			0x005999B0);

// For handle_window_messages
typedef void __fastcall InputProcsFunc(s32 arg);
struct InputProcsArray {
	InputProcsFunc* funcs[19];
};
BW_DATA(InputProcsArray*,	g_input_procs,		0x005968A0);

// For get_standard_attack_target
BW_DATA(u32*,		g_min_range,				0x006955B8); // sizeof 10
BW_DATA(s32*,		g_max_range_in_px,			0x006957A4);

// For show_right_click_error_if_needed
BW_DATA(u32*,		g_use_quiet_sounds,			0x0064086C);
BW_DATA(char*,	draw_out_text_left,			0x00641598);
BW_DATA(char*,	text_bytes,					0x00641674); // sizeof 13
BW_DATA(s32*,		unit_stat_update_timer,		0x00640B54);

// For cmdact_game_speed
BW_DATA(s32*,		last_control_index,			0x006D051C);
BW_DATA(b32*,		cpu_throttle,				0x006D5C00);
BW_DATA(b32*,		wants_redraw,				0x005968EC);
BW_DATA(s32*,		temp_cpu_throttle,			0x00655C3C);
BW_DATA(char*,	bin_interact,				0x006D1224);
BW_DATA(void**,	last_bindlg_menu_function,	0x006D1234);

// For statport_dlg_interact
BW_DATA(s8**,		g_dialog_key,				0x0068AC88);
BW_DATA(Point32*,	g_last_event_position,		0x0057FD34);

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
//	BW_DATA(const DatLoad*,	unitsDat,	0x00513C30); // now in bwdata.cpp

	BW_DATA_REF(u16*,			graphic,				unitsDat[0].address);	// 006644F8
	BW_DATA_REF(u16*,			subunit,				unitsDat[1].address);	// 006607C0
	BW_DATA_REF(u16*,			subunit_2,				unitsDat[2].address);	// 00660C38
	BW_DATA_REF(u16*,			infested_unit_partial,	unitsDat[3].address);	// 00664980 // (Id - UnitId::TerranMinistry) for it to work, last valid id is UnitId::Special_OvermindCocoon by default
	BW_DATA_REF(u32*,			construction_graphic,	unitsDat[4].address);	// 006610B0
	BW_DATA_REF(u8*,			start_direction,		unitsDat[5].address);	// 006605F0 // Direction unit will face after it is created. From 0 (top) to 31 (before top turning clockwise). 32 means random
	BW_DATA_REF(u8*,			shields_enabled,		unitsDat[6].address);	// 006647B0
	BW_DATA_REF(u16*,			max_shield_points,		unitsDat[7].address);	// 00660E00
	BW_DATA_REF(s32*,			max_hitpoints,			unitsDat[8].address);	// 00662350
	BW_DATA_REF(u8*,			elevation,				unitsDat[9].address);	// 00663150
	BW_DATA_REF(u8*,			movement_flags,			unitsDat[10].address);	// 00660FC8
	BW_DATA_REF(u8*,			rank,					unitsDat[11].address);	// 00663DD0
	BW_DATA_REF(u8*,			computer_idle_order,	unitsDat[12].address);	// 00662EA0
	BW_DATA_REF(u8*,			human_idle_order,		unitsDat[13].address);	// 00662268
	BW_DATA_REF(u8*,			return_to_idle_order,	unitsDat[14].address);	// 00664898
	BW_DATA_REF(u8*,			attack_unit_order,		unitsDat[15].address);	// 00663320
	BW_DATA_REF(u8*,			attack_move_order,		unitsDat[16].address);	// 00663A50
	BW_DATA_REF(u8*,			ground_weapon,			unitsDat[17].address);	// 006636B8
	BW_DATA_REF(u8*,			max_ground_hits,		unitsDat[18].address);	// 006645E0
	BW_DATA_REF(u8*,			air_weapon,				unitsDat[19].address);	// 006616E0
	BW_DATA_REF(u8*,			max_air_hits,			unitsDat[20].address);	// 0065FC18
	BW_DATA_REF(u8*,			computer_ai_internal,	unitsDat[21].address);	// 00660178
	BW_DATA_REF(u32*,			base_property,			unitsDat[22].address);	// 00664080 // AKA SpecialAbilityFlags, UnitProperty, etc. Use with UnitProperty::Enum.
	BW_DATA_REF(u8*,			seek_range,				unitsDat[23].address);	// 00662DB8 // AKA Target Acquisition Range, Acquire Target Range, etc.
	BW_DATA_REF(u8*,			sight_range,			unitsDat[24].address);	// 00663238
	BW_DATA_REF(u8*,			armor_upgrade,			unitsDat[25].address);	// 006635D0
	BW_DATA_REF(u8*,			size_type,				unitsDat[26].address);	// 00662180
	BW_DATA_REF(u8*,			armor_amount,			unitsDat[27].address);	// 0065FEC8
	BW_DATA_REF(u8*,			right_click_action,		unitsDat[28].address);	// 00662098
	BW_DATA_REF(u16*,			sound_ready,			unitsDat[29].address);	// 00661FC0
	BW_DATA_REF(u16*,			sound_what_first,		unitsDat[30].address);	// 0065FFB0
	BW_DATA_REF(u16*,			sound_what_last,		unitsDat[31].address);	// 00662BF0
	BW_DATA_REF(u16*,			sound_annoyed_first,	unitsDat[32].address);	// 00663B38
	BW_DATA_REF(u16*,			sound_annoyed_last,		unitsDat[33].address);	// 00661EE8
	BW_DATA_REF(u16*,			sound_yes_first,		unitsDat[34].address);	// 00663C10
	BW_DATA_REF(u16*,			sound_yes_last,			unitsDat[35].address);	// 00661440
	BW_DATA_REF(Point16*,		building_dimensions,	unitsDat[36].address);	// 00662860	// Placement Box in PyDAT
	BW_DATA_REF(Point16*,		addon_placement,		unitsDat[37].address);	// 006626E0 // Addon Position in PyDAT
	BW_DATA_REF(Box16*,		unit_bounds,			unitsDat[38].address);	// 006617C8	// Unit Dimensions in PyDAT
	BW_DATA_REF(u16*,			portrait_id,			unitsDat[39].address);	// 00662F88
	BW_DATA_REF(u16*,			mineral_cost,			unitsDat[40].address);	// 00663888
	BW_DATA_REF(u16*,			vespene_cost,			unitsDat[41].address);	// 0065FD00
	BW_DATA_REF(u16*,			time_cost,				unitsDat[42].address);	// 00660428
	BW_DATA_REF(u16*,			requirement_offsets,	unitsDat[43].address);	// 00660A70 // "Restriction Address" according to EUDDB
	BW_DATA_REF(GroupFlag*,	group_flags,			unitsDat[44].address);	// 006637A0
	BW_DATA_REF(u8*,			supply_provided,		unitsDat[45].address);	// 006646C8 // Supply shown in-game is (true supply) / 2; e.g. Zerglings have 1 true supply, Marines have 2 true supply, etc.
	BW_DATA_REF(u8*,			supply_required,		unitsDat[46].address);	// 00663CE8
	BW_DATA_REF(u8*,			space_required,			unitsDat[47].address);	// 00664410
	BW_DATA_REF(u8*,			space_provided,			unitsDat[48].address);	// 00660988
	BW_DATA_REF(u16*,			build_score,			unitsDat[49].address);	// 00663408
	BW_DATA_REF(u16*,			destroy_score,			unitsDat[50].address);	// 00663EB8
	BW_DATA_REF(u16*,			map_string_id,			unitsDat[51].address);	// 00660260
	BW_DATA_REF(u8*,			broodwar_only,			unitsDat[52].address);	// 006606D8
	BW_DATA_REF(u16*,			staredit_avail_flags,	unitsDat[53].address);	// 00661518
} // (last entry at 0x00513EAC)

namespace flingy_dat {
//	BW_DATA(const DatLoad*,	flingyDat,	0x00515A38); // now in bwdata.cpp

	BW_DATA_REF(u16*,	sprite_id,			flingyDat[0].address);	// 006CA318
	BW_DATA_REF(u32*,	top_speed,			flingyDat[1].address);	// 006C9EF8
	BW_DATA_REF(u16*,	acceleration,		flingyDat[2].address);	// 006C9C78
	BW_DATA_REF(u32*,	halt_distance,		flingyDat[3].address);	// 006C9930
	BW_DATA_REF(u8*,	turn_speed,			flingyDat[4].address);	// 006C9E20 Incorrectly known as "Turn Radius"
//	BW_DATA_REF(u8*,	???,				flingyDat[5].address);	// 006CA240
	BW_DATA_REF(u8*,	movement_control,	flingyDat[6].address);	// 006C9858
} // (last entry at 0x00515A80)

// incorrect, must be clarified

// Farty sound extender
/*
namespace sfx_dat {
//	BW_DATA(const	DatLoad*,	sfxDat,		0x00515498); // now in bwdata.cpp
	
	BW_DATA_REF(u32*,	sound_pointer,		sfxDat[0].address);	// 0068DAA0
	BW_DATA_REF(u8*,	priority,			sfxDat[1].address);	// 0068D628
	BW_DATA_REF(u8*,	sound_flags,		sfxDat[2].address);	// 0068C8C0
	BW_DATA_REF(u16*,	portrait_duration,	sfxDat[3].address);	// 0068CD38
	BW_DATA_REF(u8*,	volume,				sfxDat[4].address);	// 0068C448
}
*/

namespace weapons_dat {
//	BW_DATA(const DatLoad*,	weaponsDat, 0x00513868);

	BW_DATA_REF(u16*,			label,					weaponsDat[0].address);		// 006572E0
	BW_DATA_REF(u32*,			flingy_id,				weaponsDat[1].address);		// 00656CA8
	BW_DATA_REF(u8*,			special_attack,			weaponsDat[2].address);		// 006573E8
	BW_DATA_REF(TargetFlag*,	target_flags,			weaponsDat[3].address);		// 00657998
	BW_DATA_REF(u32*,			min_range,				weaponsDat[4].address);		// 00656A18
	BW_DATA_REF(u32*,			max_range,				weaponsDat[5].address);		// 00657470
	BW_DATA_REF(u8*,			damage_upgrade,			weaponsDat[6].address);		// 006571D0
	BW_DATA_REF(u8*,			damage_type,			weaponsDat[7].address);		// 00657258 Normal, Explosive, etc.. Use with damage_type::Enum.
	BW_DATA_REF(u8*,			behavior,				weaponsDat[8].address);		// 00656670 Fly and follow target, appear on target unit, etc. Use with WeaponBehavior enum.
	BW_DATA_REF(u8*,			remove_after,			weaponsDat[9].address);		// 00657040
	BW_DATA_REF(u8*,			explosion_type,			weaponsDat[10].address);	// 006566F8 NormalHit, RadialSplash, EnemySplash, etc. Use with WeaponEffect::Enum.
	BW_DATA_REF(u16*,			inner_splash_radius,	weaponsDat[11].address);	// 00656888
	BW_DATA_REF(u16*,			medium_splash_radius,	weaponsDat[12].address);	// 006570C8
	BW_DATA_REF(u16*,			outer_splash_radius,	weaponsDat[13].address);	// 00657780
	BW_DATA_REF(u16*,			damage_amount,			weaponsDat[14].address);	// 00656EB0
	BW_DATA_REF(u16*,			armor_pen,				weaponsDat[15].address);	// 00657678
	BW_DATA_REF(u8*,			cooldown,				weaponsDat[16].address);	// 00656FB8
	BW_DATA_REF(u8*,			damage_factor,			weaponsDat[17].address);	// 006564E0
	BW_DATA_REF(u8*,			attack_angle,			weaponsDat[18].address);	// 00656990
	BW_DATA_REF(u8*,			launch_spin,			weaponsDat[19].address);	// 00657888
	BW_DATA_REF(u8*,			forward_offset,			weaponsDat[20].address);	// 00657910
	BW_DATA_REF(u8*,			vertical_offset,		weaponsDat[21].address);	// 00656C20
	BW_DATA_REF(u16*,			error_message,			weaponsDat[22].address);	// 00656568
	BW_DATA_REF(u16*,			weapon_icon,			weaponsDat[23].address);	// 00656780
} //weapons_dat (last entry at 0x0051397C)

namespace upgrades_dat {
//	BW_DATA(const DatLoad*,	upgradesDat,	0x005136E0); // now in bwdata.cpp

	BW_DATA_REF(u16*,		mineral_cost_base,			upgradesDat[0].address);	// 00656740
	BW_DATA_REF(u16*,		mineral_cost_factor,		upgradesDat[1].address);	// 006559C0
	BW_DATA_REF(u16*,		vespene_cost_base,			upgradesDat[2].address);	// 00655840
	BW_DATA_REF(u16*,		vespene_cost_factor,		upgradesDat[3].address);	// 006557C0
	BW_DATA_REF(u16*,		time_cost_base,				upgradesDat[4].address);	// 00655B80
	BW_DATA_REF(u16*,		time_cost_factor,			upgradesDat[5].address);	// 00655940
	BW_DATA_REF(u16*,		requirement_offsets,		upgradesDat[6].address);	// 006558C0	// Restriction sound_flags according to EUDDB
	BW_DATA_REF(u16*,		upgrade_icon,				upgradesDat[7].address);	// 00655AC0
	BW_DATA_REF(u16*,		label,						upgradesDat[8].address);	// 00655A40
	BW_DATA_REF(u8*,		race,						upgradesDat[9].address);	// 00655BFC
	BW_DATA_REF(u8*,		max_repeats,				upgradesDat[10].address);	// 00655700
//	BW_DATA_REF(u8*,		???,						upgradesDat[11].address);	// 00655B3C	// Brood War Flag according to EUDDB
} //upgrades_dat (last entry at 0x00513764)

namespace techdata_dat {
//	BW_DATA(const DatLoad*,	techdataDat,	0x005137D8); // now in bwdata.cpp

	BW_DATA_REF(u16*,		mineral_cost,				techdataDat[0].address);	// 00656248
	BW_DATA_REF(u16*,		vespene_cost,					techdataDat[1].address);	// 006561F0
	BW_DATA_REF(u16*,		time_cost,					techdataDat[2].address);	// 006563D8
	BW_DATA_REF(u16*,		energy_cost,				techdataDat[3].address);	// 00656380
//	BW_DATA_REF(u16*,		???,						techdataDat[4].address);	// 00656198
	BW_DATA_REF(u16*,		requirement_offsets,		techdataDat[5].address);	// 006562F8
	BW_DATA_REF(u16*,		tech_icon,					techdataDat[6].address);	// 00656430
	BW_DATA_REF(u16*,		label,						techdataDat[7].address);	// 006562A0
//	BW_DATA_REF(u8*,		???,						techdataDat[8].address);	// 00656488	// race according to EUDDB
//	BW_DATA_REF(u8*,		???,						techdataDat[9].address);	// 00656350	// "Researched" according to EUDDB
//	BW_DATA_REF(u8*,		???,						techdataDat[10].address);	// 006564B4	// Brood War Flag according to EUDDB
} // last entry at 0x00513850

namespace orders_dat {
//	BW_DATA(const DatLoad*,	ordersDat,	0x00513EC8); // now in bwdata.cpp

	BW_DATA_REF(u16*,		label,						ordersDat[0].address);	// 00665280
	BW_DATA_REF(u8*,		use_weapon_targeting,		ordersDat[1].address);	// 00664B00
//	BW_DATA_REF(u8*,		???,						ordersDat[2].address);	// 00665940 // May be "secondary order" used for debug/assert only / "Order goes in bSecondaryOrderID instead of bMainOrderID." according to EUDDB
	BW_DATA_REF(u8*,		Unused3,					ordersDat[3].address);	// 00665A00 // May be "Non-subunit" used for debug only
	BW_DATA_REF(u8*,		Unknown4,					ordersDat[4].address);	// 00664A40
//	BW_DATA_REF(u8*,		???,						ordersDat[5].address);	// 006657C0 // May be "Subunit can use" used for debug only
	BW_DATA_REF(b8*,		can_be_interrupted,			ordersDat[6].address);	// 00665040
	BW_DATA_REF(u8*,		Unknown7,					ordersDat[7].address);	// 00665100
	BW_DATA_REF(u8*,		can_be_queued,				ordersDat[8].address);	// 00665700
	BW_DATA_REF(u8*,		disabling_keeps_target,		ordersDat[9].address);	// 006651C0
	BW_DATA_REF(u8*,		can_be_obstructed,			ordersDat[10].address);	// 006654C0
	BW_DATA_REF(u8*,		Unknown11,					ordersDat[11].address);	// 00664C80
//	BW_DATA_REF(u8*,		???,						ordersDat[12].address);	// 00664BC0 // May be "Order requires moving"
	BW_DATA_REF(u8*,		order_weapon_id,			ordersDat[13].address);	// 00665880
	BW_DATA_REF(u8*,		tech_used,					ordersDat[14].address);	// 00664E00
	BW_DATA_REF(u8*,		order_animation,			ordersDat[15].address);	// 00664D40 // Animation (IscriptAnimation) according to EUDDB
	BW_DATA_REF(s16*,		button_icon,				ordersDat[16].address);	// 00664EC0
	BW_DATA_REF(u16*,		req_offset,					ordersDat[17].address);	// 00665580 // based on context
	BW_DATA_REF(u8*,		obscured_order,				ordersDat[18].address);	// 00665400
} //orders_dat (last entry at 0x00513FA0)

namespace sprites_dat {
//	BW_DATA(const DatLoad*,	spritesDat,		0x00513FB8); // now in bwdata.cpp

	BW_DATA_REF(u16*,		image_id,					spritesDat[0].address);	// 00666160
	BW_DATA_REF(s8*,		hp_bar_size,				spritesDat[1].address);	// 00665E50
//	BW_DATA_REF(s8*,		???,						spritesDat[2].address);	// 00666570
	BW_DATA_REF(u8*,		is_visible,					spritesDat[3].address);	// 00665C48
	BW_DATA_REF(s8*,		selection_circle_image,		spritesDat[4].address);	// 00665AC0
	BW_DATA_REF(s8*,		selection_circle_offset,	spritesDat[5].address);	// 00665FD8
} //sprites_dat (last entry at 0x00513FF4)

namespace images_dat {
//	BW_DATA(const DatLoad*,	imagesDat,	0x00514010); // now in bwdata.cpp

	BW_DATA_REF(const u32*,		image_file,			imagesDat[0].address);	// 00668AA0
	BW_DATA_REF(const u8*,		is_turnable,		imagesDat[1].address);	// 0066E860
	BW_DATA_REF(const u8*,		is_clickable,		imagesDat[2].address);	// 0066C150
	BW_DATA_REF(const u8*,		use_full_iscript,	imagesDat[3].address);	// 0066D4D8
	BW_DATA_REF(const u8*,		draw_if_cloaked,	imagesDat[4].address);	// 00667718
	BW_DATA_REF(const u8*,		rle_function,		imagesDat[5].address);	// 00669E28
	BW_DATA_REF(const u8*,		remapping,			imagesDat[6].address);	// 00669A40
	BW_DATA_REF(const u32*,		iscript_id,			imagesDat[7].address);	// 0066EC48
	BW_DATA_REF(const u32*,		lo_shields,			imagesDat[8].address);	// 0066C538
	BW_DATA_REF(const u32*,		lo_attack,			imagesDat[9].address);	// 0066B1B0
	BW_DATA_REF(const u32*,		lo_damage,			imagesDat[10].address);	// 0066A210
	BW_DATA_REF(const u32*,		lo_special,			imagesDat[11].address);	// 00667B00
	BW_DATA_REF(const u32*,		lo_landing,			imagesDat[12].address);	// 00666778
	BW_DATA_REF(const u32*,		lo_liftoff,			imagesDat[13].address);	// 0066D8C0
} //images_dat (last entry at 0x005140AC)

BW_DATA_REF(u8*,	g_is_valid_sprite,			0x00629A88);

BW_DATA(u32*,		g_image_overlay_attack,		0x0066B1B0);
BW_DATA(u32*,		g_image_overlay_special,	0x00667B00);

//////////////////////////////////////////////////////////////// @}

/// @name Reserved data
/// Warning: The following section contains data reserved for internal use only.
//////////////////////////////////////////////////////////////// @{

BW_DATA(u8*, g_unit_iterator_pos, 0x006284B6);
BW_DATA(UnitsSel*, active_player_selection, ccmu_extender.new_active_selection);

typedef Unit* (__cdecl* GetActivePlayerNextSelectionFunc)();
BW_DATA(GetActivePlayerNextSelectionFunc, get_active_player_next_selection, 0x0049A850);

/// Selections of all players
struct PlayersSel {
	Unit* unit[PLAYABLE_PLAYER_COUNT][SelectionCap];
};
BW_DATA(PlayersSel*, players_selections, ccmu_extender.new_players_selections);
struct PlayerSel {
	Unit* unit[SelectionCap];
};
BW_DATA(const PlayerSel*, current_unit_selection, ccmu_extender.new_active_selection);

typedef void(__fastcall* PrepareForNextOrderFunc)(Unit*);
BW_DATA(PrepareForNextOrderFunc, prepare_for_next_order, 0x00475000);

// Contains various tile info.
BW_DATA(ActiveTile**,		g_active_tile_array,	0x006D1260);
BW_DATA(u16**,			tileIdArray,			0x005993C4);
BW_DATA(u16**,			megatileArray,			0x00628494);
//BW_DATA(u16**,			megatileExtArrayFlags,	0x00628494+0x20000); // extender only

// Stores an ordered list of all units currently in the game.
BW_DATA(UnitFinderData*,	g_unit_ordering_x,		ccmu_extender.unit_finder_x);
BW_DATA(UnitFinderData*,	g_unit_ordering_y,		ccmu_extender.unit_finder_y);
BW_DATA(u32*,				g_unit_ordering_count,	0x0066FF74);

BW_DATA(Vector32*,		g_angle_distance,		0x00512D28);
BW_DATA(u16*,				g_megatile_count,		0x005998E0);

// Used internally by Starcraft to manage temporary units lists
BW_DATA(Unit***, g_temp_units_lists_array, ccmu_extender.get_all_units_in_bounds_list);
BW_DATA(u32*, g_temp_units_list_current_array_count, 0x006BEE64);
BW_DATA(u32*, g_temp_units_list_arrays_counts_list_last_index, 0x006BEE6C);
BW_DATA(u32*, g_temp_units_list_arrays_counts_list, ccmu_extender.get_all_units_in_bounds_index);

// Font & Drawing
namespace graphics {
	struct Font; struct Bitmap;
}
BW_DATA(graphics::Font**,		font_base,				0x006CE0F4);
BW_DATA(graphics::Bitmap*,	g_game_screen_buffer,	0x006CEFF0);
BW_DATA(u8*,					g_refresh_regions,		0x006CEFF8); // 640 x 480 divided into 1200 squares of 16x16
BW_DATA(Layers*,				g_screen_layers,		0x006CEF50);
typedef void(__stdcall* DrawGameProc)(graphics::Bitmap* surface, Bounds* bounds);
static DrawGameProc const g_old_draw_game_proc = (DrawGameProc)0x004BD580;
BW_DATA(u32*,					g_current_font_color,	0x006CE0C0);
BW_DATA(u8*,					g_font_spacing,			0x006CE0C4);
BW_DATA(Box16*,				g_draw_text,			0x006CE0C8);/*
BW_DATA(u16*,					g_draw_text_left,		0x006CE0C8);
BW_DATA(u16*,					g_draw_text_top,		0x006CE0CA);
BW_DATA(u16*,					g_draw_text_right,		0x006CE0CC);
BW_DATA(u16*,					g_draw_text_bottom,		0x006CE0CE);*/
BW_DATA(u32*,					g_saved_font_color,		0x006CE0E8);

// Array used in multiplayer to randomize factions used by unit.get_faction_color() and player.get_faction_color()
// Max index is PLAYABLE_PLAYER_COUNT-1
BW_DATA(s32*, g_factions_colors_ordering, 0x0057F21C);

// Access to true player colors that differ from standard versions from graphics.h
// This is PlayerColors from BWAPI
// Max index is PLAYER_COUNT-1
BW_DATA(u8*, g_player_minimap_colors,	0x00581DD6);
BW_DATA(u8*, g_player_color_ids,		0x0058F442);
BW_DATA(u8*, g_player_in_game_colors, 0x00581D76);

const ListExtern<Image, &Image::link> unusedImages(0x0057EB68, 0x0057EB70);
BW_DATA(List<Sprite>*, g_free_sprites, 0x0063FE30);

struct SpriteTileData {
	Sprite* tails[256];
	Sprite* heads[256];
};
BW_DATA(SpriteTileData*, g_sprites_on_tile_row, 0x00629288);

// Image-related
struct ImagesDatExtraOverlayLO_Files {
	LO_Header** attack_overlays;
	LO_Header** damage_overlays;
	LO_Header** special_overlays;
	LO_Header** landing_dust_overlays;
	LO_Header** liftoff_dust_overlays;
};
BW_DATA_REF(ImagesDatExtraOverlayLO_Files*,		lo_files,				0x0051F2A8);
BW_DATA_REF(const LO_Header* const*,				g_shield_overlays,		0x0052E5C8);

BW_DATA(const ColorShiftData*,					g_color_shift,			0x005128F8); // Use ColorRemapping::Enum as index
BW_DATA_REF(GrpHead* const*,						g_image_grp_graphics,	0x0051CED0);

template <typename T>
struct PlayerFlags {
	T flags[PLAYER_COUNT];
};
BW_DATA(PlayerFlags<u8>*,		player_alliance,			0x0058D634); // See bw::is_allied_to()
BW_DATA(PlayerFlags<u32>*,	player_vision,				0x0057F1EC);

BW_DATA(Unit**,				g_active_portrait_unit,		0x00597248);

BW_DATA_REF(AI_Main*,			g_aiscript_controller,		0x0068FEE8);
BW_DATA(AiRegion**,			g_ai_region_captains,		0x0069A604);
BW_DATA(u32*,					g_ai_supply_reserved,		0x006CA4BC);
BW_DATA(AiScript**,			g_first_active_aiscript,	0x0068C100);
BW_DATA(AiScriptArray*,		g_aiscript_array,			0x0068C0FC);

BW_DATA(ResourceAreaArray*,	g_resource_areas,		0x00692688);

//-------- Internal constants --------//

BW_DATA(b32*, g_is_game_paused,				0x006509C4);  // See bw::g_is_game_paused()
BW_DATA(b8*,  g_is_brood_war,				0x0058F440);  // See bw::is_brood_war_mode()
BW_DATA(u32*, g_cheat_state,				0x006D5A6C);  // See bw::is_cheat_enabled()
BW_REF(b32,	g_is_in_replay,					0x006D0F14);  // See bw::is_in_replay()
BW_DATA(b32*, g_is_replay_paused,			0x006D11B0);
BW_DATA(s32*, g_max_unit_width,				0x006BEE68);
BW_DATA(s32*, g_max_unit_height,			0x006BB930);
BW_REF(b32, g_is_in_game_loop,				0x006D11C8);
BW_REF(u32, g_last_random_number,			0x0051CA14);  // See bw::random(), bw::rand_between()
BW_DATA(b32*, g_can_update_powered_status,	0x0063FF44);
BW_REF(b32, g_is_placing_building,			0x00640880);

//-------- Tech levels --------//

struct _scTechs {
	u8 is_enabled[PLAYER_COUNT][24];		// 0x0058CE24
	u8 is_researched[PLAYER_COUNT][24];		// 0x0058CF44
};
struct _bwTechs {
	u8* is_enabled[PLAYER_COUNT];			// 0x0058F050
	u8* is_researched[PLAYER_COUNT];		// 0x0058F140
};

// Available and Researched techs are exactly next to each other, that's why with 2 entries structures, only 2 addresses are needed here when BWAPI got 4 of them.
BW_DATA(_scTechs*, TechSc, 0x0058CE24); //Use with ScTech::Enum
BW_DATA_REF(_bwTechs*, TechBw, 0x0058F050); //Use with BwTech::Enum

//-------- Upgrade levels --------//

struct _scUpgrs {
	u8 max_level[PLAYER_COUNT][46];		// 0x0058D088
	u8 current_level[PLAYER_COUNT][46];	// 0x0058D2B0
};
struct _bwUpgrs {
	u8* max_level[PLAYER_COUNT];			// 0x0058F278
	u8* current_level[PLAYER_COUNT];		// 0x0058F32C
};
BW_DATA(_scUpgrs*,		UpgradesSc, 0x0058D088); // Use with ScUpgrade::Enum
BW_DATA_REF(_bwUpgrs*,	UpgradesBw, 0x0058F278); // Use with BwUpgrade::Enum

// Requirement opcode lists
namespace requirements
{
	BW_DATA_REF(u16*, units,		0x00514178);
	BW_DATA_REF(u16*, orders,		0x00514CF8);
	BW_DATA_REF(u16*, research,	0x00514908);
	BW_DATA_REF(u16*, upgrade,	0x005145C0);
	BW_DATA_REF(u16*, tech,		0x00514A48);
}

BW_DATA(GuardAiArray*,       g_guard_array,			0x0067D400);
BW_DATA(GuardAiListPlayers*, g_player_guard_list,		0x00685108);

BW_DATA(u32*,			g_ai_update_cooldown,			0x0059CC94);

// For update_creep_disappearance_unit
BW_DATA(u32*,			g_map_creep_width,						0x006D0F08);
BW_DATA(u32*,			g_dying_unit_creep_disappearance_count, 0x00658AE4);
BW_DATA(u32**,		g_current_tiles,						0x006D0E84);

// For draw_tooltip_with_icon
BW_DATA(char*,		grp_stat_res_icon_mapping,		0x00515228); // sizeof 8
BW_DATA(GrpHead*,		icons_grp,						0x0068C238); // This is wrong. -DF
BW_DATA(GrpHead**,	g_icons_grp,					0x0068C238);
BW_DATA(Box16*,		tooltip_required_area,			0x006CE0D8);

// For do_attack_notify_unit
BW_DATA(char*,		g_palette_index_reference,		0x006CEB20); // sizeof at least 20
BW_DATA(u32*,			notify_cooldown,				0x00640870);
//BW_DATA(SoundData*,	g_sound_data,					0x006D1270); // THIS IS NOT EXTENDED!!!

// For stat_data_unit_wireframe_select_update
BW_DATA(char**,	off_68C1FC,							0x0068C1FC);
BW_DATA(char*,	g_palette_remap,					0x0050CDC1);

// For kill_units_at_location_proc
BW_DATA(u32*,		trig_remove_unit_active,			0x005971DC);
BW_DATA(u32*,		trig_killing_additional_unit,		0x00596CD4);
BW_DATA(u32*,		trig_kill_unit_count,				0x005971E0);

// For btnsact_rally_point
BW_DATA(u8*,		g_order_id_ground,					0x00641691);
BW_DATA(char*,	g_order_id_obstructed,				0x00641693);
BW_DATA(char*,	g_order_id_unit,					0x00641692);
BW_DATA(u16*,		g_buttonset_special_buttonset_id,	0x0068C1C8);

// For select_multiple_units_from_unit_list
BW_REF(char,	g_client_selection_changed,			0x0059723C);

// For set_ingame_input_procs
BW_DATA(HACCEL*,				common_accelerator,		0x006D120C);
BW_DATA(HACCEL*,				accelerator_tables,		0x005968F0);
BW_DATA(InputProcsFunc**,		left_click_proc,		0x006556EC);
BW_DATA(InputProcsFunc**,		right_click_proc,		0x006556F0);

BW_DATA(u16*,					g_minimap_cursor,		0x006556FC);
BW_DATA(RECT*,				g_minimap_rect,			0x00512D00);

// For process_right_click_action
BW_DATA(u16*,			is_unit_in_point_x_pos,			0x006509CC);
BW_DATA(u16*,			is_unit_in_point_y_pos,			0x006509CE);

// For input_game_right_mouse_click
BW_DATA(Unit**,		local_selection_group_2,		ccmu_extender.new_local_selection_group);

// For init_damage_overlay_counts
BW_DATA_REF(u8*,		g_building_overlay_state_max,	0x005240D0);

// To move in-game timer while leaderboard is active
BW_DATA(BinDlg**,		leaderboard_dialog,				0x00597284);
BW_DATA(char*,		leaderboard_type,				0x0058D70C);

// For begin_gas_building
BW_DATA(char*,		can_draw_placement_preview,		0x006D5BE4);

// For create_all_selection_circles
BW_DATA(char**,		game_result_text,				0x00628668);

// For find_fow_target
BW_DATA(Thingy**,		g_first_fow_sprite,				0x00654868);
BW_DATA(Thingy**,		g_last_fow_sprite,				0x0065486C);
BW_DATA(Thingy**,		g_first_free_fow_sprite,		0x006509D0);
BW_DATA(Thingy**,		g_last_free_fow_sprite,			0x00654870);

// For play_building_complete_sound
BW_DATA(u32*,			building_complete_something,	0x006D1AE8);

// For trigact_transmission
BW_DATA(char*,			g_is_executing_wait,		0x006509B8);
BW_REF(TriggerLink*,		g_current_trigger_node,		0x006509AC);
BW_DATA(u32*,				g_trigger_wait,				0x00650980);

// For draw_minimap_box
BW_DATA(char*,			g_minimap_state,			0x006D5BBE);
BW_DATA(u32*,				g_minimap_box_checksum,		0x0059C1A8);
BW_DATA(u32*,				g_minimap_draw_neutrals,	0x005993AC);

// For draw_train_progress
BW_DATA(GrpHead**,		g_cmdbtns_handle,			0x0068C1C0);

// For draw_all_minimap_boxes
BW_DATA(Bitmap**,			g_current_surface,			0x006CF4A8);
BW_DATA(Bitmap*,			g_minimap_surface_2,		0x0059C194);
BW_DATA(u32*,				g_minimap_unit_box_count,	0x0059C2B8);

// For execute_trigger_conditions
BW_DATA(void**,			g_trigger_condition_list,	0x00515A98);

// For play_sound trigger
BW_DATA(s32*,				g_big_volume_array,			0x005008F0);

BW_DATA(u32*,				g_game_speed_replay_modifiers,	0x005124D8);

// For play_yes_unit_responses
BW_DATA(Unit**,			g_place_building_parent_unit,	0x00640874);

// For sprite_to_iscript_loop
BW_DATA(u16*,			g_map_tile_height,					0x0057F1D6);
BW_DATA(Sprite**,		g_horizontal_sprite_lines_begin,	0x00629688);
BW_DATA(Sprite**,		g_horizontal_sprite_lines_end,		0x00629288);

// For get_cursor_type
BW_DATA(RECT*,		g_cursor_limit_rect,				0x005993B0);
BW_DATA(char*,		g_is_drag_selecting,				0x0066FF5C);

// For stats_panel_display hooks
BW_DATA(char**,		g_queues_buffers,					0x00519F40);

// For adding implicit ESC to submenus
BW_DATA(u32*,			g_active_portrait_buttonset_id,		0x0068C14C);
BW_DATA(u32*,			g_current_buttonset_id,				0x0068C1C4);

// For request_military_to
BW_DATA_REF(u32*,		ai_strength_air,					0x006BB210);
BW_DATA_REF(u32*,		ai_strength_ground,					0x006BB5A0);

// For show_cursor_marker
BW_DATA(Thingy**,		g_first_used_thingy_1,				0x00652918);
BW_DATA(u8*,			g_can_draw_thingy,					0x00652920);
BW_DATA(char**,		g_iscript_data,						0x006D1200);

// For play_warp_in_overlay
BW_DATA(UpdateFunction*,	g_sg_update_funcs,				0x00512510);
BW_DATA(DrawFunction*,	g_sg_draw_funcs,				0x005125A0);

// For iscript_opcode_cases
BW_DATA(char*,		cbullet_damage,						0x0050434C);

// For create_unit_building_sprite_validity_array
BW_DATA(u32*,			g_current_visibility_hash,			0x0063FD30);
BW_DATA(u32*,			g_want_visibility_hash_update,		0x00629D90);
BW_DATA(u8*,			g_region_vision_hash,				0x0063FE38);
BW_DATA(u32*,			g_current_hash_offset,				0x0063FD28);

BW_DATA(ControlGroupPlayerArray*, control_group_array,	ccmu_extender.new_saved_selections);

// For remove_healthbar
BW_DATA(Layer*,		g_game_layer,						0x006CEFB4);
BW_DATA(Image**,		g_first_free_healthbar,				0x0057EB6C);
BW_DATA(Image**,		g_last_free_healthbar,				0x005254B8);

BW_DATA(Image**,		g_first_free_selection_circle,		0x0052E4C0);
BW_DATA(Image**,		g_last_free_selection_circle,		0x0052F564);

BW_DATA(u16*,			g_temp_iscript_header,				0x006C2310);
BW_DATA(u16*,			g_iscript_speed_something,			0x006C2314);

BW_DATA(char*,		g_draw_color_index,					0x006CF4AC);

BW_DATA(void**,		g_repulse_handle,					0x006D5CD8);

BW_DATA(u8*,			g_images_draw_if_cloaked,			0x00667718);

// For buttonsets.cpp
BW_DATA(BinDlg**,		g_buttonset_dialog,					0x0068C148);
BW_DATA(u16*,			g_highlighted_icon,					0x0068C1BC);
BW_DATA(u8*,			g_buttonset_unit_order_id,			0x0068C1E4);
BW_DATA(u8*,			g_buttonset_subunit_order_id,		0x0068C1B8);

// For issue_place_building_order
BW_DATA(u8*,			g_place_building_order_id,			0x0064088D);

// For replay/witness UI
BW_DATA(ForceNames*,	force_names,						0x0058D5BC);

// For game_recv
BW_DATA(u32*,			g_sequence,							0x006556E8);
BW_DATA(u32*,			g_active_storm_id,					0x00512680);
BW_DATA(s32*,			g_is_bad_hash,						0x006552A8);
BW_DATA(s32*,			g_array_player_status,				0x006552A8);

// For reveal_sight_at_location
BW_DATA(SightRange*,		g_sight_values,					0x00513A18);
BW_DATA(SightCallbacks*,	g_sight_callbacks,				0x00504524);

// For create_thingy
BW_DATA(Thingy**,			g_first_free_thingy,			0x00654878);
BW_DATA(Thingy**,			g_last_free_thingy,				0x0065487C);

// For remove_subunit_or_gas_container
BW_DATA(Unit**,			g_last_dying_unit,				0x0059CC98);

// For unit_selection_click
BW_DATA(b32*,				g_is_double_clicking,			0x0066FF58);

// For assign_player_names
BW_DATA(u32*,				g_score_screen_player_count,	0x0059A0E0);

// For find_best_cargo_return_to_target
BW_DATA(Box16*,			g_full_map_pixel_dimensions,	0x006284A0);

// For draw_unit_box
BW_DATA(Bitmap*,			g_minimap_surface,					0x0059C18C);
BW_DATA(Bitmap*,			g_minimap_surface_fog,				0x0059C194);
BW_DATA(void**,			g_minimap_vision_CB,				0x0059C188);
BW_DATA(void**,			g_minimap_surface_CB,				0x0059C1A0);
BW_DATA(u16*,				g_ping_zoom_div,					0x0059CC68);
BW_DATA(u16*,				g_ping_zoom_lshift,					0x0059C1B0);
BW_DATA(u16*,				g_map_width,						0x0059CC74);
BW_DATA(u16*,				g_map_height,						0x0059CC64);
BW_DATA(u8*,				g_minimap59C2C0,					0x0059C2C0);
BW_DATA(u8*,				g_minimap59CB60,					0x0059CB60);
BW_DATA(BinDlg**,			g_minimap_dlg,						0x0059CC60);
BW_DATA(u16*,				g_minimap_zoom_level,				0x0059CC6C);
BW_DATA(u32*,				g_minimap_move_tile_x,				0x0059CC70);
BW_DATA(const VR4**,		vr4FileHandle,						0x00628444);
BW_DATA(const VX4**,		vx4FileHandle,						0x00628458);

// For replay autosaves
BW_DATA(u8*,				g_is_league_game,					0x00629268);

// For draw_standard_button_graphic
BW_DATA(char*,			g_print_xy_alignment,				0x006CE110);

// For draw_stat_button_image
BW_DATA(CmdBtnPalette*,	g_cmdbtn_palette,					0x0068C150);

// For load_statdata_bin
BW_DATA(char*,			aztp,								0x00512700);

// For ai_prepare_to_flee
BW_DATA(u32*,				unk_static,							0x0051ABFC);

BW_DATA(char*,			g_text_buffer,						0x006CA76C);

// For load_unload_proc 
BW_DATA(UnitPath**,		g_first_free_path,					0x006BEE84);
BW_DATA(UnitPath**,		g_path_array_start,					0x006BEE8C);

// For initialize_provider_version
BW_DATA(bool*,			g_is_expansion,						0x0058F440);
BW_DATA(u32*,				g_is_demo,							0x005124D0);

// For print_stat_res_info
BW_DATA(char**,			g_countdown_timer_strings,			0x0068C240);

// For load_menu_glu_expcampaign
BW_DATA(u32*,				g_gl_glues_mode,					0x006D11BC);

// For weapon_behavior_can_hit_target_check
BW_DATA(u32*,				g_neutral_carapace_swarm,			0x005873F0);

// For tile height extension
BW_DATA(ActiveTile**,		activeTileArray,					0x006D1260);
BW_DATA(const MTXM**,		zergCreepTileData,					0x005993A8);
BW_DATA(const MTXM**,		MTXMHandle,							0x005993C4);
BW_DATA(const VF4**,		vf4FileHandle,						0x005993D0);
BW_DATA(const CV5**,		cv5FileHandle,						0x006D5EC8);

// For on_player_join
BW_DATA(u32*,				g_local_storm_id,					0x0051268C);
BW_DATA(u32*,				g_current_network_mode,				0x0059688C);

// For update_visibility_hash
BW_DATA(u8*,				g_region_vision_filter,				0x00629C90);

// For desync debugging
BW_DATA(u8*,				g_player_flag,						0x0065FC10);
BW_DATA(u32*,				g_mask_hash_current_y,				0x0065EB28);

// For glu_main_dialog_interact (glucustom.cpp)
BW_DATA(u32*,				g_is_broodwar,						0x006D11E4);

typedef bool __fastcall glu_main_function(BinDlg*, DialogEvent*);
BW_DATA(glu_main_function**, glu_main_functions, 0x0051A388);

// For get_unit_portrait_override
BW_DATA(u16*,				g_campaign_index,					0x0057F244);

// For game_talking_portrait
BW_REF(u32,					g_trigger_portrait_active,			0x0068AC4C);

// For do_unit_event_notification
BW_DATA(u32*,				g_notification_message_id,			0x0068ACA0);

// For update_selected_unit_portrait
BW_DATA(u32*,				g_unk_portrait,						0x0068AC60);

// For dialog_music_fade
BW_REF(HANDLE*,				g_direct_sound_handle,				0x006D5BAC);
BW_DATA(MusicStruct*,		g_music_bank,						0x00512BB8);
BW_REF(u32,					g_big_music_volume,					0x006D5BB8);

// For ignoring inputs in menu
BW_DATA(BinDlg**,         g_popup_dialog,                     0x006D5BF4);
BW_DATA(u8*,              g_chat_message_type,                0x0068C144);

// For play_addon_detach_sound
BW_DATA(u8*,				g_audio_something,						0x006D1A98);

// For unit_die
BW_DATA(u32*,				g_ignore_unit_flag80_clear_subcount,	0x006D5BD0);

// For image_render
BW_DATA(u8*,				UnkRGBMashup,						0x005993D8);

// For init_title
BW_DATA(BinDlg**,			g_main_dlg,							0x0057F0DC);

// For create_main_window
BW_DATA(HINSTANCE*,		g_main_hinstance,					0x0051BFAC);
BW_DATA(HIMC*,			g_app_context,						0x006D6438);

// For show_or_hide_minimap
BW_DATA(s8*,				g_hide_minimap_terrain,				0x006D5BBF);

typedef int (__fastcall MinimapVisionCallback)();
BW_DATA(MinimapVisionCallback*, g_minimap_vision_callback,	0x0059C188);

BW_REF(Unit*,				g_first_active_unit,				0x00628430);
BW_REF(Unit*,				g_last_active_unit,					0x0059CC9C);

//-------- Format strings used by Starcraft implementations of printf --------//

BW_DATA(const char*, FORMATSTRING_TEXT_SPACE_TEXT, 0x00503614); //"%s %s"
BW_DATA(const char*, FORMATSTRING_COLORYELLOW_VALUE_COLORCYAN_SPACE, 0x00505708); //"\x03%d\x01 "
BW_DATA(const char*, FORMATSTRING_VALUE_SPACE, 0x00505710); //"%d "
BW_DATA(const char*, FORMATSTRING_CURRENTVALUE1_MAXVALUE1_COLOR_CURRENTVALUE2_MAXVALUE2, 0x00505714); //"%d/%d %c%d/%d"
BW_DATA(const char*, FORMATSTRING_COLOR_CURRENTVALUE_MAXVALUE, 0x00505724); //"%c%d/%d"
BW_DATA(const char*, FORMATSTRING_COLORWHITE_CURRENTVALUE_MAXVALUE, 0x0050572C); //"\x04%d/%d"
BW_DATA(const char*, FORMATSTRING_TEXT_SPACE_VALUE_IN_PARENTHESIS, 0x00505734); //"%s (%d)"
BW_DATA(const char*, FORMATSTRING_TEXT_SPACE_VALUE, 0x0050573C); //"%s %d"

//////////////////////////////////////////////////////////////// @}

#undef BW_DATA
#undef BW_DATA_REF
#pragma pack()