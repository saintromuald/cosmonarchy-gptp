#pragma once
#include "BW/bwdata.h"
#include "BW/structures.h"

#define NOMINMAX
#include <windows.h>
#include <source_location>

#define BW_FUNC(address, return_and_convention, name, ...) \
typedef return_and_convention name##_func(__VA_ARGS__); \
name##_func* const name = (name##_func*) address;

namespace helper {
    BW_FUNC(0x0045B440, void __stdcall, ai_expand_to, AiScript* aiscript, u32 resarea, u32 block);
    BW_FUNC(0x004A1E50, void __fastcall, remove_unit_ai, Unit* unit, b32 unit_was_killed);
    BW_FUNC(0x00487B00, Thingy* __stdcall, find_blocking_fow_resource, u32 x, u32 y, u32 radius);
    BW_FUNC(0x00493640, void __cdecl, create_pylon_aura);
    BW_FUNC(0x004845F0, void __cdecl, input_target_order_right_mouse_click);
    BW_FUNC(0x00484600, void __cdecl, input_place_building_right_mouse_click);
    BW_FUNC(0x00425B50, void __stdcall, draw_unit_status_string, BinDlg* dialog);
    BW_FUNC(0x0048A460, void __stdcall, execute_all_game_triggers, u32 ms_per_frame);
    BW_FUNC(0x0045D040, s32 __stdcall, select_new_unit, Unit* unit);
    BW_FUNC(0x0049B870, bool __cdecl, update_alliances);
    BW_FUNC(0x00437250, bool __fastcall, player_has_transports, u32 player);
    BW_FUNC(0x00440FA0, bool __fastcall, return_cargo_to_best_depot_proc, Unit* depot, Unit* worker);
    BW_FUNC(0x004D27A0, void __fastcall, alloc_background_image, const char* filename, Bitmap* bitmap, Palette* palette, const char* log_filename, u32 log_line);
    BW_FUNC(0x00410154, b32 __stdcall, sbmp_load_image, const char* filename, PaletteEntry* palette, const char* buffer, u32 buffer_size, u32* width, u32* height, u32* bpp);
    BW_FUNC(0x004B5220, char __fastcall, glu_expcampaign_custom_ctrl_id, BinDlg* dialog, DialogEvent* dlg_event);
    BW_FUNC(0x004BE0B0, char __cdecl, refresh_cursor_0);
    BW_FUNC(0x0041ca00, void __stdcall, refresh_region);
    BW_FUNC(0x004BE060, s32 __cdecl, cursor_refresh);
    BW_FUNC(0x0049BFD0, char __cdecl, update_screen_position);
    BW_FUNC(0x004D4960, bool __fastcall, title_dlg_proc, BinDlg* dialog, DialogEvent* dlg_event);
    BW_FUNC(0x004D1D70, s32 __fastcall, main_window_proc, void* main_window, HWND hwnd, UINT msg, u32* wparam, u32* lparam);
    BW_FUNC(0x004ADEA0, s32 __stdcall, compile_list_callback, const char* string, s32 a3, s32 a4, s32 a5);
    BW_FUNC(0x004DDD30, char* __stdcall, get_string_by_id, s16 id);
    BW_FUNC(0x004c9780, void __stdcall, save_options_menu_result, BinDlg* dialog);
    BW_FUNC(0x004f57a0, void __fastcall, load_bin_dlg, const char* bin_name, BinDlgProc dlgproc);
    BW_FUNC(0x004de140, void __fastcall, show_queued_network_error, u16 index);

    // Menu functions
    BW_FUNC(0x004db930, void __stdcall, glu_main);
    BW_FUNC(0x004baf30, void __stdcall, glu_connection);
    BW_FUNC(0x004b9e70, void __stdcall, glu_chat);
    BW_FUNC(0x004ad330, void __stdcall, glu_bnet);
    BW_FUNC(0x004edc30, void __stdcall, glu_login);
    BW_FUNC(0x004b2930, void __stdcall, glu_campaign);
    BW_FUNC(0x004f7550, void __stdcall, glu_ready_terran);
    BW_FUNC(0x004f71f0, void __stdcall, glu_ready_zerg);
    BW_FUNC(0x004f78b0, void __stdcall, glu_ready_protoss);
    BW_FUNC(0x004b86c0, void __stdcall, glu_join);
    BW_FUNC(0x004b6490, void __stdcall, glu_load);
    BW_FUNC(0x004b4af0, void __stdcall, glu_scorescreen);
    BW_FUNC(0x004b19c0, void __stdcall, glu_modem);
    BW_FUNC(0x004b52a0, void __stdcall, glu_ex_campaign);
    BW_FUNC(0x004dcc90, void __stdcall, glu_generic);

    // Default DlgProcs and UpdateFuncs indexed by DialogControlType
    BW_ARRAY(bw::DialogInteraction*, 15, g_default_dialog_interaction_functions, 0x005014ac);
    BW_ARRAY(bw::DialogDraw*,        15, g_default_dialog_draw_functions,        0x00501504);

    // Per control type
    BW_FUNC(0x00418eb0, u32 __fastcall, default_interaction_dialog,    bw::Dialog* dialog, DialogEvent* event);
    BW_FUNC(0x004e1c20, u32 __fastcall, default_interaction_button,    bw::Dialog* dialog, DialogEvent* event);
    BW_FUNC(0x004e1bc0, u32 __fastcall, default_interaction_option,    bw::Dialog* dialog, DialogEvent* event);
    BW_FUNC(0x004e1b50, u32 __fastcall, default_interaction_checkbox,  bw::Dialog* dialog, DialogEvent* event);
    BW_FUNC(0x004191e0, u32 __fastcall, default_interaction_image,     bw::Dialog* dialog, DialogEvent* event);
    BW_FUNC(0x00415af0, u32 __fastcall, default_interaction_slider,    bw::Dialog* dialog, DialogEvent* event);
    BW_FUNC(0x00416980, u32 __fastcall, default_interaction_textinput, bw::Dialog* dialog, DialogEvent* event);
    BW_FUNC(0x00419190, u32 __fastcall, default_interaction_label,     bw::Dialog* dialog, DialogEvent* event);
    BW_FUNC(0x0041baa0, u32 __fastcall, default_interaction_listbox,   bw::Dialog* dialog, DialogEvent* event);
    BW_FUNC(0x0041bc40, u32 __fastcall, default_interaction_combobox,  bw::Dialog* dialog, DialogEvent* event);
    BW_FUNC(0x004e17e0, u32 __fastcall, default_interaction_lightup,   bw::Dialog* dialog, DialogEvent* event);

    BW_FUNC(0x004ab840, u32* __stdcall, glu_main_load_ums_templates);
    BW_FUNC(0x004c4680, void __stdcall, glu_login_leave_game);
    BW_FUNC(0x004b5180, b32 __stdcall, dlgproc_glu_campaign_activate, BinDlg* dialog);
    BW_FUNC(0x004d26b0, u8* __stdcall, generic_malloc_wrapper, u32 size);

    BW_FUNC(0x00450210, void __stdcall, glu_chat_update_download_graphic);
    BW_FUNC(0x004c46e0, void __stdcall, process_make_game_public_command);
    BW_FUNC(0x00471670, void __stdcall, process_move_to_slot_request, const Command_MoveToForce* command, u32 sender);
    BW_FUNC(0x00470d10, void __stdcall, register_network_player, u8 player_id, u16 flags, u16 unk_1, u16 protocol_version);
    BW_FUNC(0x004513f0, void __stdcall, chat_update_race_slots, bw::Dialog* dialog);
    BW_FUNC(0x004b9bf0, void __stdcall, glu_chat_dlgproc_create, BinDlg* dialog);
    BW_FUNC(0x004b9ac0, void __stdcall, glu_chat_map_preview_init, BinDlg* dialog);

    BW_FUNC(0x004d9cd0, u32 __stdcall, get_cdkey_hash);
    BW_FUNC(0x0045ade0, u8 __stdcall, find_force_balanced_slot);
    BW_FUNC(0x004a99c0, u32 __stdcall, find_first_available_slot);
    BW_FUNC(0x00472ab0, void __stdcall, add_player_to_active_list, u8* active_list, u32 sender);
    BW_FUNC(0x004c4160, void __stdcall, update_game_mode, u32 open_slots);
    BW_FUNC(0x0041bc40, u32 __fastcall, generic_combobox_interact, BinDlg* dialog, DialogEvent* event);
    BW_FUNC(0x004e1b50, u32 __fastcall, generic_checkbox_interact, BinDlg* dialog, DialogEvent* event);
    BW_FUNC(0x004e1560, u32 __fastcall, generic_menu_button, BinDlg* dialog, DialogEvent* event);
    
    BW_FUNC(0x00450a60, u32 __fastcall, chat_race_combobox_interact, BinDlg* dialog, DialogEvent* event);

    // Hud functions
    BW_FUNC(0x0041d470, void __stdcall, hud_update_surface_masks);
    BW_FUNC(0x00456ef0, void __cdecl,   unload_statdata_bar);   // destroys statdata dialog + progressbar buffers
    BW_FUNC(0x00458cf0, void __cdecl,   unload_buttonset);      // destroys buttonset dialog + cmdbtns.grp + cmdicons.grp
    BW_FUNC(0x0045e710, void __cdecl,   unload_statport);       // stops portrait video, destroys portrait dialog, frees portdata
    BW_FUNC(0x00481480, void __cdecl,   reset_status_screen_area); // bw_refresh + reset status area to full-screen
    BW_FUNC(0x00456b40, void __fastcall, statdata_bar_update, BinDlg* dialog, DialogEvent* event);
    BW_FUNC(0x00458730, void __fastcall, cmdbuttons_bar_update, BinDlg* dialog, DialogEvent* event);
    BW_FUNC(0x00459B00, u32 __fastcall, statbtn_dlgproc, BinDlg* dialog, DialogEvent* dlg_event);
    BW_FUNC(0x004f36c0, u32 __fastcall, textbox_dlgproc, bw::Dialog* dialog, DialogEvent* dlg_event);
    BW_FUNC(0x004becf0, u32 __fastcall, statlb_dlgproc, bw::Dialog* dialog, DialogEvent* dlg_event);
    BW_FUNC(0x004e5910, u32 __fastcall, statres_dlgproc, BinDlg* dialog, DialogEvent* dlg_event);
    BW_FUNC(0x004f5240, u32 __fastcall, stat_f10_dlgproc, bw::Dialog* dialog, DialogEvent* dlg_event);
    BW_FUNC(0x004b8590, u32 __fastcall, glujoin_dlgproc, bw::Dialog* dialog, DialogEvent* dlg_event);
    BW_FUNC(0x004f4d60, u32 __fastcall, statfluf_dlgproc, bw::Dialog* dialog, DialogEvent* dlg_event);
    BW_FUNC(0x004a5a80, void __stdcall, load_minimap);
    BW_FUNC(0x0045f320, void __stdcall, load_statport);
    BW_FUNC(0x00458570, void __stdcall, load_stat_data);

    // Sound functions
    BW_FUNC(0x004A5EF0, s32 __cdecl, get_music_volume_centibels);   // Master music volume in DS centibels
    BW_FUNC(0x004BC320, void __cdecl, sound_update_volumes);        // Reimplemented in music_hooks; applies g_options_list volumes

    // Generic modal text entry with custom prompt
    BW_FUNC(0x004b6e50, BOOL __stdcall, glu_modal_string_prompt, const char* prompt, char* output, u32 output_size, const char* disallowed_chars);

    BW_FUNC(0x004be120, void __stdcall, refresh_cursor);

    BW_FUNC(0x004F2DE0, b32 __fastcall, is_stacked, Unit* unit);
    
    BW_FUNC(0x00445E80, void __stdcall, assign_to_nearest_resarea, u32 res_type, u8* res_type_list);
    BW_FUNC(0x00446350, void, create_resource_areas);

    BW_FUNC(0x0045B850, void __stdcall, aiscript_cases, AiScript* script);
    
    void hud_destroy_top_surface(SurfaceContext* surface);  // 0041d5a0
    void glu_create(bool multiplayer); // 004af6d0

    bool has_money_can_make(Unit* builder, u32 unit_to_build);

    // Hud functions
    DialogTheme* dialog_grp_construct(const char* filename, const char* log_filename, u32 log_line);
    void* load_graphic(const char* filename, u32* bytes_read, const char* log_filename, u32 log_line);
    void apply_dialog_theme(DialogTheme* theme);

    s32 rand_func();
    bool unit_hp_below_33_percent(Unit* unit);
    bool unit_hp_between_33_66_percent(Unit* unit);
    s32 get_max_hp(Unit* unit);
    u16 get_last_queue_slot_id(Unit* unit);
    void add_text_to_dialog(BinDlg* dialog, s16 index, const char* text);
    void stats_panel_draw_energy(BinDlg* dialog);
    void hide_optional_status_screen_controls(BinDlg* dialog);
    void show_control_group(BinDlg* dialog, u16 firstId, u16 lastId);
    void hide_dialog(BinDlg* dialog);

    // For draw_supply_unit_info
    s32 get_supplies_used_no_args();
    u32 get_supplies_available(u8 a1, s32 a2);
    void unit_stat_act_standard(void* a1);

    // For ai_target_enemy_proc
    bool ai_unit_can_attack(Unit* unit);

    // For draw_building_info
    s32 ui_needs_redraw();
    Unit* generic_status_update_drawn_values();
    void draw_progress_bar(BinDlg* dialog);
    void draw_nuke_silo_info(BinDlg* dialog);
    bool unit_is_active_transport(Unit* a1);
    bool is_queue_slot_active(Unit* unit, s32 slot);
    void draw_training_progress(BinDlg* dialog);					// 004268D0
    bool is_constructing_addon(const Unit* a1);
    bool is_attempting_protoss_build(Unit* a1);
    void draw_addon_progress(BinDlg* a1);
    void draw_tech_progress(BinDlg* a1);
    void draw_upgrade_progress(BinDlg* a1);
    bool unit_is_geyser(Unit* unit);
    void draw_gas_quantity(BinDlg* a1);
    s8 get_unit_ground_weapon(Unit* a1);
    void draw_display_buttons(BinDlg* a1);
    BinDlg* get_control_from_index(BinDlg* a1, s16 a2);
    void set_text_string(BinDlg* a1);
    void set_unit_status_string_text(BinDlg* a1);
    s32 get_unit_name(Unit* a1);
    void draw_supply_unit_info(BinDlg* dialog);						// 00427540

    // For get_unit_name
    const char* get_unit_name_stat_string(u16 unit_id);				// 0047B090

    // For get_unit_strength
    s32 get_current_hp_plus_shields(Unit* a1);
    s32 get_base_strength(s32 a1, Unit* a2);

    // For AI_RestorationRequirementsProc
    s32 can_weapon_target_unit(WeaponId::Enum weapon_id, Unit* target, Unit* attacker);

    // For AI_EMPShieldRequirementsProc
    s32 ai_target_unit_is_worth_hitting(Unit* a1, Unit* a2);
    s32 create_splash_weapon_proc(u16 range, u16 pos_x, u16 pos_y, s32(__fastcall* callback)(s32, s32), Unit* caster);
    s32 unit_shields_clump_in_area_accumulator_proc(Unit* a1, Unit* a2);

    // For hallucinationHit
    void unit_was_hit(Unit* a1, Unit* a2, s32 a3);
    void unit_attack_notify_noop(Unit* unit);
    Image* create_shield_overlay(s32 a1, Unit* a2);

    // For draw_image_hp_bar

    s32 unk_billions(s32 a1, s32 a2, s32 a3, s32 a4);
    s32 unk_billions2(s32 a1, s32 a2, s32 a3, s32 a4);
    s32 hpBarSurfaceBuffer(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5);
    s32 hpBarSurfaceBuffer2(s32 a1, s32 a2, u16 a3, s32 a4);
    s32 renderHPBar(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5);
    s32 get_unit_max_energy(Unit* a1);
    s32 getHpBarDrawSizes(s32 a1, s32 a2, s32 a3, s32 a4);

    // For cannot_cast_spell
    bool unitIsFrozen(Unit* a1);
    bool isUnitCasterNotHallucination(Unit* a1);
    b32 get_active_tile_flag(s16 y, s16 x, s32 includeFlags, s32 excludeFlags);

    // For compile_health_bar
    void update_image_data_draw(Image* result);
    void play_image_iscript(Image* img, u8 iscript_anim);

    // For ai_find_best_transport
    bool can_enter_transport(Unit* a1, Unit* a2);
    s32 get_unit_distance_to_halt(s32 a1, Unit* a2, s32 a3);

    // For map_data_transfer
    s32 dataTransfer00(s32 a1, char a2, s32 a3);
    s32 dataTransfer01(u32 a1, s32 a2, u32 a3);
    s32 dataTransfer02(s32 a1, s32 a2, char a3);
    s32 dataTransfer03(s32 a1, char a2, s32 a3);
    s32 dataTransfer04(u32 a1, s32 a2, s32 a3);
    s32 dataTransfer05(s32 a1, s32 a2);
    VectorHead* vectorTransfer(char a1, char a2);
    s32 dataTransfer06(s32 a1);

    // For get_random_yes_unit_response_local
    u32 get_random_unit_response(u32 min, u32 max);													// 0048E940
    s16 get_region_id(s32 x, s32 y);																// 0049C9F0

    // For ai_train_unit
    char ai_training_worker(Unit* result, Unit* parent);											// 00435700
    void ai_training_overlord(Unit* result, Unit* parent);											// 00435770
    void ai_training_normal(Unit* result, Unit* parent);											// 00435DB0
    void ai_order_to_destination(Unit* unit, s32 order_id, s32 x, s32 y);							// 0043D5D0
    void ai_assign_region_to_slowest_unit(AiRegion* region);										// 00436F70

    // For ai_create_nuke
    Unit* create_unit(s16 unit_id, s32 x, s32 y, s32 player);
    void update_unit_stats_and_finish_building(Unit* unit);
    u32 update_unit_strength_and_apply_default_orders(Unit* unit);
    void hide_unit(Unit* unit);

    // For AI_TrainingBroodling
    void ai_set_region_state(s32 region_type, AiRegion* region);									// 004390A0
    void add_military_ai(AiRegion* region, Unit* unit, s32 always_this_region);						// 0043DA20

    // For ai_training_worker
    char add_worker_structure_ai(Unit* worker, AiTown* town);										// 00433DD0

    // For AI_AddUnitAI
    BuildingAi* create_building_ai(BuildingAiList* buildingAI);
    void issue_order_targeting_nothing(Unit* unit, u8 order_id);
    Unit* set_secondary_order(Unit* result, u8 order_id);
    void ai_make_guard(s32 player, Unit* unit);
    bool unit_is_active_resource_depot(Unit* unit);
    u32 get_default_refinery_type(u32 player);

    // for ai_unit_was_hit // some of these should probably be methods in api/etc
    char cancel_unit(Unit* structure);
    Unit* ai_get_actual_target(Unit* attacker, Unit* target);
    Unit* check_arbiter_cloaking_range(Unit* unit, u16 id);
    void ai_order_unit_cloaking(Unit* unit);
    void ai_order_lurker_burrow(Unit* unit);
    bool unit_is_in_attack_group(Unit* unit, b32 exclude_grouping_regions);
    bool unit_is_active_transport(Unit* unit);
    s32 is_unit_loaded(Unit* unit);
    s32 ai_try_return_home(Unit* unit, s32 dontIssueOrder);
    s32 ai_ask_for_help(Unit* self, Unit* attacker, s32 onlyNearbyGuards);
    s16 get_region_id_from_unit(Unit* unit);
    s32 get_enemy_air_strength_in_region(u16 region_id, s32 player);
    s32 get_enemy_strength_in_region(u16 region_id, s32 player, s32 includeAir);
    bool unit_is_performing_unbreakable_code(Unit* unit);
    bool ai_cast_spell_behavior(Unit* unit, bool is_aggro);
    s32 ai_flee(Unit* unit, Unit* enemy);
    bool unit_has_ground_path(Unit* unit, s16 x, s16 y);
    bool ai_has_no_defense_build(s32 player);
    bool unit_can_see_cloaked_target(Unit* target, Unit* unit);
    void ai_respond_to_cloaked_threat(Unit* threat, Unit* unit);
    bool unit_can_attack_target(Unit* target, Unit* unit, b32 check_detection);
    b32 ai_update_attack_target(Unit* unit, s32 accept_if_sieged, s32 accept_critters, s32 must_reach);

    // for iterate_building_ai
    u16 ai_remove_from_towns(Unit* worker);
    s32 ai_get_queued_count(Unit* builder, u32 unit_id);
    void prepare_guard_ai(u16 y, u32 player, u16 unit_id, u16 x);

    // For ai_send_suicide
    bool is_threat(Unit* target, Unit* unit);															// 00476930
    void prepare_suicide(Unit* unit);																	// 0043CD40
    void ai_attack_add(u32 count, UnitId::Enum unit_id, u32 player);                                    // 00447230
    u32 ai_attack_clear(s32 player);																	// 00439500
    bool ai_attack_manager(s32 player, s32 x, s32 y, s32 bOverrideLastAttack, s32 bFallbackAirOnly);	// 0043ABB0

    // Attack extender

    // for AI_TrainAttackForce
    s32 ai_add_to_region_military(AiRegion* region, s32 unit_id, s32 priority);
    u32 ai_add_cost_to_needs(u32 unit_id, u32 player, u32 type);
    u32 ai_add_spending_request(u16 unit_id, u32 player, u8 type, char priority, void* value);

    // for AIRegion_GroupForAttack
    u32 ai_recalculate_region_strength(AiRegion* region);
    Unit* ai_target_enemy_in_region(AiRegion* region);
    s32 ai_move_military_to_defend(AiRegion* region);
    void ai_get_region_military_types(AiRegion* region, b32* hasGroundMilitary,
        b32* cantAttackGround, b32* cantAttackAir);
    s32 ai_can_travel_to_region(AiRegion* region1, AiRegion* region2);
    Unit* ai_pick_attack_target_no_buildings(AiRegion* region);
    u32 ai_move_military_to_other_region(AiRegion* target, AiRegion* source, u32 pullUnitsNearRegion,
        u32 sendPureSpellcasters, u32 dontSendFromBunkers);
    AiTown* ai_iterate_town_something(AiRegion* region);
    u32 ai_attack_timer_decrement(u32 player);
    s32 ai_has_forces_moving_to_group(u32 player);

    // for ai_attack_manager
    u16 populate_regions_with_own(char* region_data, u32 player);
    void populate_regions_with_enemy(u32 player, char* region_data);
    s32 ai_get_neighboring_region_types(char* region_data);
    void ai_finish_region_accessibility(char* region_data);
    s32 ai_set_attack_grouping_region(u32 airAttack, u32 alsoVeryNear, u32 player, char* region_data, s32 x, s32 y, u32 alwaysOverwriteLastAttack);
    char cancel_unit(Unit* structure);

    bool unit_cant_see_cloaked_target(Unit* unit, Unit* target);											// 00401D60
    bool unit_can_infest(Unit* unit);																		// 00402750
    void ai_attack_unit(Unit* unit);																		// 0043FFD0
    Unit* find_best_attack_target(Unit* unit);																// 00443080
    bool ai_try_return_home(Unit* unit, u8 unk_byte_value);					    							// 00462EA0
    void set_interceptors_to_standby(Unit* unit);															// 00465780
    Order* remove_order_from_unit_queue(Order* order, Unit* unit);											// 004742D0
    void prepend_order_targeting_unit(Unit* unit, Unit* target, u8 order_id);								// 00474A70
    bool is_target_within_min_movement_range(Unit* unit, Unit* target, u32 range);							// 004763D0
    bool ai_is_unreachable_position(Unit* unit, s32 x, s32 y);												// 00476610
    bool is_unit_in_weapon_range(Unit* unit, Unit* target);													// 00476870
    void get_weapon_begin_iscript(Unit* unit, IscriptAnimation::Enum anim);							    	// 00476ED0
    void pick_new_target_if_needed(Unit* unit);																// 00477510
    void attack_move_try_pick_target(Unit* unit, u8 order_id);												// 00477820
    bool attack_move_react_to_attack(Unit* unit, u8 order_id);												// 00478370
    bool attack_unit_main(Unit* unit);																		// 00478540
    bool unit_is_critter(Unit* unit);																		// 0047B450
    void move_to_collide(Unit* unit, Unit* target);														    // 00495400

    bool is_unit_visible(Unit* unit);																		// 004E5DB0
    bool is_target_visible(Unit* unit, Unit* target);														// 004E5E30
    u8 get_right_click_action_order(Unit* unit);															// 004E5EA0
    void set_resource_target(Unit* unit);															        // 004EB290
    void make_to_hold_position(Unit* unit);																	// 004EB5B0
    bool move_to_target(Unit* target, Unit* unit);															// 004EB720
    bool set_move_target_xy(Unit* unit, s32 x, s32 y);														// 004EB820
    s32 move_towards(Unit* unit, Unit* target);																// 004EB900
    bool change_movement_target_move_target_2(Unit* unit, s32 x, s32 y);									// 004EB9C0

    bool is_distance_greater_than_halt_distance(Unit* unit, s32 x, s32 y, u32 distance);					// 00401240
    void check_unstack(Unit* unit);												                            // 0042D9A0
    Unit* check_collision(Unit* unit);												                        // 0042D9A0
    bool start_building(Unit* unit);																		// 00467030
    Unit* begin_gas_building(Unit* unit, u32 unit_id);														// 004678A0
    void add_overlay_below_main(Sprite* sprite, u32 imageId, u32 x_offset, u32 y_offset, u32 direction);	// 00498D70
    void replace_sprite_images(Sprite* sprite, u32 image_id, u32 image_direction);							// 00499BB0
    void display_last_network_error_for_player(u32 player);										            // 0049E530
    void update_unit_strength(Unit* unit);												                    // 0049FA40
    void iscript_opcode_cases(Image* image, void* iscript_header, u32 dont_modify_image, u32 out_value);	// 004D74C0
    void switch_to_warp_texture(Image* image);																// 004D8500
    bool set_thingy_visibility_flags(Thingy* thingy);									                    // 004878F0
    const char* set_unit_stat_txt_error_message(const char* message);									    // 0048CCB0
    void play_sound_at_pos(u32 sound_id, v2u16 pos, u32 bool_play = 1, u32 min_volume = 0);				// 0048EC10
    u32 randomize_short(u32 unknown_index);																	// 004DC4A0
    u32 iterate_units_at_location_target_proc_recall_targets(Unit* unit, Box16* coords);					// 004E8280
    void move_unit_duplicate(Unit* unit, s32 x, s32 y);														// 004EBAE0

    //// spider_mine
    Unit* spider_mine_acquire_target(Unit* mine);										                    // 00441270
    void instant_cloak(Unit* unit);												                            // 0049B1E0
    void unburrow_generic(Unit* unit);												                        // 004E99D0
    void burrow_generic(Unit* unit);																		// 004E9A30

    // For orders_place_mine
    s32 tech_use_allowed(Unit* unit, u16 tech_id, u32 player);							                    // 0046DD80
    u32 get_distance_fast(s32 pos1_x, s32 pos2_x, s32 pos1_y, s32 pos2_y);									// 0040C360
    s32 orders_hold_pos_suicidal(Unit* unit);												                // 004EB5B0
    void turn_unit(Unit* unit, s8 direction);													            // 00495F20
    void to_idle(Unit* unit);																                // 004753A0
    s32 is_unit_movable(Unit* unit);														                // 00401DC0
    void do_next_queued_order(Unit* unit);																	// 00475000

    // For burrow_generic
    void toggle_unit_path(Unit* unit);									                                    // 004E42A0
    char switch_sprite_id(Sprite* sprite, s32 sprite_id);													// 00499860

    //// building_morph
    void refund_building_cost(u16 unit_id, u8 player);														// 0042CE70
    void update_building_ai(Unit* unit);																	// 00433FE0
    bool is_valid_building_morph(u16 building_id);															// 0045CC60
    void update_new_unit_cagra(u32 id, s32 x, s32 y);														// 0045CE90
    void cancel_zerg_building_morph(Unit* unit);															// 0045D410
    void stop_creep_disappearing_at_building(Unit* unit);												    // 0047D770
    void increment_unit_scores_ex(Unit* unit, s32 count, b32 update_morph_merge_unit_score);				// 00488D50
    void play_building_complete_sound(Unit* building);														// 0048F310
    void refresh_sprite_data(Sprite* sprite);																// 004983A0
    void display_last_network_error_for_player(u32 player);									                // 0049E530
    void replace_unit_with_id(Unit* unit, u16 new_unit_id);													// 0049FED0

    //// building_terran
    bool is_distance_greater_than_halt_distance(Unit* unit, s32 x, s32 y, u32 distance);					// 00401240
    void fix_target_location(v2u16* coords, u32 unit_id);								                    // 00401FA0
    bool is_unit_positions_2_equal(Unit* unit);																// 00402160
    b32 generic_progress_build(Unit* unit, u32 hp_gain, b32 show_hidden);									// 004679A0
    void prepend_order_targeting_nothing(Unit* unit, Order* order, u8 order_id);							// 00474760
    void add_order_targeting_nothing(Unit* unit, Order* order, u8 order_id);								// 004749A0
    bool can_still_place_building();														                // 0048DDA0
    void end_building_placement();															                // 0048E310
    void play_building_complete_sound(Unit* building);														// 0048F310
    void set_secondary_waypoint(Unit* unit, s32 x, s32 y);									                // 00494BB0
    void display_last_network_error_for_player(u32 player);									                // 0049E530
    u32 rand_between(u32 min, u32 max, u32 seed_type);														// 004DC550
    void unit_completion_init(Unit* unit, b32 has_construction_anim);										// 004E65E0
    bool order_to_move_to_target(Unit* unit, Unit* target);													// 004EB980
    b32 unk_terrain_collision_2(Unit* unit, s32 x, s32 y);													// 004F1870

    // For updatePlacementState_Units
    bool does_building_block(Unit* unit, s32 xTile, s32 yTile);												// 00473410
    s32 is_tile_blocked_by(Unit** unit_array, Unit* builder, s32 tile_x, s32 tile_y,				    	// 00473300
        bool is_addon, bool also_invisible);

    // For update_placement_state_for_placement_box
    s32 tile_location_is_unbuildable(Unit* builder, s32 player, s32 xTile, s32 yTile, s32 unit_id,			// 00473FB0
        s32 placementStateEntry, s32 bCheckVision, s32 bAlsoInvisible, s32 bWithoutVision);
    
    // For refresh_screen
    s32 update_placement_state_for_placement_box();															// 0048DCE0
    s32 unk_building_placement_related(s32 a1, v2u16* a2);												// 0048D7F0
    s32 init_placement_box_pos(v2u16* in_out);															// 0048D660
    char unk_building_placement_related_mouse(s32 a1, s32 a2);												// 0048D7B0

    //// liftland
    void set_unit_pathing(Unit* unit, u8 unkPathRelated);																			// 00401360
    bool is_unit_positions_2_equal(Unit* unit);																						// 00402160
    Unit* connect_addon_check(Unit* unit);																							// 004404A0
    void modify_building_grounded_status(Unit* unit);																				// 00463470
    void toggle_pathing_related(Unit* unit);																						// 004634E0
    void flying_building_lift_if_still_blocked(Unit* unit);																			// 00463640
    void complete_addon(Unit* unit, Unit* addon);																					// 00463D50
    void crush_immobile_units(Unit* unit);																							// 00464300
    void disconnect_addon(Unit* unit);																							// 00464930
    void clear_building_from_tile_flag(Unit* unit, s32 x, s32 y);																	// 00469EC0
    void set_building_tile_flag(Unit* unit, s32 x, s32 y);																			// 00469F60
    void end_collision_tracking(Unit* unit);																						// 0046A560
    void start_collision_tracking(Unit* unit);																						// 0046A5A0
    void force_order_done(Unit* unit);																								// 00475420
    bool update_creep_disappearance_unit(Unit* unit);																				// 0047DF90
    void update_creep_disappearance_unit_id(u32 unit_id, u32 x, u32 y, b32 completed);												// 0047DE40
    void place_building_proc();																										// 0048E6E0
    void play_landing_interrupt_sound(Unit* unit);																						// 0048F5A0
    Unit* set_unit_movement_speed(Unit* unit, u32 speed);																			// 004951C0
    void set_shadow_y(Sprite* sprite, u8 newVerticalOffset);																		// 00498150
    void create_landing_dust(Sprite* sprite);																						// 004997E0
    void remove_from_selection(Unit* unit);																							// 004C3B40

    //// create_init_units
    b32 spreads_creep(u32 unit_id, b32 defaultReturn);																				// 00413870
    void apply_creep_at_location_from_unit_id(s32 x, s32 y, u16 unit_id);															// 004148F0
    void create_building_larva(Unit* main_building);																				// 0049D660
    b32 show_finished_unit(Unit* unit);																								// 0049EC30
    Unit* find_best_unit(Box16* coords, u32 searchProc);																			// 004E8830

    //// larva_creep_spawn
    u8 spread_creep(u32 unit_id, s32 x, s32 y);																						// 00414680
    void ai_training_overlord(Unit* main_building, Unit* larva);																	// 00435770
    void update_unit_strength(Unit* unit);																							// 0049FA40
    u32 iterate_units_at_location_target_proc_larva_count(Unit* unit, Box16* coords);												// 004E8280
    void set_larva_order_state(Unit* unit);																							// 004E8DA0
    bool get_larva_spawn_pos(Unit* unit, v2s32* coords);																			// 004E8E10

    // TODO: move somewhere else?
    // SC Variables and Buffers used in this file
    u32* const mapTileBufWidth = (u32*)(0x006D0F08);
    u32* const mapTileBufHeight = (u32*)(0x006D0C6C);
    u16** const mapTileBuf = (u16**)(0x006D0E84);
    v2s32* const someCreepPointBuf = (v2s32*)(0x006D7578);
    typedef u32(*isTileVisible_func)(u32 x, u32 y);
    typedef bool(*SetCreepRelationalData_callback)(u16* tileBackup, u8* creepOverlayVal, u32* retval, u16* tile, u32 arg);
    isTileVisible_func* const isTileVisible_ptr = (isTileVisible_func*)(0x006D0C78);

    u32 get_num_surrounding_creep_tiles(u32 x, u32 y);																				// 00414290
    void set_creep_relational_data(u32 x, u32 y, SetCreepRelationalData_callback callback, u32 arg);								// 00413FA0
    u32 iterate_directional_creep_data(u32 x, u32 y, u32 callback, u32 arg);														// 00413B30

    // for apply_creep_at_location_from_unit_id
    
    typedef s32 __stdcall CreepChange(s32, s32, s32, s32, s32);
    s32 modify_creep_state(s32 x_tile, s32 y_tile, CreepChange* creepChange, u32 param);											// 00413FA0

    //// burrow_tech
    void force_order_done(Unit* unit);																								// 00475420
    void targeted_order(Unit* unit, u32 unit_id, u32 unk1, u32 unk2, u32 order_id, u32 unk3, u32 unk4, u32 unk5);					// 004754F0

    //// die_order
    void clear_unit_sprites(Sprite* unit_sprite, Sprite* subunit_sprite);															// 00496F00
    void refresh_all_visible_images_at_screen_pos(Sprite* thingy_sprite, u32 visibility_flags);										// 00497480
    void unit_die(Unit* unit);																										// 004A0740
    void remove_unit(Unit* unit);																									// 004A0990
    void hide_and_disable_unit(Unit* unit);																							// 004E6340

    //// CMDRECV_Cancel
    void cancel_research_proc(Unit* unit);																							// 00453E30
    void cancel_upgrade_proc(Unit* unit);																							// 00454280
    void refund_queue_slot(Unit* unit, u16 slot);																					// 00466A70
    void cancel_last_training(Unit* unit);																							// 00466E40
    void cancel_addon_proc(Unit* unit);																								// 004E66E0

    //// cmdrecv_build
    b32 order_is_allowed(Unit* unit, u16 order, u32 nationID);																		// 0046DC20
    bool place_building_message(u32 result_function_473FB0);																		// 0048D930
    bool CMDRECV_place_building_allowed(Unit* unit, u8 order_id, u16 builtUnitId);													// 0048DBD0
    void process_building_placement(Unit* unit, u32 order_id, u16 dimensionX, u16 dimensionY, u32 builtUnitId);						// 0048DE70
    void CMDRECV_place_building(s16 x, s16 y, u8 order_id, u16 builtUnitId);														// 0048E190

    //// CMDRECV_MergeArchon
    Unit* templar_merge_partner(Unit* unit, Unit** units_list, u32 units_list_length);												// 004BFA80

    //// cmdrecv_stop
    void parse_orders_dat_requirements();																							// 0046D450
    u16* parse_tech_dat_requirements();																								// 0046D4C0
    s32 parse_requirement_opcodes(Unit* unit, u32 order_id, u32 player, u16* address, u32 req_offset);								// 0046D610

    //// give_unit
    void apply_speed_upgrade_from_unit_type(Unit* unit);																			// 00454370
    void recheck_attacks(Unit* unit);																								// 00476D30
    void increment_unit_scores(Unit* unit, s32 count);																				// 00488BF0
    void create_unit_selection(Unit* unit, u32 selection_index);																	// 004E6180
    void destroy_unit_selection(Unit* unit);																						// 004E6290
    void drop_powerup(Unit* powerup);																								// 004F3B70

    //// cast_order
    u32 get_weapon_cooldown(Unit* unit, WeaponId::Enum weapon_id);																				// 00475DC0
    bool is_ready_to_attack(Unit* unit, WeaponId::Enum weapon_id);																				// 00476640
    u32 get_stat_txt_string(Unit* unit, u32 player, const char* message);															// 0048EF30
    bool cannot_cast_spell(Unit* unit);																								// 00492140
    bool ordered_spell_progress_moving(Unit* unit, b32 order_state_zero);															// 00492620

    // For spell_order
    bool can_target_spell(u16 tech_id, Unit* target, u16 fow_unit_id, Unit* caster, u16 pos_x, u16 pos_y, s32* error);				// 00492020
    b32 spell_order_move_towards_target(Unit* caster, u32 range);																	// 00492450

    // For can_target_spell
    s16 can_target_spell_on_fow_unit(s32 tech_id, u16 unit_id);																		// 004914B0

    void increment_unit_death_scores(Unit* unit, u8 player);																		// 00488AF0
    void give_sprite(Unit* unit, u8 player);																						// 0049E4E0
    void give_unit(Unit* unit, u32 player, s32 unkScore);																			// 0049EFA0
    void change_unit_buttonset(Unit* unit, u16 unit_id);																			// 004E5D60
    void show_and_enable_unit(Unit* unit);																							// 004E6490

    //// tech_target_check
    bool is_infestable_target(Unit* unit);																							// 00402210

    //// trap_hooks
    // For update_unit_strength_and_apply_default_orders
    void begin_cloak(Unit* unit, s32 sound_id);																						// 0049B5B0
    void refresh_unit_vision(Unit* unit);																								// 004E5F30
    void init_unit_trap_doodad(Unit* unit);																							// 004E6490
    Order* perform_another_order(u8 order_id, Order* order, Unit* unit, u16 fow_unit_id, v2u16 position_xy, Unit* target);		// 004745F0
    void order(s32 some_bool, Unit* unit, s8 order_id, v2u16 pos, Unit* target_unit, u32 fow_unit_id);							// 00474810
    u32 get_unit_strength(Unit* a1, s32 a2, bool a3);																				// 00431800

    //// make_nydus_exit
    void play_worker_error_sound(Unit* unit, u32 error_text_index);																	// 0048F6C0

    //// research_upgrade_orders
    bool is_unit_upgrade_available(Unit* unit);																						// 004033D0
    void apply_speed_upgrade_from_upgrade_id(Unit* unit, u8 upgrade_id);															// 00454540
    void play_upgrade_complete_sound();																								// 0048F070
    void play_research_complete_sound();																							// 0048F150
    void set_upgrade_level(u32 player, u32 upgrade_id, u8 upgrade_level);															// 004CE770

    //// merge_units
    bool advance_remaining_build_time(Unit* unit);																					// 00466940
    u8 delete_pathing(Unit* unit);																									// 0046A820
    void order_immediate(Unit* unit, u8 order);																						// 00474B40
    void play_unit_ready_sound(Unit* unit);																					// 0048F440
    void merge_status_into_unit(Unit* unit_dst, Unit* unit_src);																	// 00493180
    void end_cloak(Unit* unit, u32 sound_id);			 																			// 0049B440
    bool delayed_set_move_target_xy(Unit* unit, u16 x, u16 y);																		// 004EB960

    //// nuke_orders
    void set_all_overlay_directions_generic(Unit* unit, u32 unk);																	// 00401140
    u32 ai_return_to_nearest_base_forced(Unit* unit);																				// 0043DB50

    //// target_related

    // for is_unit_target_out_of_max_range
    s32 get_unit_max_weapon_range(Unit* unit, WeaponId::Enum weapon_id);																		// 00475870

    // for is_ready_to_attack
    u32 get_distance_to_unit(Unit* unit_1, Unit* unit_2);																			// 00430D30
    u32 distance_from_dbox_corner(Unit* a1, __int16 a2, __int16 a3);
    bool is_unit_in_attack_angle(s32 pos_x, s32 pos_y, Unit* unit, WeaponId::Enum weapon_id);

    // for can_cast_ordered_spell_now
    bool is_target_within_min_range(Unit* unit, u32 min_range, Unit* target);														// 00430F10
    s32 get_flingy_halt_distance_in_subpixels(Unit* unit);																						// 00494F90

    // for orders_stay_in_range
    void prepend_order_targeting_ground(__int8 order_id, __int16 x, __int16 y, Unit* unit);
    s32 orders_hold_position_suicidal(Unit* a1);
    s32 get_direction_from_points(s32 dest_y, s32 x, s32 dest_x, s32 y);
    s32 get_direction_from_points(v2u16 a, v2u16 b);

    // for ai_attack_unit
    void stimpack_proc(Unit* unit);																									// 004554A0

    // for generic_right_click_unit_proc_2
    s32 is_order_valid(char order_id, Unit* self, Unit* unused, Unit* target, s32* error);											// 00474D90

    // for cmdrecv_hold_position
    Unit* get_active_player_next_selection();																						// 0049A850
    s32 order_allowed(Unit* a1, u16 a2, s32 a3);																					// 0046DC20
    void targeted_order(s16 a1, s32 a2, Unit* a3, s16 a4, s32 a5, s32 a6, char a7, s32 a8, s32 a9, s32 a10);						// 004754F0

    // for check_unit_visibility
    u32 get_detecting_players(Unit* unit);																							// 00441130
    u32 iterate_player_units_something(u32 aResult, Unit* unit);																	// 0049E8A0
    void make_sprite_visible(Sprite* sprite);																						// 00497ED0
    s32 update_building_land_unit_selection(u32 detection_status, Unit* selection);													// 004C3B40
    char remove_selection_circle(Sprite* sprite);																					// 004975D0
    void make_sprite_invisible(Sprite* sprite);																						// 00498A10

    //// receive_command
    void function_004756B0(Unit* unit, u8 order_id, u32 unk1, u32 unk2, u32 command_type);											// 004756B0
    void function_004756E0(Unit* unit, Unit* target, u32 order_id, u32 command_type);												// 004756E0
    u32 can_do_targeted_order(Unit* unit, u8 order_id);																				// 0049A410
    bool can_target_self(Unit* unit, u8 order_id);																					// 0049A480

    //// load_unload_proc
    u8 ignore_iscript_go_to_idle(Unit* unit);																						// 00476550
    u32 can_unload(Unit* transport, Unit* loaded_unit, v2u16* pos);																// 004E76C0
    void unload_unit_ext(Unit* unit, u32 unk);																						// 004E7E10

    //// stats_panel_display
    void stat_nuke_count(BinDlg* dialog, u32 index);																				// 00425310
    void stats_hangar_count(BinDlg* dialog, u32 index);																				// 004253D0
    void stats_shield_level(BinDlg* dialog, u32 index);																				// 00425510
    void stats_armor_level(BinDlg* dialog, u32 index);																				// 00425600
    void stats_weapon_level(BinDlg* dialog, u32 index, WeaponId::Enum weapon_id);													// 00425790
    bool stat_spidermine_count(BinDlg* dialog, u32 index);																			// 00426300

    // For get_unit_rank_string and check_hotkey_char
    __int8 unit_get_race(Unit* unit);																									// 0047B150
    const char* get_stat_txt_string(u16 id);																						// 004C36F0
    char* get_stat_txt_mutable_string(u16 id);																						// 004C36F0

    // For stats_weapon_level / stats_armor_level
    char get_armor_count(Unit* unit, s32 a2);																							// 00453FC0
    void show_dialog(BinDlg* dialog);																								// 004186A0
    
    s32 disable_dialog(bw::Dialog* dialog);																								// 00418640
    inline s32 disable_dialog(BinDlg* dialog) { return disable_dialog(dialog->to_dialog()); }
    
    void update_dialog(bw::Dialog* dialog);                                                                                         // 0041C400
    inline void update_dialog(BinDlg* dialog) { update_dialog(dialog->to_dialog()); }

    //// bunker_hooks
    Unit* find_random_attack_target(Unit* unit);																					// 00442FC0
    b32 attack_apply_cooldown(Unit* unit);																							// 00478B40
    void set_image_direction(Image* image, s8 direction);																			// 004D5F80

    //// api
    void apply_corrosive_spores(Unit* unit);
    Bullet* create_bullet(WeaponId::Enum weapon_id, Unit* source, s16 x, s16 y, u8 attacking_player, u8 direction);
    void weapon_bullet_shot(Unit* target, Bullet* bullet, u32 damage_divisor);
    Unit* create_unit_at_pos(u16 unit_id, u16 player, u32 x, u32 y);																// 004CD360
    Flingy* create_flingy(u32 flingy_id, u32 player, u32 x, u32 y);																	// 00496460

    //// unit_morph
    u8 set_archon_pathing(Unit* unit);																								// 0046A820
    bool has_supplies_for_unit(u32 unit_id, u32 player, b32 can_show_error_message);												// 0042CF70
    Unit* zerg_dual_birth(Unit* unit);																							    // 0045D910
    void order_new_unit_to_rally(Unit* unit, Unit* factory);																		// 00466F50
    void ai_training_unit(Unit* unit, Unit* creator);																				// 004A2830
    void delete_building_ai(BuildingAi* building, BuildingAiList* buildingList);                                                    // 00404500
    void delete_worker_ai(WorkerAi* ai, WorkerAiList* workerList);                                                                  // 00404470
    void delete_ai_town_if_needed(AiTown* town);                                                                                    // 004347E0
    void remove_from_attack_force(u32 unit_id, u32 player);																			// 00447040
    bool ai_has_worker_or_building(u32 region_id, u32 player);																		// 00432AD0

    //// hallucination_spell
    // for orders_hallucination1
    bool spell_order(Unit* unit, u32 tech_id, u16* energy_cost, u32 sight_range, u32 error_message_index);							// 004926D0

    void remove_unit(Unit* unit);
    bool place_hallucination(Unit* unit);																							// 004F66D0

    // for create_hallucination
    void apply_hallucination_stats(Unit* halUnit);																					// 004F6180
    s32 hallucinate_scarabs(Unit* a1, Unit* a2);																					// 004F65B0

    //// feedback_spell
    void replace_feedback_sprite(Unit* unit, u32 spriteId);																			// 004F6350

    //// weapon_effect
    bool is_unit_distance_within(Unit* target, Bullet* bullet, u32 radius);
    void do_weapon_hit(Unit* target, Unit* attacker, u8 dmgDivisor);
    void ai_unit_was_hit(Unit* self, Unit* attacker, u32 main_target_reactions, u32 dont_call_help);								// 0043F320

    // for air_splash
    Unit* find_best_unit(Box16* radius, s32 bullet, s32(*func)(Unit*, Bullet*));													// 004E8830
    s32 splash_func(Bullet* bullet, Unit* unit);																					// 0048B570

    // For iterate_loaded_units
    Unit* iterate_loaded_units(Unit* unit, Unit*(__fastcall* func)(Unit*, Unit*), s32 a3);											// 004E6D00

    //// mindcontrol_spell
    void mind_control_hit(Unit* attacker, Unit* target);																			// 004F6910

    //// weapon_damage
    u16 get_unit_strength(Unit* unit, bool use_ground_strength); // 00431800
    Unit* create_hallucination(Unit* target, u32 player);        // 004F6B90

    // For updateCorrosiveSpores
    u32 get_corrosive_spores_image(Unit* unit);																						// 004F41F0

    //// recharge_shields
    void insert_first_order(Unit* unit, u8 order_id);																				// 004749D0
    u8 has_overlay(Unit* unit);																										// 0047B720
    bool is_hangar_unit(Unit* unit);

    //// stopholdpos_orders
    Unit* acquire_medic_heal_target(Unit* medic);																					// 004422A0
    u32 do_medic_heal(Unit* unit, Unit* target);																					// 00463C40
    void order_to_attack_target(Unit* unit, Unit* target, u32 process_subunit, u32 target_is_cloaked);								// 00476FC0
    b32 attack_recent_attacker(Unit* unit);																							// 004770E0

    //// medic_orders
    b32 ai_order_heal(Unit* unit, Unit* target);																					// 00463530
    void perform_another_order_at_coords(Unit* unit, s32 x, s32 y, u8 order_id);													// 004748E0
    void update_target_something(Unit* unit, Unit* target, u8 order_id);															// 00474940
    void order_interrupt(Unit* unit, Unit* target, u16 x, u16 y, u32 order_id);														// 00474C30
    void unit_order_move_to_target_unit(Unit* unit, Unit* target);																	// 00479FE0

    //// load_unload_orders
    b32 ai_are_on_connected_regions(Unit* unit, s32 x, s32 y);																		// 00438E70
    void ai_unload_failure(Unit* unit);																								// 0043DF30
    void insert_order_before(Unit* unit, Order* order, u8 order_id, s16 x, s16 y, Unit* target, u16 target_unit_id);					// 00474540
    void do_order(Unit* unit, u32 order_id, s32 x, s32 y, bool stop_previous_orders);													// 00474C70
    Unit* get_loaded_unit_from_index(Unit* transport, s32 index);																	// 004E6C40
    b32 get_first_loaded_worker(Unit* transport);																					// 004E6D90
    b32 ai_find_transport(Unit* unit, Unit** unitTargetOffset);																		// 004E6EF0
    void load_unit_proc(Unit* unit, Unit* unitToLoad);																				// 004E78E0	(Hooked)
    b32 unload_unit_proc(Unit* unit);																								// 004E7F70 (Hooked)
    void issue_order_targeting_ground(Unit* unit, u32 order_id, u32 x, u32 y);														// 00475260

    //// siege_transform
    void set_unit_retreat_point(Unit* unit, u8 angle);																				// 004011A0
    bool get_closer_to_destination(Unit* unit);																						// 00402BE0
    void order_return_to_idle(Unit* unit);																							// 00463770
    bool check_for_close_unit(Unit* unit);																							// 00464290
    void play_subunit_iscript(Unit* unit, IscriptAnimation::Enum anim);																// 00474380

    //// unit_train
    u32 progress_build_get_hp_gain(Unit* unit);																							// 00402C40
    void refund_unit_train_cost(u32 unit_id, u8 player);																			// 0042CEC0
    void clear_building_ai_queue_slot(Unit* unit, u32 build_queue_slot);																		// 00432430
    void add_hangar_unit(Unit* main_unit, Unit* added_unit);																		// 00466300
    Unit* attempt_train_hatch_unit(Unit* trainer, u32 builtUnitID, u32 flag);														// 00468200

    //// weapon_fire
    // For progress_bullet_flight
    void do_missile_damage(Bullet* bullet);																							// 0048B770

    // For bulletBehavior_Bounce
    Bullet* change_move_pos(Bullet* bullet, s32 pos_y, s32 pos_x);																		// 00495240
    void progress_bullet_movement(Bullet* bullet);																						// 0048B250
    Unit* find_next_bounce_target(Bullet* bullet);																						// 0048B1E0

    // For find_next_bounce_target_proc
    bool unit_is_subunit(Unit* a1);																									// 00401D40
    bool unit_is_carrier(Unit* a1);																									// 00401470
    bool unit_is_reaver(Unit* unit);																								// 00401490
    bool unit_has_path_to_target(Unit* unit, Unit* target);																			// 0049CBB0
    s32 can_be_infested(Unit* unit, Unit* target);																					// 00402210

    // For find_next_bounce_target
    Unit* iterate_units_at_location_target_proc(Box16* a1, Unit*(__fastcall* a2)(Unit*, Unit*), Unit* a3);							// 004E8280

    // For create_bullet
    s32 initialize_bullet(char player, Unit* parent, s16 y, Bullet* bullet, WeaponId::Enum weapon_id, s32 x, s32 angle);						// 0048BEC0

    // For writeBullets
    /*
    void packBulletData(Bullet* a1, s32 a2); // 0048A9A0
    size_t _lockwrite(const void* a1, size_t a2, size_t a3, FILE* a4); // 00411931
    void* SMemAlloc(s32 a1, char* a2, s32 a3, s32 a4); // 0041006A
    s32 compressWrite(void* a1, u32 a2, FILE* a3); // 004C3450
    bool SMemFree(void* a1, char* a2, s32 a3, char a4); // 00410070
    Bullet* unpackBulletData(Bullet* a1, s32 a2); // 0048A8D0
    */

    //// weapon_spellhit
    bool can_unit_id_fit_at_xy(s32 x, s32 y, u32 unit_id);														    // 0042D810
    Unit** get_all_units_in_bounds(Box16* coords);																	// 0042FF80
    void increment_unit_kill_count(Unit* unit);																		// 004759C0
    u32 get_unit_damage_bonus(Unit* unit, WeaponId::Enum weapon_id);															// 00475EC0
    void attack_overlay_and_notify(Unit* attacker, Unit* target, u32 weaponType, u32 direction);			        // 00479730
    u32 get_unit_bullet_damage(Bullet* bullet, Unit* target);														// 0048ACD0
    u32 get_tech_use_error_message(Unit* target, u8 player, u16 tech_id);										    // 00491E80
    void disable_unit(Unit* unit);																				    // 00492CC0 (hooked in weapon_spellhit)
    void set_all_image_group_flags_palette_11(Sprite* sprite);														// 00497430
    void IterateUnitsAtLocationTargetProc_CorrosiveAcid(u32 attacking_player_id, Box16* coords);					// 004E8280
    void iterate_units_at_location_target_proc_ensnare(Unit* attacker, Box16* coords);								// 004E8280
    void iterate_units_at_location_target_proc_plague(Unit* attacker, Box16* coords);								// 004E8280
    void plague_proc(Unit* unit, Unit* attacker);																	// 004F4AF0
    void iterate_units_at_location_target_proc_maelstrom(Unit* attacker, Box16* coords);							// 004E8280
    void iterate_units_at_location_target_proc_stasis_field(Unit* attacker, Box16* coords);							// 004E8280
    void apply_ensnare(Unit* unit);																					// 004F45E0

    // For create_scanner
    s32 play_sound_from_unit(s32 sound_id, Unit* unit, s32 play = 1, s32 min_volume = 0);							// 0048ED50
    void return_all_interceptors(Unit* unit);																		// 004655F0

    // For maelstrom_proc
    void orders_self_destructing(Unit* unit);																		// 00475710
    char apply_maelstrom(Unit* unit, u8 time);																		// 004553F0

    // For CBulletDamage
    s32 _fastcall lakizilisk_splash_proc(Unit* target, Bullet* bullet);												// 0048B680
    s32 _fastcall ground_splash_proc(Unit* target, Bullet* bullet);													// 0048B750
    b32 _fastcall air_splash_proc(Unit* unit, Bullet* bullet);														// 0048B150
    s32 _fastcall air_splash_proc_2(Unit* unit, Bullet* bullet);													// 0048B730
    bool unit_has_no_main_order_type(Unit* unit);																	// 00475A50
    s32 set_unit_event_complete();																					// 004C36C0

    // For find_best_unit
    Unit** find_all_units(Box16* coords);																			// 004308A0

    //// selection
    bool unit_is_owned_by_current_player(Unit* unit); 																// 00401170
    bool is_unit_burrowed(Unit* unit);																				// 00402A70
    void sc_memcpy_0(u32 dest, u32 src, u32 memsize);																// 00408FD0
    void show_rally_target(Unit* unit);																				// 00468670
    bool unit_is_unselectable(u16 unit_id);																			// 0046ED80
    void function_0046F040(Unit* current_unit, Unit** unit_list, Unit* clicked_unit, u32 list_length);				// 0046F040
    u32 combine_lists(Unit* unit, Unit** unit_list_1, Unit** unit_list_2, u32 list_length);							// 0046F290
    void select_units_if_alt_not_held(Unit** unit_list, u32 unit_list_length, u8 play_sound, u32 send_command);		// 0046FA00
    u32 unit_to_unit_id(Unit* unit);																				// 0047B1D0
    bool unit_is_standard_and_movable(Unit* unit); 																	// 0047B770
    void select_building_sfx(Unit* unit);																			// 0048F910
    void create_new_unit_selections_from_list(Unit** unit_list, u32 unit_list_length);								// 0049AE40
    void cmdact_select(Unit** unit_list, u32 unit_list_length); 													// 004C0860

    //// cloak_nearby_units
    void secondary_order_cloak(Unit* unit);																			// 00491790
    void remove_overlay_range(u32 from, u32 to, Unit* unit);														// 00492370

    //// update_unit_state
    bool unit_hp_is_in_redzone(Unit* unit);																			// 004022C0
    void restore_all_unit_stats(Unit* unit);																		// 00492F70 (hooked by hooks\update_status_effects.cpp)

    // For update_units
    bool is_selected_unit_group_enabled();																			// 0049A2C0
    char refresh_layers_3_and_4();																					// 0048D9A0
    void cancel_target_order();																						// 0048CA10
    s32 update_disruption_web_status();																				// 004EB2F0
    Unit* progress_dying_unit_frame(Unit* unit);																	// 004EB5E0
    void reveal_sight_at_location(s32 sight_range, u32 vision_flags, s32 pt_x, s32 pt_y, bool is_air);				// 004806F0
    void update_visibility(Unit* unit);																				// 004EBE10
    void progress_active_unit_frame(Unit* unit);																	// 004EBC30
    void check_unit_visibility(Unit* unit);																			// 00443390
    void progress_unit_frame(Unit* unit);																			// 004ECF70
    void unit_update(Unit* unit);																					// 004EC420
    void update_unit_power();																						// 00493790

    // For update_burrowing_cloaking_units
    s32 is_unit_cloaking(Unit* unit);																				// 004916E0

    // For do_cloak
    void update_sprite_flag_on_palette(Sprite* sprite);																// 00497E80
    void show_images_hidden_by_cloak(Sprite* sprite);																// 00498170
    void update_sprite_images_for_cloak(Sprite* sprite);															// 00498A90

    // For progress_unit_frame
    char refresh_sprite(Sprite* sprite, s32 spriteId);																// 004EBBD0
    void update_unit_state(Unit* unit);																				// 004EC290
    void progress_order(Unit* unit);																				// 004EC4D0
    void perform_secondary_orders(Unit* result, s32 unused);														// 004ECF70
    void sprite_to_iscript_loop(Sprite* sprite);																	// 00497920

    //// select_larva

    u32 getLarvaeUnitsFromList(Unit** unitList, Unit** outputList);													// 00423190

    //// uifeatures

    s32 bwfxn_print_xy(const char* text, s32 x, s32 y);
    void get_color_handle(TextColor color);      // Colors: 1 - last color, 2 - 8, 14 - 17, 21 - 25, 27 - 31
    inline void get_color_handle(LobbyTextColor color) { get_color_handle((TextColor)color); };
    void set_font_handle(u32* font);																				// 0041FB30

    // For targeted_order
    void queue_new_order(u32 order_type, s16 pos_x, s16 pos_y, Unit* unit, u8 order_id, Unit* target,
                         u16 fow_unit_id, v2u16 position, Unit* secondary_target);		                        // 00474B90
    s32 show_stat_txt_to_player(const char* text, u32 player);														// 0048CF00

    // For select_units
    s32 select_single_unit_from_id(s32 index);																		// 00496D30
    s32 select_multiple_units_from_unit_list(u32 unit_count, Unit** selection, u8 play_sound, u32 send_command);	// 0049AEF0
    u8 send_turn();																									// 00485A40
    u8 packet_error_handle(s32 result, s32 a1, const char* a3, s32 a4, s32 a5);

    //// repair_order

    void get_repair_info(Unit* repairTarget, u32* repairCostMineral, u32* repairCostGas, s16* unkRepairInfo);		// 00466D10
    bool verify_resources(u32 mineralCost, u32 gasCost, u32 player);												// 00466EE0
    bool subtract_repair_resources(Unit* worker, Unit* target);														// 00466FB0

    // For is_order_valid
    bool unit_is_resource(Unit* unit);																				// 004688E0

    //// cosmonarchy_specific
    u8 getHangerTrainCount(Unit* unit);
    bool unit_is_training_or_morphing(Unit* unit);
    u8 initialize_randomizer_info();
    void ai_loop();
    s32 update_active_tile_info();
    s32 remove_fow_cheat();
    void update_units();
    s32 image_drawing_bullet_drawing();
    void update_images();
    void update_thingies();

    // create_unit
    Unit* unit_constructor(s32 x, s32 y, u16 unit_id, s32 player, s32 unused);										// 004A06C0
    Unit* visible_units_add(Unit* unit);																// 0049DCA0
    Unit* list_push_back_hidden_unit_entry(Unit* unit);																// 0049DCE0
    Sprite* unit_destructor(Unit* unit);																			// 004A0990
    void* get_image_attack_frame(void* result_arg, Sprite* sprite, s32 index, u32 overlay_type);					// 00401DF0
    void set_sprite_main_image_offset(Sprite* sprite, s8 x, s8 y);												    // 00401E40

    //// psi_field_util
    bool add_psi_field_sprite(Unit* unit);

    // stim_packs
    void queue_game_command(u8* params, u32 param_length);

    // misc
    Thingy* create_thingy(u32 sprite_id, u16 pos_x, u16 pos_y, u32 player); // 00488210
    bool do_units_collide(Unit* first, Unit* second);
    void unburrow(Unit* unit);
    Image* create_healthbar(Sprite* sprite);
    void remove_hp_bar(Image* a1);
    void refund_all_queue_slots(Unit* unit);
    
    void set_callback_timer(bw::Dialog* dialog, u16 timer_index, u32 time, TimerCallback callback);                 // 00416090
    inline void set_callback_timer(BinDlg* dialog, u16 timer_index, u32 time, TimerCallback callback) { set_callback_timer(dialog->to_dialog(), timer_index, time, callback); }
    
    void clear_callback_timer(bw::Dialog* dialog, u16 timer_index);                                                 // 00416000
    inline void clear_callback_timer(BinDlg* dialog, u16 timer_index) { clear_callback_timer(dialog->to_dialog(), timer_index); }
    
    void remove_from_selections(Unit* unit);																		// 0049A7F0
    u32 fxn_interact(BinDlg* dialog, u32 data_struct_offset);
    void register_user_dialog_action(BinDlg* dialog, BinDlgProc* interact_funcs, u32 count);                        // 00418100

    void enable_dialog(bw::Dialog* dialog);                                                                         // 00418E00
    inline void enable_dialog(BinDlg* dialog) { enable_dialog(dialog->to_dialog()); }
    
    void replay_stat_buttons(BinDlg* dialog);																		// 00427A80
    void update_command_card();																						// 004591D0
    void refresh_button_tooltip();																					// 00459770
    void bindlg_blit_surface(bw::Dialog* dialog);																		// 004C35F0
    s32 req_check(u32 requirement_func, u16 requirement_var, u32 player, Unit* unit);
    void update_image_position_offset(Image* image);

    // Identical to function @ 0x004F3AF0
    void set_resource_amount_carried(Unit* worker, u8 amount_carried, u32 chunk_image_id, bool is_mineral);

    // ai misc
    s32 ai_has_buildings_needing_power(u32 player);
    s32 ai_count_economy_units(u32 player, u32 unit_id);
    s32 get_supplies_remaining(u8 player, u16 unit_id);
    s32 get_supplies_used(s32 byte_data, u8 player);
    bool ai_isnt_at_define_max(s32 player, s32 unit_id);
    void apply_cloaking_order(Unit* unit);																	// 00491B30

    //// use_tech
    const void issue_tri_target_order(u8 order1, u8 order2, u8 order3);										// 0048CAF0
    s32 check_energy_for_tech_use(s32 tech_id);																// 00423660

    //// pathfinding
    s32 is_point_at_unit_border(s32 unused, Unit* other, Unit* self, v2u16 position_xy);						// 004EB720
    s32 ai_prepare_moving_to(Unit* unit, s32 x, s32 y);														// 004E7420

    // order attack behavior
    bool should_stay_at_range(Unit* unit);
    bool is_unreachable(Unit* target, Unit* unit);
    bool is_unit_target_out_of_max_range(Unit* unit, Unit* target);											// 00476430
    void move_for_melee_range(Unit* target, Unit* unit);													// 00477F10

    // For create_upgrade_level_tooltip
    s16 create_tooltip(char* text);																			// 004591A6
    char refresh_selection_screen();																		// 00481480

    // lobby.cpp
    
    // For init_game
    void switch_all_available_computer_slots_to_open();														// 004DBBE0
    bool set_last_open_slot_to_computer(s8 race);															// 004DBB70

    // For start_game
    void boot_reason(u8 player);																			// 004B8870
    bool is_authorized_map();                                                                               // 004C4870
    void set_timer_func();																					// 00472E00
    s32 get_tick_count();																					// 004FE0C4

    // For initialize_bullet
    Flingy* initialize_flingy(s32 flingy_id, s16 y, s32 x, Flingy* flingy, u8 player, s8 angle);		    // 00496360
    void bullet_behavior_reassign(Bullet* bullet);															// 0048BD50
    s32 weapon_behavior_can_hit_target_check(Unit* target, Unit* attacker);									// 004765B0
    void appear_on_target(Bullet* bullet, s16 x, s16 y);													// 004955C0
    Bullet* attack_fly_to_target(Bullet* bullet, s32 x, s32 y);												// 00494B70

    // For ai_progress_build
    void display_error_message_id_with_sfx(s32 player, s32 message_id, s32 sound_id);						// 0048EE30

    // For ai_manage_building_placement
    s32 get_bunker_placement_position(s32 placement_id, s32* region_id);									// 004442E0
    Unit* get_disabled_unit_in_range(s32 owner, v2u16 position);											// 00440420
    void ai_get_build_location(s32 unit_id, char* placement_data, s32 player, v2u16 try_position,			// 004461B0
        s32 max_range);
    void ai_placement_data_something(s8* placement_data, v2u16 try_position);								// 00443610
    u32 get_tile_distance(s32 x, s32 y, s32 x2, s32 y2);													// 00444100

    // For initialize_unit_counts
    char team_game_count_races(char force, char* bIsLegal, char race);										// 004857B0

    // For stat_res_timer_proc
    //void show_supply_info(BinDlg* dialog, s16 graphic, s16 mathA, u16 mathB);								// 004E53B0
    //void print_stat_res_info(BinDlg* dialog, u32 icon_graphic, u32* accrued_value, u32 resource_count, b32 do_not_accrue);	// 004E52E0

    // For redraw_supply_info
    bool unit_stat_cond_standard();																			// 00424980

    // For draw_supply_unit_status
    void draw_shield_upgrade(BinDlg* dialog, u16 status_screen_index);										// 00425510
    void draw_armor(BinDlg* dialog, s32 unit_id, u16 status_screen_index);									// 00425600

    // For progressBulletFrame
    void bullet_behavior_fly(Bullet* bullet);																// 0048B3B0
    void bullet_behavior_follow(Bullet* bullet);															// 0048B500
    void bullet_behavior_bounce(Bullet* bullet);															// 0048B2D0
    void bullet_behavior_persist(Bullet* bullet);															// 0048BC70
    void bullet_behavior_instant(Bullet* bullet);															// 0048A560
    void bullet_behavior_area(Bullet* bullet);																// 0048B440

    // For give_unit_to_player
    void give_all_unit_properties_to_player(Unit* unit, s8 player);											// 0047C9F0
    Unit* get_hidden_miner(Unit* unit);																		// 00468A60
    bool unit_has_hangar(Unit* unit);																		// identical to 004014B0
    s32 for_hangar_units(Unit* unit, Unit* (__fastcall* func)(Unit*, Unit*), s32 arg);						// 00465200
    Unit* get_creating_unit(Unit* unit);																		// 00466A30

    // For UnitMove
    void move_sprite(Sprite* sprite, s16 x, s16 y);															// 00497A10
    u32 update_unit_sorting_info(u32 x_diff, Unit* unit, u32 y_diff);										// 0046A000

    // For bullet_behavior_reassign
    void play_sprite_iscript(Sprite* sprite, char scriptId, s32 bPlay);										// 00499D00
    Bullet* assign_weapon_data(Bullet* bullet, char behavior_type);											// 0048AA80

    // For ai_training_normal
    AiRegion* get_ai_region_info_from_unit_loc(Unit* unit);														// 00438BF0
    void add_guard_ai_to_unit(Unit* unit);																	// 00462960

    s8 create_unit_allowed(u16 unit_id, Unit* unit, s32 player);											// 0046E1C0

    // For ai_military_should_wait_for_group
    bool ai_are_on_connected_regions(Unit* unit1, Unit* unit2);												// 00438E10
    bool ai_check_military_grouping(Unit* unit);															// 00439550

    // For remove_unit_ai
    s32 remove_from_ai_structs(Unit* unit, UnitAi* unitAI);													// 00439D60
    s32 remove_worker_or_building_ai(Unit* unit, b32 force_remove_building_ai);								// 00434C90
    void ai_remove_cleric(Unit* unit);																		// 004477C0

    // AI utility
    void give_ai(Unit* unit);																				// 00463040
    void ai_training_simple_military(Unit* new_unit, Unit* source_unit);									// 0043E280

    // For delete_ai_town_if_needed
    AiTown* try_begin_delete_town(AiTown* town);															// 00434330
    AiScript* delete_aiscripts_using_town(AiTown* town);													// 0045AE20

    // For display_establishing_shot
    void* load_init_credits_bin(const char* a1);															// 004D8DF0

    // For ai_are_on_connected_regions
    bool ai_regions_are_connected(u32 player, u32 region1, u32 region2);									// 00437E70

    // For btsncond_zerg_building_rally
    u32 btnscond_rally(u32 unused, u32 player, Unit* unit);													// 00429520

    // For ai_get_build_location
    u32 ai_get_building_placement_data(u32 max_range, u32 unit_id, char* placement_data, u32 player, v2u16 desired_pos);		// 00445D60
    char* ai_remove_unreachable_regions(char* placement_data, v2u16 desired_pos);												// 00443900
    u32 ai_remove_dangerous_regions(u32 unit_id, char* placement_data, u32 placement_id, v2u16 desired_pos);					// 00443870
    u32 ai_remove_far_tiles(char* placement_data, v2u16 desired_pos, u32 max_range);											// 00444E20
    u32 ai_remove_different_height(char* placement_data, v2u16 desired_pos);													// 004436C0
    u32 ai_placement_related_something(u32 unit_id, char* placement_data, v2u16 desired_pos);									// 00443E30
    void ai_remove_unfit_tiles(u32 unit_id, char* placement_data, u32 player, v2u16 desired_pos);								// 00444CE0
    u32 ai_remove_addon_places(u32 unit_id, char* placement_data, u32 player, v2u16 desired_pos);								// 00443CC0
    u32 get_tower_placement_position(char* placement_data, u32 player, v2u16 desired_pos, u32 unit_id, u32 unk_flag);		// 00443AC0
    u32 get_pylon_placement_position(char* placement_data, u32 player, v2u16 desired_pos);									// 004448D0
    u32 get_default_placement_position(u32 unit_id, char* placement_data, u32 player, v2u16 desired_pos);						// 004449A0

    // For ai_check_military_grouping
    void get_region_center_from_id(u16 region_id, u32* out_x, u32* out_y);														// 0049CAA0

    // For ignore_iscript_and_go_to_idle
    void play_idle_iscript(Sprite* sprite);																						// 00497880

    // For issue_order_targeting_ground
    void order_copy(s32 a1, Unit* a2, s8 a3, v2u16* a4, Unit* a5, s32 a6);													// 00447810

    // For unit_destructor_special
    s32 remove_creep_at_unit(s32 left, s32 top, s16 unit_id);																	// 00414560

    //// For direct_sound

    // For soundThread
    void fastFileRead(s32 a1, s32 a2, char* a3, s32 fileName, s32 buffer, s32 bytesRead, char* logFileName, s32 logLine);		// 004D2D10
    s32 parseWaveFile(s32 a1, s32 a2, s32 a3);																					// 004BBD80
    u32 get_sound_duration(s32 sound_id);																						// 004BB890

    /*
    // For soundInit
    void dSoundDestroy();																										// 004BC180
    s32 dSoundCreate(s32 a1);																									// 004BB640
    s32 setCoopLevel(s32 a1, s32 a2);																							// 004BB600
    s32 createSoundBuffer(s32 a1);																								// 004BB5A0
    s32 unkSoundCoreFunc();																										// 004BBA90
    s32 sFileDdaInitialize(char directSound);																					// 004101C0
    s32 sVidInitialize(char video);																								// 004101C6
    s32 loadButtonSFXFile();																									// 004BCB20
    void initVolume();																											// 004BC450
    HANDLE beginThreadEx(s32 lpThreadAttributes, s32 dwStackSize, s32 threadId, s32 a4, s32 dwCreationFlags, s32 lpThreadId);	// 0040CD3C
    */

    //// For ai_requests

    // For ai_perform_task
    void ai_overlord_behavior(Unit* unit);																						// 00434DF0
    void ai_nuke_silo_behavior(Unit* unit);																						// 00434D50
    bool ai_unit_construction_behavior(s32 player, Unit* unit, s32 someBool);													// 00433890
    char ai_plan_do_merge_morph(s32 player, Unit* unit, s32 someBool);															// 00435F10
    s32 ai_morph_behavior(s32 player, Unit* unit, void* unit_ai);																// 00434FF0
    void ai_train_behavior(Unit* unit, void* unit_ai, s32 player);																// 00434720
    void ai_upgrade_behavior(Unit* unit, s32 player);																			// 00434670
    s32 ai_research_behavior(Unit* unit, s32 player);																			// 0043462F - return value unused
    char ai_construct_behavior(Unit* unit, void* unit_ai, s32 player);															// 00434E70
    s32 ai_make_detectors_behavior(Unit* unit, s32 player);																		// 00435130

    // For ai_plan_do_merge_morph
    Unit* ai_get_mutalisk_for_morph(s32 player);																				// 004375F0
    s32 ai_get_high_templar_for_summon(s32 player, Unit** first, Unit** second);												// 00437410
    s32 ai_get_dark_templar_for_summon(s32 player, Unit** first, Unit** second);												// 00437320
    void assign_order_with_target(Unit* unit, Unit* target, s32 order_id);														// 004752B0
    s16 ai_reserve(s32 player, s32 someBool);																					// 00447D00
    AI_Main* ai_pop_spending_request(s32 player);																				// 004476D0
    Unit* AI_GetHydraliskForMorph(s32 player);																					// 00437570
    s32 is_unit_id_race_equal_to_unit_race(u16 unit_id, Unit* unit);															// 00432480
    bool has_resources_for_unit(u16 unit_id, s32 player);																		// 004487B0
    char ai_perform_task(Unit* unit);																							// 004361A0
    s32 ai_get_request_build_type(s32 player);																					// 00447590
    bool ai_train_unit(Unit* unit, u16 unit_id, char requestId, void* value);													// 004672F0
    
    // For scenario.cpp
    Unit* ai_create_unit(u32 unit_id, s32 player, s32 x, s32 y);																// 004A0AD0
    bool is_ai_script_name_valid(u32 player, u32 name);																			// 0045AEA0
    s32 parse_aiscript_name(s32 name, s32* is_bw);																				// 0045AEF0
    AiScript* run_aiscript(Box32* area, u32 player, u32 block, u32 is_bw);														// 0045B170
    void ai_stop(AiScript* ai_script);																						// 00403380

    AiTown* aiscript_start_town(u32 x, u32 y, u32 resarea, u32 player);															// 00434220
    AiTown* aiscript_start_areatown(Box32* area, u32 resarea, u32 player);														// 00434060
    void ai_update_town_regions(u32 player);																				    // 0045B810

    typedef s32 __cdecl VictoryFunc();
    extern VictoryFunc* Victory;

    void display_text_message(char* text, u32 color, u32 ticks, b32 from_trigger);
    u32 get_text_display_time(char* text);

    // For prepare_unit_move_custom
    s32 sort_unit(Unit* unit);																									// 0046A300
    void refresh_grp_image(Sprite* sprite);																						// 00497530

    //// misc
    s8 SETS(s32 x);
    s8 OFSUB(s32 x, s32 y);

    char* get_chk_string(u32 i);

    typedef void   __cdecl playNextMusic_Func();
    typedef HANDLE __cdecl stopMusic_Func();

    extern playNextMusic_Func* play_next_bw_music;
    extern stopMusic_Func* stop_bw_music;

    void prepare_formation_movement(MovementGroup* movement_group, bool can_be_obstructed);						// 0049A8C0
    Unit* find_fow_unit(u16 y_pos, u16 x_pos, u32 fow_unit_id);													// 00487FD0
    void targeted_order_unit(Unit* a1, Unit* a2, int a3, char a4);												// 004756E0
    void targeted_order_ground(int a1, int a2, Unit* a3, int a4, int a5, int a6);								// 004756B0
    Unit* get_next_commanded_unit();																			// 0049A850
    void get_formation_movement_target(Unit* unit, MovementGroup* movement_group);								// 0049A500

    u8 get_right_click_order(u32 type, Unit* unit, s32 x, s32 y, Unit** target, s32 unit_id);

    void edit_unit_flags(Unit* unit, u16* flags);																// 004CD480
    void unit_placement_error(const char *a1, char a2);

    void ai_build(u32 unit_id, b32 only_if_needed, u32 count, AiTown* town, u32 priority);						// 004323D0
    void ai_add_build_request(AiTown* town, s32 method, s32 count, s32 id, s32 priority, b32 only_if_needed);	// 00431F90

    // For HandleWindowMessages
    char check_last_file_error();																				// 004D28D0
    s32 has_messages_waiting(void* tag_msg, s32 b_something);													// 004D1650
    void send_mouse_move_event(void* fast_call_func);															// 004D1AE0
    s32 send_input_to_all_dialogs(DialogEvent* dlg_event);														// 00419FD0
    VectorHead* play_briefing_wav_begin();																		// 004BBE50
    void iterate_timers();																						// 00415DC0

    // For ai_train_defensive_military
    u32 ai_region_train_attack_force(AiRegion* region);															// 0043E670
    u32 ai_defend_region(AiRegion* military_region, s32 defense_region, s32 against_air, s32 region_strength);	// 0043F100
    u32 ai_add_base_unit_spending_request(u32 final_unit_id, u32 player_id, AiRegion* region, u8 priority);		// 004484A0

    void play_sound_global(u32 id);										// 004BC2C0
    VectorHead* play_sound_from_file(const char* filename, char* unused);				// 004BC490

    b32 is_valid_for(u32 a1, Unit* unit, u32 a3);
    b32 region_is_for_attack_prepare(AiRegion* region);

    u32 ai_count_units_in_town(AiTown* town, u32 unit_id, bool finished);								// 00433210 Aka AI_WaitBuild

    u32 load_scenario_singleplayer(char* title, u32 unused, char* error, char* player, u32 speed);		// 004A8590
    u32 get_ground_height_at_px(s32 y, s32 x);															// 004BD0F0
    s32 cannot_chase_target(Unit* attacker, Unit* target);												// 004A1140
    s32 get_target_priority(Unit* attacker, Unit* target);												// 00442160
    u32 add_unit_to_possible_targets(s32 threat_level, Unit* unit);										// 00440160

    void enable_doodad_state(Unit* unit);																// 0047BA00
    void disable_doodad_state(Unit* unit);																// 0047B960
    char unfreeze_unit(Unit* unit);																		// 004F62D0
    s32 dialog_set_progress_bar_value(BinDlg* dialog, s32 index, u32 value);							// 00457200

    // For vespene rework in harvest.cpp
    void set_gathering_flags(Unit* unit);																// 00468AA0
    bool get_gas_exit_point(Unit* worker, Unit* resource_node, v2u16* position, s32* direction);		// 00468B00
    void move_unit(Unit* unit, s16 x, s16 y);															// 004EB9F0
    Unit* finished_mining(Unit* worker, Unit* resource_node);											// 00468E40
    Order* delete_order(Unit* unit, u8 order_id);														// 00474400
    void order_targeting_nothing(u8 order_id, Unit* unit, u32 stop_previous_orders);					// 00474D60
    void hide_unit_partial(Unit* unit, s32 hide_unit);													// 00493CA0

    void order_begin_harvest(Unit* node, Unit* worker);													// 00468D10
    Unit* remove_harvesters(Unit* node);																// 00468770
    Unit* remove_from_current_harvesters(Unit* worker, Unit* node);										// 004686D0

    s32 ai_worker_repair_controller(Unit* unit, AiTown* town);											// 00433C70
    s32 ai_try_progress_spending_queue_worker(AiTown* town, Unit* unit);								// 004348C0
    bool unit_is_mineral_field(Unit* unit);																// 00468890
    void ai_auto_harvest(Unit* resource, Unit* worker, Unit* fallback);									// 00433B90
    Unit* harvest_next_nearest_resources_ex(s32 worker_count_bool, Unit* worker, v2u16 worker_pos);	// 004432D0
    Unit* get_ai_gas_node(AiTown* town, s32 gas_state, s32 index);										// 00432B90
    u32 ai_get_vespene_type(Unit* node, s32 player_id);													// 004326E0
    void ai_resource_need_split(u8* out_gas_percentage, u32 player, u8* out_mineral_percentage);		// 004475B0
    u32 ai_add_town_spending_request(u32 player, u32 priority, u32 unit_id, void* param);				// 00448400

    u32 get_closest_base_from_startloc(u32 player_id, u32 x, u32 y);									// 00443A20
    Unit* get_resource_area_gas(u32 resarea, Unit** gas_1, Unit** gas_2, Unit** gas_3);					// 00444460
    Unit* get_resource_area_mineral(u32 resarea);														// 004445E0
    u32 get_optimal_mineral_workers(u32 resarea, u32 player);											// 004439B0
    void ai_get_old_peons(AiTown* town, u32 count);														// 004355C0

    bool target_is_inside_minimum_range(Unit* unit, Unit* target);										// 004764D0
    bool is_unit_enemy(Unit* unit, Unit* target);														// 0047B910
    u32 play_transmission_from_location(u32 sfx_id, u32 volume, u32 misc, Unit* unit);					// 004BC270
    s32 update_text_display(s32 update_case);															// 0048CB80

    typedef void __fastcall BTNSACT_ChangeButtons_Func(s16 id);
    extern BTNSACT_ChangeButtons_Func* BTNSACT_ChangeButtons;

    typedef char __cdecl updateSelectedUnitData_Func();
    extern updateSelectedUnitData_Func* updateSelectedUnitData;

    void update_hud_info();																					// 004D93F0

    void remove_powerup_image_overlay(Unit* worker);														// 004F3900

    void clear_tile_flags_from_area(u32 flags, u32 tile_width, u32 tile_height, u32 tile_x, u32 tile_y);	// 00472E50
    void set_map_tile_flags(u32 flags, u32 tile_width, u32 tile_height, u32 tile_x, u32 tile_y);			// 00472EB0
    void set_creep(u32 tile_x, u32 tile_y, u32 tile_width, u32 tile_height, b32 skip_border);				// 0045CD50

    s32 get_creep_spread_area(u32 unit_id, u32 x, u32 y, b32 completed, Box32* rect);						// 00413DB0

    typedef s32 ClipCreepAreaInBounds_Callback(u32, u32, u32, u32, u32, u32, u32);
    s32 clip_creep_area_in_bounds(u32 tile_x, u32 tile_y, ClipCreepAreaInBounds_Callback* callback);		// 00413D10

    s32 mark_cagra_to_disappear(u32 tile_x, u32 tile_y);													// 0047DD60

    void* process_options_change(BinDlg* control);															// 004C9780
    void load_dialog(const char* filename, void* function);													// 004F57A0
    void destroy_dialog(bw::Dialog* dialog);                                                                // 00419EA0
    inline void destroy_dialog(BinDlg* dialog) { destroy_dialog(dialog->to_dialog()); }

    typedef void main_menu_options_custom_interact_func(BinDlg* control);
    extern main_menu_options_custom_interact_func* main_menu_options_custom_interact;

    typedef bool __fastcall game_menu_dialog_interact_func(BinDlg* control, DialogEvent dlg_event);
    extern game_menu_dialog_interact_func* game_menu_dialog_interact;

    u32 stop_creep_disappearing(u32 x_tile, u32 y_tile);													// 0047D540

    void secondary_order_spawning_larva(Unit* unit);														// 004EA780

    AiTown* create_ai_town(u32 player, u32 x, u32 y);														// 00432020

    Sprite* sprite_destructor(Sprite* sprite);																// 00497B40
    void reassign_path(Unit* unit);																	        // 0042F740
    u32 initialize_unit(Unit* unit, u32 y, u32 unit_id, u32 x, u8 player);  								// 0049F206
    void update_hp_gain(Unit* unit);																		// 0047B180

    bool unit_is_geyser_ex(Unit* resource_node, Unit* worker);												// 00468930
    void prepend_order(Unit* unit, u8 order_id, v2u16 target_pos, Unit* target_unit);						// 00474AF0
    bool ai_is_building_safe(Unit* building);																// 00438910
    Unit* ai_find_best_repair_unit(Unit* unit);																// 00443160

    u32 ai_find_expansion(u32 player, u32 resarea, u32 amount);												// 004453E0

    b32 ai_try_expanding_to(u32 resarea);																	// 00443A00

    u8 get_seek_range(Unit* unit);													// 00476000

    Unit* let_next_unit_mine(Unit* worker);																	// 00468970
    s32 begin_harvest(Unit* worker, Unit* node);															// 00468C70

    u32 get_missing_attack_force_units(AiRegion* ai_region, u32* attack_wave, u32 is_campaign);				// 00438050
    s32 ai_defender_bunker_behavior(Unit* unit, u16 region_id);												// 0043C480

    bool ai_has_incomplete_military_in_region(AiRegion* ai_region);											// 00436EB0

    void delete_military_ai(MilitaryAi* military_ai, AiRegion* ai_region);									// 004371D0

    bool ai_has_incomplete_military_in_region(AiRegion* region);											// 00436EB0

    void update_burrowing_cloaking_units();																	// 00492EB0
    s32 use_weapon_iscript(Unit* unit, WeaponId::Enum weapon_id, u32 flag_or_timer, char iscript_anim);				// 00476C90

    bool place_building_resource_restriction_check(u32 check_vision, u16 tile_x, u16 tile_y);				// 00473990

    Image* create_image(Sprite* sprite, u32 id, s32 h_offset, s32 v_offset, u32 direction);					// 00498F40
    Sprite* create_sprite(u32 sprite_id, u32 x, u32 y, u32 player);											// 004990F0

    void refresh_all_visible_images_at_screen_position(Sprite* sprite, u8 vision_filter);					// 00497480

    void set_tooltip_info(u32 mineral_cost, u32 vespene_cost, u32 p_energy, u32 t_energy, u32 z_energy, u32 p_supply, u32 t_supply, u16 z_supply); // 00458800

    void bw_print_icon(Bounds* bounds, FxnDrawIconData* data, int pos_x, int pos_y);						// 0041D090
    void bw_print_xy(const char* text, s32 pos_x, s32 pos_y);												// 004202B0

    bool unit_is_in_notify_range(Unit* unit);																// 004E5AD0
    u32 reset_all_unit_attack_notify_timers_in_range(Unit* unit);											// 0048EAE0
    void do_unit_event_notification(Unit* unit, char event_type, u32 timer, bool ping, u32 message_id);		// 0045EFE0

    s32 stat_data_unit_wireframe_select_update(BinDlg* dialog, Event* event_arg);							// 00456F50
    char status_screen_update_tooltip_mouse_held(BinDlg* dialog, DialogEvent* dialog_event);				// 00457DE0
    u32 status_screen_button(BinDlg* dialog_1, BinDlg* dialog_2);											// 00458220

    void status_screen_draw_stat_button_border(BinDlg* dialog);												// 00456D30
    u32 status_screen_wireframe_coloring_related(Unit* unit);												// 004567C0
    char status_screen_wireframe_shield_related(Unit* unit);									            // 004566B0
    u32 status_screen_wireframe_hp_related(Unit* unit);														// 00456730

    char toggle_unit_pathing_properties(Unit* unit);														// 004030D0

    u32 unit_is_at_height(Unit* unit, u32 height);															// 0045F8D0
    bool kill_units_at_location_proc(Unit* unit, KillUnitArgs* args);										// 004C7E20
    void kill_powerup(Unit* unit);																			// 004F3C60

    void send_message_something(u32 unused, const char* string);											// 004F31D0
    u32 set_ingame_input_procs();																			// 00484CC0
    void update_current_buttonset();																		// 004599A0
    bool is_unit_owned_by_player_ex(u32 player, Unit* unit);												// 0045FEF0

    char issue_place_building_order(char* unused, u32 unit_id, u8 order_id);								// 0048E730

    bool compare_unit_rank(Unit* unit, Unit* compare_unit);													// 0049A350

    void do_targeted_order(u32 x, u32 y, Unit* target, u32 fow_unit_id);									// 004A55F0
    void minimap_game_target_order(Event* event_arg);														// 004A55F0
    char input_target_order_left_mouse_click(Event* event_arg);												// 004BD500

    char input_place_building_left_mouse_click(Event* event_arg, u32 unused, v2u16 position);				// 0048E5D0

    void initialize_input_procs();																			// 004D2FF0

    u16 get_minimap_cursor_pos(u16* x, u16* y);																// 004A3D70

    void update_palette();																					// 0041E5C0

    u32 order_acquire_generic_idle_unit_proc(Unit* unit, u32 x_pos, u16 y_pos, Unit** target, u16 fow_unit_id);	// 00455900
    u32 order_acquire_generic_right_click_harvest_proc(Unit* node, Unit* worker);								// 004557C0

    char get_generic_unit_idle_order(u32 order_unit_id, Unit* unit);										// 00455760
    Unit* iterate_units_at_location_target_proc_in_point(Box16* location, void* func, Unit* target);		// 004E8280
    void set_rally_unit(Unit* factory, Unit* target);														// 00466B40
    void set_rally_position(Unit* factory, u16 x_pos, u16 y_pos);											// 00466B40

    s32 is_outside_of_game_screen(u32 current_x, u32 current_y);											// 004D1140
    bool accepts_right_click_commands(Unit* unit);															// 004556D0
    Unit* find_unit_at_point(u32 x_pos, u32 y_pos);															// 0046F3A0
    s32 can_move(Unit* unit);																				// 00455660
    Thingy* show_command_response(u32 x_pos, u16 y_pos, Unit* unit);										// 00456490
    char cmdact_right_click_order(Unit* target, u32 x_pos, u32 y_pos, u32 unit_id, u32 keycode_used);		// 004C0380
    s32 show_right_click_error_if_needed(Unit* target);														// 00455A00
    void play_yes_unit_response(Unit* unit);																// 0048F4D0
    u32 sort_all_units(Unit** units, Unit** units_2, Unit* unit);											// 0046F0F0

    char tech_is_researched_bw(u32 player, u16 tech_id);													// 004CE8A0

    void drone_build_execute(Unit* unit);																	// 0045D2E0

    Unit* find_unit_at_tile(u16 x_pos, u16 y_pos, u32 unit_id); 											// 004734B0
    void inherit_gas_overlays(Sprite* sprite);																// 004D5BB0

    // For create_all_selection_circles
    Image* create_selection_circle(Sprite* sprite, u8 color, u16 image_id);									// 004D7070

    // For unit_stat_action_transport
    Unit* transport_status_update_drawn_values();															// 00424FC0
    void set_spell_special_button_graphic(BinDlg* dialog);													// 00424BA0
    void get_active_portrait_unit_name(BinDlg* dialog);														// 00425EC0
    void unit_stat_action_building(void* dialog);															// 00427890
    void kill_all_loaded_units(Unit* unit);																	// 0049FDD0

    // For trigact_transmission
    s32 trigact_play_wav(TriggerAction* action);															// 004C77D0
    Unit* get_doodad_for_unit_state(u8 location, u16 unit_id, u32 player);									// 004C7380
    u8 minimap_ping(u32 x_pos, u32 y_pos, u32 color);														// 004A34C0
    void game_talking_portrait(u32 x_pos, u32 time, u32 unit_id, u32 y_pos);								// 0045EDD0
    void create_text_message_with_timer(const char* string, u32 timer);										// 0048CF20

    // For select_unit_initial
    void remove_selection_circle_image(Image* image);														// 004D4FA0
    char illegal_team_check(Unit* unit);																	// 00485710

    // For ai_update_guard_needs
    void delete_guard_ai(GuardAi* guard_ai, GuardAiList* guard_ai_list);									// 00403030
    u32 ai_request_guard(GuardAi* guard_ai, u32 player);													// 00448630
    Unit* find_nearest_available_military(u32 player, u32 unit_id, v2u16 point);							// 004379B0

    void create_bunker_attack_overlay(Unit* unit);															// 00477FD0
    
    void draw_unit_box(s32 x_pos, s32 y_pos, u32 x_dims, u32 y_dims, u8 color, bool is_building);			// 004A3FD0

    void draw_minimap_unit_box_2(u32 player);																// 004A47B0
    char draw_minimap_unit_box(u32 player);																	// 004A4650
    void draw_all_minimap_unit_boxes(u32 player);															// 004A48E0
    u32 thingy_visiblity_check(u32 check_not_visible, Thingy* thingy);										// 00487CF0

    u16 get_remaining_build_time_percentage(Unit* unit);													// 004669E0

    s32 get_sfx_volume_from_screen_distance(u32 x, u32 y);													// 0048E8D0
    s32 get_sfx_pan_from_screen_distance(u32 x);															// 0048E850

    [[noreturn]] void fatal_file_not_found(u32 code, const char* filename);                                 // 004212C0

    void EAX_order(Unit* unit, u32 order_func_offset);
    void EBX_order(Unit* unit, u32 order_func_offset);
    void ECX_order(Unit* unit, u32 order_func_offset);
    void EDI_order(Unit* unit, u32 order_func_offset);
    void ESI_order(Unit* unit, u32 order_func_offset);
    void PUSH_order(Unit* unit, u32 order_func_offset);

    bool is_better_target(Unit* self, Unit* auto_target, Unit* compare_unit);								// 004769D0
    void react_to_hit(Unit* attacker, Unit* victim);														// 004776C0
    u8 ask_for_help(Unit* attacker, Unit* helper);															// 00476E60

    s32 play_loaded_sound(u32 loaded_sound, s32 volume, s32 pan, u32 sound_id, u32 unit_id, u32 sound_flags, u32 some_bool, u32 priority);	// 004BBAF0
    s32 queue_sound(u32 sound_id, s32 volume, s32 pan, u32 unit_id);																		// 004BBEB0

    bool unit_finished_summon(Unit* unit);																	// 00436E40
    u32 get_total_hp_plus_shields(Unit* unit);																// 00402710

    bool unit_is_morphing(Unit* unit);																		// 0045CD00
    char cmdact_unload(Unit* unit);																			// 004BFDB0
    Target ai_prepare_to_flee(Unit* target, Unit* attacker);												// 00476A50

    bool unit_is_enemy(Unit* other);																		// 004D1380

    u32 can_target_order_on_fow_unit(Unit* self, char order_id, u16 fow_unit_id, s32* error);						// 004746D0
    bool unit_can_attack_ground_type(u16 fow_unit_id, Unit* self);													// 00475C80
    u16 get_targeting_error_string(WeaponId::Enum weapon_id, s32* error, Unit* target);						    	// 0046EFE0
    char cmdact_target_order(u8 order_id, Unit* target, u16 x_pos, u16 y_pos, u16 fow_unit_id, char keycode_used);	// 004C0300

    void show_cursor_marker(u16 x_pos, u16 y_pos);															// 00488660
    void bw_sprintf_s(char* buffer, int buffer_size, const char* format_string, int value);					// 0041F1B0

    Image* set_image_palette_type(Image* out_image, u8 palette_type);										// 004D58B0

    u16 set_iscript(Image* image, u32 iscript_id);															// 004D6640

    Image* update_image_frame_index(Image* image);															// 004D5940
    u32 get_path_distance_to(Unit* unit, v2u16 position);													// 0042FA30

    void fire_unit_weapon(Unit* unit, WeaponId::Enum weapon_id);																					// 00479C90
    Thingy* ufl_unstable_randomize(Flingy* flingy);																						// 004955F0
    void do_weapon_damage(u32 damage, Unit* target, WeaponId::Enum weapon_id, u32 divisor, char direction, Unit* attacker, u32 attacking_player);	// 00479930
    Image* iscript_create_image(Image* relative_image, u32 image_id, char x, char y, u8 draw_above);									// 004D6D90
    u32 iscript_use_lo_file(void* out_coords, Image* image, u8 a3, u8 frame_offset);													// 004D5770
    void hide_image(Image* image);																										// 00401100
    u16 show_image(Image* image);																										// 00401120
    Image* image_destructor(Image* image);																								// 004D4CE0
    void flip_image(Image* image, char direction);																						// 004D5F30
    void iscript_play_sound(Image* image, u32 sound_id);																				// 004D6610
    void turn_unit_left(Unit* unit, char amount);																						// 00496170
    u32 get_difference_between_points(char point_1, char point_2);																		// 00494BD0
    u32 get_modified_unit_speed(u32 speed, Unit* unit);																					// 0047B5F0
    Image* get_empty_image();																											// 004D4E30
    Image* images_insert(Image* image, Image* next, Image* prev);																		// 0047A070
    void some_play_image_crap_that_crashes(char x_pos, char y_pos, Image* image, Sprite* sprite, u32 image_id);							// 004010E0
    u32 get_closest_point_of_target(Unit* unit, u32 out_x, u32 out_y);																	// 004762C0

    bool unit_has_path_to_destination(Unit* unit, s32 pos_x, s32 pos_y);																// 0049CB60
    Unit** get_all_units_in_region(u16 region_id);																						// 00430B50

    void subtract_queue_cost(u8 player);																								// 0042CE40
    Unit* get_unit_powerup(Unit* unit);																									// 004014E0
    void droleth_build_excisant(Unit* unit);																							// 0045D790

    void cancel_queued_trains(AiTown* town);																							// 004326A0
    u32 get_upgrade_count(AiTown* town, u32 upgrade_id);																				// 004325A0
    u32 get_tech_count(AiTown* town, u32 tech_id);																						// 00432530
    u16 ai_build_bunker(AiTown* town, u32 player);																						// 0043EBB0
    u32 ai_build_turret(AiTown* town, u32 player);																						// 0043AAE0

    s32 get_bunker_position(u32 player, v2u16* return_pos);																			// 004376F0
    void do_area_fixups_for_enemy(u32 player, u16 region_id, u32 iterations);															// 00444EB0
    s32 ai_manage_building_placement(Unit* builder, u32 unit_id, v2u16 try_pos, v2u16* out_pos, s32 area_tiles);					// 004465C0
    u32 ai_finish_unit(Unit* unit);																										// 00433640
    s32 get_turret_position(u32 player, v2u16* out_pos);																				// 00437670
    void get_secondary_hachirosk_build_location(u32 resarea, u16* out_x, u16* out_y);													// 00444630

    char update_buttonset();																											// 00458DE0

    void refresh_image(Image* image);																									// 004970A0
    void compile_healthbar(Image* image, Sprite* sprite);																				// 004D6010
    void create_selection_circle_image(Image* image, Sprite* sprite, u8 color, u16 image_id);											// 004D6810

    b32 return_interceptor_if_cannot_attack(Unit* steward);																				// 00465810
    u32 get_next_strafe_position(Unit* unit, u8 strafe_type, v2u16 pos);																// 00465D30
    void get_weapon_iscript_init(Unit* unit, IscriptAnimation::Enum animation);															// 00476ED0

    b32 set_temp_iscript_animation(IscriptAnimation::Enum animation);																	// 0042D600

    u32 get_visibility_mask_from_position_and_size(s16 x_pos, s16 y_pos, s16 width, s16 height);										// 00402570

    Image* copy_image_palette_type(Image* image, Image* from);																			// 004D5E40

    bool orders_carrier_reaver_idle(Unit* unit, u32 range1, u32 range2);												// 00465A60
    b32 can_draw_placement_rect_maybe(u16 unit_id, Unit* unit);												// 00436A10
    Image* prepare_placement_preview();																		// 004D5B60
    void place_building_normal_proc();																		// 0048E490
    void place_building_refinery_proc();																	// 0048E430
    u32 prepare_placement_box(b32 is_addon, u32 unit_id);													// 0048DAF0
    void refresh_screen();																					// 0048DDC0

    void update_unit_damage_overlay(Unit* unit);															// 004E6090

    s32 get_recv_string(u32 string_id, u32 data_len, u32 buffer, char* string, s32 string_type);			// 00485980
    void CMDRECV_save_game(char* buffer);																	// 004C2910
    FILE* CMDRECV_load_game(char* buffer);																	// 004CF950
    s8 CMDRECV_restart_game();																				// 004BFB10
    void CMDRECV_select(char* buffer);																		// 004C2750
    void CMDRECV_shift_select(char* buffer);																// 004C2560
    void CMDRECV_shift_deselect(char* buffer);																// 004BFB40
    u16 CMDRECV_build(s8* buffer);																			// 004C23C0
    s8* CMDRECV_vision(s8* buffer);																			// 004BF9C0
    void CMDRECV_ally(char* buffer);																		// 004C28A0
    void CMDRECV_pause_game();																				// 004C0BC0
    void CMDRECV_resume_game();																				// 004C0B00
    s32 CMDRECV_use_cheat(char* buffer, char* cheat);														// 004C0AD0
    u8 CMDRECV_hotkey(char* buffer);																		// 004C2870
    Unit* CMDRECV_right_click(Unit* unit);																	// 004C2370
    void CMDRECV_target_command(char* buffer);																// 004C2320
    char CMDRECV_cancel_construction();																		// 004C2EF0
    Unit* CMDRECV_cancel_unit_morph();																		// 004C2EC0
    Unit* CMDRECV_stop(s8* buffer);																			// 004C2190
    Unit* CMDRECV_carrier_stop();																			// 004C1430
    Unit* CMDRECV_reaver_stop();																			// 004C1240
    Unit* CMDRECV_nothing();																				// 004C1050
    Unit* CMDRECV_return_cargo(char* buffer);																// 004C2040
    s16 CMDRECV_train(char* buffer);																		// 004C1C20
    s16 CMDRECV_cancel_train(s8* buffer);																	// 004C0100
    Unit* CMDRECV_cloak();																					// 004C0720
    Unit* CMDRECV_decloak();																				// 004C0660
    s32 CMDRECV_unit_morph(char* buffer);																	// 004C1990
    Unit* CMDRECV_unsiege(char* buffer);																	// 004C1F10
    Unit* CMDRECV_siege(char* buffer);																		// 004C1E80
    Unit* CMDRECV_train_fighter();																			// 004C1800
    Unit* CMDRECV_unload_all(s8* buffer);																	// 004C1CC0
    char CMDRECV_unload(Unit* unit);																		// 004C24F0
    Unit* CMDRECV_merge_archon();																			// 004C0E90
    Unit* CMDRECV_hold_position(char* buffer);																// 004C20C0
    Unit* CMDRECV_burrow(char* buffer);																		// 004C1FA0
    Unit* CMDRECV_unburrow();																				// 004C1AC0
    Unit* CMDRECV_cancel_nuke();																			// 004BFCD0
    void CMDRECV_liftoff(s8* buffer);																		// 004C1620
    Unit* CMDRECV_research(char* buffer);																	// 004C1BA0
    Unit* CMDRECV_cancel_research();																		// 004C0070
    Unit* CMDRECV_upgrade(char* buffer);																	// 004C1B20
    void CMDRECV_cancel_upgrade();																			// 004BFFC0
    Unit* CMDRECV_cancel_addon();																			// 004BFF30
    char CMDRECV_building_morph(char* buffer);																// 004C1910
    Unit* CMDRECV_stim_pack();																				// 004C2F30
    b32 CMD_game_hash(HashPacket* packet);																	// 0047CDD0
    void CMDRECV_voice_enable(s8* buffer);																	// 004BF820
    s32 CMDRECV_set_latency(char* buffer, s32 unused);														// 00485E60
    s32 CMDRECV_set_replay_speed(s32 speed, u8 is_paused, u32 modifier);									// 004DEB90
    char CMDRECV_leave_game(char* buffer);																	// 004C2E90
    char CMDRECV_minimap_ping(char* buffer);																// 004BF7A0
    Unit* CMDRECV_merge_dark_archon();																		// 004C0CD0
    void bwfxn_display_text(s32 player, s32 text, s32 is_replay);											// 0048D1C0
    void save_replay_action(void* game_actions, void* buffer, u32 action_size, char player);				// 004CDE70

    char create_dashed_selection(u32 player, Sprite* sprite, char color);									// 00499A10

    b32 is_melee_game(char game_type, s16 game_type_param, char game_type_2);								// 004AAFA0
    void randomize_player_slot();																		// 004A9A30

    void place_building_refinery_check(Unit* unit);															// 0048E1E0

    void move_screen(v2u16 pos);																			// 0049C440

    b32 is_at_home(Unit* unit);																				// 00462930
    u32 is_under_carapace_swarm(Unit* unit);																// 00476210

    void iscript_fireweapon(Unit* unit, WeaponId::Enum weapon_id);														// 00479D40
    void iscript_useweapon(Unit* unit, WeaponId::Enum weapon_id);														// 00479C30

    void draw_button_graphic(BinDlg* dialog, void* dest);													// 004F0050
    void draw_alternate_button_graphic(BinDlg* dialog1, BinDlg* dialog2, void* dest);						// 004EFEB0
    void draw_standard_button_graphic(BinDlg* dialog1, BinDlg* dialog2, void* dest);						// 004EFD90
    Unit* get_best_unit(Unit* unit, Box16* dimensions, bool(__fastcall* func)(Unit*, Unit*), Unit* unit2);	// 004E8740

    u32 draw_box(u16 x, u16 y, u16 w, u16 h);																// 004E1D20
    s32 copy_last_replay(char* name);																		// 004DED30

    void insert_order_targeting_ground_before(u8 order_id, Order* order, Unit* unit, u16 pos_x, u16 pos_y);	// 00474790
    void unit_order_move_to_target_reset_order_state(Unit* unit, u8 order_id, Unit* target);				// 00479FA0
    void issue_order_targeting_fow(Unit* unit, u8 order_id, u32 fow_unit_id, u16 pos_x, u16 pos_y);			// 00474FB0

    b32 draw_clamped_dialog_image(Box16* clip, u32 sprite_index, u32 pos_x, u32 pos_y, u32 flags);		    // 004EF600
    u8* bw_draw(u16 x_pos, u32 y_pos, u32 x_clamp, u16 y_clamp);											// 004E1C70
    void dialog_draw_text(void* arg, BinDlg* button, u32 flag1, u32 flag2);									// 004EF870

    b32 draw_dialog_image(Box16* dims, void* bitmap, u32 width, u32 height, u32 flags);						// 004EF440

    void image_render_fxn_0_0(u32 screen_x, u32 screen_y, void* image_data, Box32* rect, u32 color_data);	// 0040ABBE

    void draw_func_normal(u32 screen_x, u32 screen_y, void* image_data, Box32* rect, u32 color_data);	    // 0040ABBE

    void file_fatal(HANDLE* file, u32 something);																								// 004D2880
    void fixup_and_init_dialog_data(BinDlg* dialog, void(__fastcall* function)(const char*, Bitmap*, Palette*, const char*, u32), const char* log_filename = nullptr, u32 log_line = 0);	// 004194E0
    void initialize_dialog(u32(__fastcall* function)(BinDlg*, DialogEvent*), BinDlg* dialog);													// 00419D20
    s32 display_dialog(BinDlgProc dialog_proc, BinDlg* dialog);

    s32 get_fastest_distance(u32 angle);																										// 00494C10

    void SC_sprintf_s_2(char* buffer, int buffer_size, const char* format_string, int value1, int value2);										// 0041F1B0

    void insert_order_targeting_unit_before(Unit* target, u8 order_id, Order* order, Unit* unit);												// 004747E0
    void set_flag_attack_move(Unit* unit);																										// 004026A0

    Unit* set_unit_movement_speed_ex(Unit* unit);																								// 00495100

    s32 remove_from_selections_2(Unit* unit, s32 index);																						// 0049A170

    u32* get_cd_key_info(const char** owner, u32* key);																							// 004D9BF0
    u32 local_get_lang();																														// 0041F1D0

    s32 make_valid_position(v2u16* old_pos, v2u16* new_pos, Box16* acceptable_area);														// 0042A5C0

    void cancel_zerg_building(Unit* unit);																										// 0045DA40
    void update_building_construct_shields_amount(Unit* unit);																					// 0047B6A0
    void cmdrecv_place_addon(u32 pos_x, u32 pos_y, u32 order_id, u32 built_unit_id);															// 0048E0A0
    void cmdrecv_place_building_normal(u32 pos_x, u32 pos_y, u32 order_id, u32 built_unit_id);													// 0048E0A0

    u32 distance_from_box_corner(Unit* unit, u16 pos_x, u16 pos_y);																				// 00430E50

    void load_campaign_progress();																												// 004B5050
    BinDlg* load_full_menu_dialog(u32 filename, void* default_value, char* unused, u32 read, char* log_filename, u32 log_line);					// 004DDBE0
    
    u32 glu_load_bin_dialog(bw::Dialog* dialog, bw::DialogInteraction* proc);                                                                    // 0041A080
    inline u32 glu_load_bin_dialog(BinDlg* dialog, void* proc) { return glu_load_bin_dialog(dialog->to_dialog(), (bw::DialogInteraction*)proc ); }
    
    u32 change_menu();																															// 004DCFA0
    void store_selection_group_hp_and_type();																									// 004DCFA0

    void issue_order_before_2(u8 order_id, Order* order_queue_head, Unit* unit, v2u16* pos, Unit* target);									// 004747C0

    u8 apply_defensive_matrix(Unit* unit);				// 00455010
    u8 apply_lockdown(Unit* unit, u32 duration);		// 00455380
    u8 apply_irradiate(Unit* unit);						// 00454E80
    u8 apply_stasis_field(Unit* unit, u32 duration);	// 004F67B0
    u8 apply_plague(Unit* unit);						// 004F4550

    Unit* orders_die(Unit* unit);						// 00479480
    void orders_powerup_1(Unit* unit);					// 004F3EA0
    void orders_infest_mine_4(Unit* unit);				// 004EA290
    void orders_enter_exit_gas(Unit* unit);				// 00469980
    Sprite* orders_powerup_2(Unit* unit);				// 004F3E10
    void orders_reset_collision_1(Unit* unit);			// 004671B0
    void orders_reset_collision_2(Unit* unit);			// 0042E320
    void orders_pickup_4(Unit* unit);					// 004E7B70
    void orders_rescue_passive(Unit* unit);				// 004A1EF0
    void set_kills_string_text(BinDlg* dialog);         // 00425DD0

    void set_unit_hp(Unit* unit, u32 amount);					// 00467340
    s32 unit_movement_another_path(Unit* unit, v2u16 dest);	// 0042FE00

    void scroll_screen_left(u32 x_diff);						// 0049C1A0 
    void scroll_screen_right(u32 x_diff);						// 0049C0C0 
    void scroll_screen_down(u32 y_diff);						// 0049C280 
    void scroll_screen_up(u32 y_diff);							// 0049C360 

    void update_carryable_sprite_flags(Sprite* sprite);			// 004981B0

    void print_lobby_string(u32 player_id, int string);							// 004B91C0
    b32 is_multiplayer_mode_bnet();												// 004C4200
    u32 battlenet_play_sound(char unk1, u32 event_type, char unk2);				// 004AD0B0

    Unit* remove_repulse_tile(Unit* unit);										// 004533D0
    u8 transform_unit_sprite(Unit* unit, u32 new_unit_id);						// 0049F860
    void remove_from_client_selection_3(Unit* unit);							// 0049F7A0
    void update_unit_order_finder(Unit* unit);									// 0046A3A0
    void set_repulse_angle(Unit* unit);											// 00453300
    void transform_subunit(Unit* unit, u32 new_unit_id);						// 0049F6A0
    u8 apply_unit_effects(Unit* unit);											// 00492DA0

    void refresh_layer_5();														// 004BD350

    void manage_vision_references(s32 vision_filter, Unit* unit);				// 0049E8A0
    u32 get_updated_sight_range(Unit* unit, bool is_for_spellcasting);			// 004E5B40
    u8 create_defensive_matrix(Unit* unit);										// 00454F90

    Image* create_image_overlay(Sprite* sprite, u32 image_id, s8 x_offset, s8 y_offset, u32 direction); // 00498E00

    bool statport_button_press(BinDlg* dialog_1, BinDlg* dialog_2);				// 0045F0B0
    BYTE* statport_alloc();														// 0045E360
    
    bool generic_dialog_interact(bw::Dialog* dialog, DialogEvent* event);		// 00418EB0
    inline bool generic_dialog_interact(BinDlg* dialog, DialogEvent* event) { return generic_dialog_interact(dialog->to_dialog(), event); }

    bool generic_common_interact(BinDlg* dialog, DialogEvent* dlg_event);		// 00418C40

    void glu_main_create(bw::Dialog* dialog);                                   // 004DB280

    void dialog_swish_in(bw::Dialog* dialog);                                   // 004DE150
    inline void dialog_swish_in(BinDlg* dialog) { dialog_swish_in(dialog->to_dialog()); }

    void dialog_music_fade(u32 music_id);										// 004A6310
    void glu_main_destroy(BinDlg* dialog);										// 004DB140
    char load_glu_pok_bindlg(const char* string);								// 004B7180
    s32 select_game_mode(u32 game_mode);										// 004DB6E0

    char dialog_swish_out(bw::Dialog* dialog);                                  // 004DD040
    inline char dialog_swish_out(BinDlg* dialog) { return dialog_swish_out(dialog->to_dialog()); }

    s32 load_staredit_process(bw::Dialog* dialog);                              // 004DB490
    s32 glu_main_display_cdrom_error_bindlg();									// 004DB770

    s32 register_menu_functions(BinDlgProc dialog_proc_array, bw::Dialog* dialog, u32 count, u32 palette); // 004DD9E0
    inline s32 register_menu_functions(BinDlgProc dialog_proc_array, BinDlg* dialog, u32 count, u32 palette) {
        return register_menu_functions(dialog_proc_array, dialog->to_dialog(), count, palette);
    }

    void game_menu_bindlg(BinDlg* dialog);										// 004CA370
    void game_menu_bindlg_bypass(BinDlg* dialog);								// 004CA450

    u32 select_unit_group(u8 control_group);									// 00496B40
    u8 keypress_escape();														// 00489DA0

    b32 unit_is_valid_caster(Unit* unit);										// 00401210

    u8 trigger_portrait_finished(BinDlg* dialog, u16 timer);							// 0045E610
    void display_update_portrait(u32 portrait_id, Unit* unit, u16 idle_or_talking);		// 0045EA30
    void trigger_portrait_something(BinDlg* dialog, u16 type_arg);						// 0045EC40
    void animate_unit_portrait_for_event(BinDlg* dialog, u16 type_arg);					// 0045EF50
    b32 set_last_event_position(u32 x, u32 y);											// 0045E3A0
    u32 svid_play_end(u8 video);														// 004100EE
    void start_unit_portrait(BinDlg* dialog);											// 0044EDB0
    u16 get_unit_portrait_override(u16 unit_id);										// 0045E320
    void play_music(u32 music_id);														// 004A6140

    u32 pixel_pos_has_cagra(s16 pos_x, s16 pos_y);										// 004E8BC0
    b32 chk_larva_stray(Unit* larva, u32 pos_x, u32 pos_y);								// 004E93E0
    void chk_larva_stray0(Unit* larva, u32* pos_x, u32* pos_y);							// 004E94B0

    void issue_queued_order_target(Unit* target, Unit* attacker, u8 order_id, u32 stop_previous);	// 00474D10

    void play_addon_detach_sound(Unit* unit);									// 0048F8A0
    Unit* find_best_return_cargo_target(Unit* worker);							// 00441310
    void return_resources_from_unit(Unit* unit);								// 004F39A0

    void unit_destructor_special_cases(Unit* unit);								// 0049F4A0
    void remove_references(Unit* unit);											// 0049EB70
    Bullet* clear_bullet_targets(Unit* unit);									// 0048AAC0
    void building_death(Unit* unit, Sprite* sprite);							// 0049FD00
    Unit* remove_unit_from_lists(Unit* unit);									// 0049E3F0
    Unit* hidden_units_remove(Unit* unit);							// 0049DE50
    Unit* visible_units_remove(Unit* unit);								// 0049DEA0
    void unit_movement_instant_stop(Unit* unit);								// 0046BF00
    void progress_flingy(Unit* unit);											// 00495EE0
    void finish_unit_movement(Unit* unit);										// 0046AD90
    u32 get_random_yes_unit_response_local(u32 unit_id);						// 0048EAB0
    s32 bw_refresh(s32 left, s32 height, s32 top, s32 width);					// 0041E0D0
    Image* make_cloaked(u8 param, Sprite* sprite, u8 status, u8 timer);			// 00497FD0
    void unit_refund_partial_technology(Unit* unit);							// 00453DD0
    void unit_refund_partial_upgrade(Unit* unit);								// 00454220

    void refresh_all_visible_sprites(Sprite* sprite);							// 004976C0
    void update_image_draw_data(Image* image);									// 004D57B0
    void orders_nuke_launch(Unit* unit);										// 00464730
    bool unit_can_have_rally(Unit* unit);										// 0047B2E0

    void blit_surface(Bounds* drawbounds, Bitmap* surface, s16 x, s16 y);		// 0041D260
    void fatal_error(char* error_type);											// 004215D0
    HIMC ImmGetContextBW(HWND hwnd);											// 0041003A
    HIMC ImmAssociateContextBW(HWND hwnd, HIMC himc);							// 00410052
    void dark_minimap_related();												// 004A3A00

    Thingy* iscript_create_sprite(Image* image, u16 sprite_id, u32 x, u32 y, s8 elevation);	// 004D7120

    char compile_list(s32(__stdcall* callback)(u32, u32, u32, u32));			// 004AAD00
    s32 get_last_game_type_from_registry(BinDlg* dialog);						// 004AD5B0
    void unk_list_related(BinDlg* dialog);										// 004AEED0
    void dialog_set_selected_immediate(u8 id, BinDlg* dialog);					// 0041A850

    void chk_unit_apply_burrow_flag(Unit* unit, u8 flag, b32 try_burrow);		// 004CCF20

    void refresh_unit_after_move(Unit* unit);									// 00494160

    void orders_attack_move(Unit* unit);										// 00479040
    b32 ai_sieged_phalanx_can_attack_nearby_unit(Unit* unit);					// 00442460

    void transfer_resource_to_worker(Unit* node, Unit* worker);					// 004696D0

    void ensnare_hit(Unit* unit, u32 x, u16 y);									// 004F4830

    s32 is_selection_group_updated();											// 00424660
    void status_update_multiselection(BinDlg* dialog);							// 00425960
    void status_update_tooltip();												// 00457FE0
    void status_update_single_wireframe();										// 004568B0

    u32 calculate_unit_strength(UnitId::Enum unit_id, WeaponId::Enum weapon_id);			// 00431270
    u32 filter_unit_strength(UnitId::Enum unit_id, u32 strength);				// 00431150
    bool should_attack_unit(u32 attacking_player, Unit* target);				// 00431550
    u32 get_targeting_priority(Unit* unit, b32 is_ground_target);				// 00431730
    bool unit_is_clean_detector(Unit* unit);									// 00403430
    char* get_unit_rank_string(Unit* unit);										// 0042F370
    u32 get_upgrade_time_cost(u32 player, u8 upgrade_id);						// 00453F70
    void create_context_help_from_dialog(BinDlg* dialog, const char* text);		// 00481510
    u16 create_context_tooltip(s32 height, s32 width, char* text);				// 00481510
    void parse_unit_dat_requirements();											// 0046E100

    s32 confirm_building_placeability(Unit* builder, u16 unit_id, u8 player, u16 tile_x, u32 tile_y); // 00473EF0
    
    const char* set_worker_error(Unit* unit, s32 error_string);					// 0048F7B0
    void play_building_placement_sound(Unit* unit);								// 0048F630
    void play_building_placement_sound_0(u16 unit_id);                          // 0048EFF0

    bool tiles_in_rect_are_not_explored_for(u16 width, u16 height, u16 x_pos, u16 y_pos, u8 player); // 004027F0
    bool tiles_in_rect_are_not_visible_for(u16 width, u16 height, u16 x_pos, u16 y_pos, u8 player);  // 00402770
    Thingy* get_placement_parent(u32 sprite_id, u32 x_dims, u32 y_dims, v2u16 parent_dims);        // 00487C70

    s32 vespene_geyser_placebox(u32 tile_x, u32 tile_y, u8 player, u32 check_vision);                // 00473DB0
    u32 update_nydus_placement_state(u8 player, u32 tile_x, u32 tile_y, v2u16 position, u32 placement_state_entry, b32 check_vision); // 00473150
    s32 get_psi_placement_state(u8 player, u16 unit_id, u16 tile_x, u16 tile_y);                     // 00473920
    u32 update_creep_building_placement_state(u8 player, u32 tile_x, u32 tile_y, v2u16 position, u32 placement_state_entry, b32 check_vision, b32 unk_bool); // 00473010
    u32 update_building_placement_state_map_tile_flags(u16 unit_id, u32 tile_x, u32 tile_y, u8 player, v2u16 pos, u32 placement_state_entry, b32 check_vision); // 00473A10
    u32 update_building_placement_state_units(Unit* builder, u32 tile_x, u32 tile_y, u8 player, u16 unit_id, v2u16 pos, u32 placement_state_entry, bool is_not_addon, bool also_invisible, b32 without_vision); // 00473720

    void orders_init_psi_provider(Unit* unit);                                                      // 00493F70
    s8 remove_selection_circle_and_hp_bar(Sprite* sprite);                                          // 00497620
    void hide_all_psi_fields();                                                                     // 00494100
    void draw_cancel_unit_context(BinDlg* dialog);                                                  // 00457B10
    void statdata_user_interact(u32 dialog_index, BinDlg* dialog);                                  // 004576A0
    void draw_cancel_upgrade_context(BinDlg* dialog);                                               // 004575A0
    void create_context_from_dialog(BinDlg* dialog, const char* text);                              // 00457350
    void draw_unload_unit_context(BinDlg* dialog);                                                  // 00457C20
    void do_melee_town_region_fixups(s32 pos_x, s32 pos_y, u32 player, u32 some_type);              // 00445AA0
    void ai_make_chokepoint_region_state_4(u32 player, s32 pos_x, s32 pos_y);                       // 0043A8F0
    void ai_delete_guard_needs(u32 player);                                                         // 004626E0
    void return_to_idle_at_pos(Unit* unit, u16 pos_x, u16 pos_y);                                   // 00475350

    void game_recv(u8* data_buffer, u32 data_length, b32 is_replay);                                // 004865D0
    void report_game_result();                                                                      // 004C4790
    void iscript_set_pos(Image* image, s8 pos_x, s8 pos_y);                                         // 004D5900
    void orders_critter_wander(Unit* unit);                                                         // 0047C3C0

    // For lobby creation and joining
    bool initialize_spi(u32 provider_id);                                           // 004d3cc0
    bool join_game(const GameData* game_data);                                      // 004d3b50
    void load_map_info(MapFile* map_info);                                          // 004a7740
    void load_replay_info(MapFile* map_info);                                       // 004a7c30
    bool load_map_forces(const char* map_path, u32* force_flags, b32 is_campaign);  // 004bf5d0
    u32 load_map_to_globals(GameData* game_data, u32 game_speed);                   // 004a68d0
    bool create_multiplayer_lobby(GameData* game_data, char* game_password);        // 004d3910
    bool prep_singleplayer_game();                                                  // 004dbea0
    bool start_singleplayer_game(GameData* game_data);                              // 004d3fc0

    // For managing character files
    // List characters in Starcraft/characters and call the callback on each
    void iterate_characters(void(__stdcall* callback)(CharacterFile* character));   // 004a8ab0
    u32 save_character_file(CharacterFile* character);                              // 004a8880
    void character_file_path(const char* name, char* output, u32 buffer_size);      // 004a8790

    // For adding additional latency options
    void latency_dialog_destroy(BinDlg* dialog, DialogEvent* dialog_event);         // 004e8920
    void latency_dialog_create(BinDlg* dialog);                                     // 004e89c0
    void latency_print_result(u32, const char* string);                             // 0048d0c0
    void latency_activate_radio(BinDlg* dialog);                                    // 004e1900

    // Shared behaviors between button, defaultbutton, checkbox, and optionbutton
    bool button_shared_interact(BinDlg* dialog, DialogEvent* event);                // 004e1bc0
    
    u8 combobox_add_item(BinDlg* dialog, BinDlgComboboxFlags flags, const char* string);    // 0041b760
    void combobox_clear_items(BinDlg* dialog);                                              // 0041b890
    void generic_combobox_update(BinDlg* dialog);                                           // 0041b410
    void scroll_selection_into_view(BinDlg* dialog, u8 index);                       // 0041a600

    void update_dialog_text(BinDlg* dialog, u16 index, const char* string);         // 004dcf30
    void glu_update_palette(u32 fade_time);                                         // 0041ea30
    void gamma_adjust_palette(u32 palette_count, Palette* output_palette, Palette* input_palette);   // 0041dc20
    // Recomputes lookup table for translucency effect
    void setup_dialog_palette_effects(BinDlg* dialog, Palette* palette);            // 004178b0
    void menu_pop_z_index(BinDlg* dialog);                                          // 004dcc00
    void glu_change_menu_palette(u32 fade_time);                                    // 0041e9e0

    bool initialize_bnet(u32 provider_id);                                          // 00452a30

    graphics::Font* load_font(const char* font_name);                               // 0041fd10

    // For glu_chat
    void process_player_presence(const Command_PlayerPresence* command, u32 sender);            // 004713e0
    void process_join_force_request(const Command_JoinForce* command, u32 sender);              // 00471750
    void process_slot_state_command(const Command_SetSlotState* command, u32 sender);           // 00471570
    void process_update_force_command(const Command_UpdatePlayerForce* command, u32 sender);    // 00471860
    void process_start_game_sync(const Command_StartGameSync* command, u32 sender);             // 00472110
    void update_nation_array(u32 player_id);                                                    // 004a8d40
    void reject_and_disconnect(u32 player_id, u8 reason);                                       // 00470480
    void lobby_recv_original(u32 size, u32 sender, void* command_buffer);                       // 00486040

    void glu_chat_update_control_visibility(BinDlg* dialog, b32 hide_controls, b32 show_ok_button); // 004b89f0

    u32 glu_chat_dlgproc_activate(BinDlg* dialog);          // 004b9910
    void glu_chat_prep_labels(BinDlg* dialog);              // 004b9770
    void glu_chat_dlgproc_destroy(BinDlg* dialog);          // 004b8d70
    void glu_chat_map_preview_destroy(BinDlg* dialog);      // 004b8d90
    
    b32 is_blocked_position(Unit* unit, v2u16 position);                                          // 0042E1D0
    void decollide_or_unstack(Unit* unit, v2u16* position);                                       // 004F29E0
    void set_next_waypoint(Unit* unit, u32 pos_x, u32 pos_y);                                       // 004EB130
    void get_allocated_path_single_step(Unit* unit, u32 next_pos, u32 end_pos);                     // 0042F830

    void issue_order_by_type(u8 order_id, u16 pos_x, u16 pos_y, u32 order_type, Unit* unit, Unit* target,
                             u32 fow_unit_id, v2u16 position, Unit* secondary_target);             // 00475470
    void command_targeted(u16 pos_x, u16 pos_y, Unit* unit, u32 unit_id, u8 order_id, u8 is_queued); // 0049AB00
    s32 node_is_unpathable(Unit* node);                                                              // 00473560
    Unit* get_nearest_unit_from(u32 pos_x, u32 pos_y, u16 range, s32(__fastcall* proc)(Unit*, Unit*), Unit* compare); // 004E86C0

    bool can_enter_nydus(Unit* unit, Unit* nydus);                                                    // 004E8C20
    bool can_recharge_shields(Unit* unit, Unit* battery);                                             // 00493520

    s8 reset_game_input_procs(u16 cursor_index);                                                      // 004843F0
    void update_mineral_field_image(Unit* mineral_field);                                             // 00468830

    b32 game_is_team_game();                                                                        // 00484F50
    s32 is_cpu_force(u32 force_id_1, s8 force_id_2);                                                // 00484DF0

    void map_ui_colors_to_palette(Palette* palette, void* map_function);                            // 0041e450

    void set_fog_mask(u32 tile_flags, u32 x_tile, u32 y_tile, u32 x_final, u32 y_final);            // 00472E50

    u32 get_system_time(LPSYSTEMTIME system_time);                 // 0040C7DF

    bool is_distance_greater_than_path_distance_to_order_target(Unit* unit, s32 distance);  // 00465480

    void resize_replay_action_set(ReplayData* replay_data);         // 004CDD30
    void new_replay_action(ReplayData* replay_data);                // 004CDDC0

    bool unit_has_power(u16 unit_id, u32 pos_x, u32 pos_y, u8 player); // 004936B0

    s32 convert_current_order_to_carrier_reaver_idle_order(Unit* unit);                     // 00465390
    s32 convert_current_order_to_carrier_reaver_fight_order(Unit* unit);                    // 00465360
    
    u32 get_resarea_id(Unit* unit);                                 // 00445930

    bool modal_ok_cancel(const char* prompt); // 004b73b0
    s32 save_replay(char* filename, b32 delete_existing = true);   // 004DFAB0
}