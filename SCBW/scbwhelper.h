#pragma once
#include "SCBW\scbwdata.h"

#define NOMINMAX
#include <windows.h>

#define SCBW_FUNC(address, return_and_convention, name, ...) \
typedef return_and_convention name##_func(__VA_ARGS__); \
name##_func* const name = (name##_func*) address;

namespace helper
{
	bool has_money_can_make(Unit* builder, u32 unitToBuild);

	// For loadRaceUI
	void get_race_sound_list(s8 a1);
	s32 dialog_grp_construct(s32 a1, s32 a2, s32 a3, s32(__fastcall* a4)(s32, s32, s32, s32));
	void load_graphic(s8 a1, s32 a2, s32 a3, s8 a4, s32 a5);

	s32 rand_func();
	bool unit_hp_below_33_percent(Unit* a1);
	bool unit_hp_between_33_66_percent(s32 a1, Unit* a2);
	s32 get_max_hp(Unit* a1);
	u16 get_last_queue_slot_id(Unit* unit);
	void add_text_to_dialog(BinDlg* dialog, s16 index, const char* text);
	void draw_energy(BinDlg* a1);
	void hide_optional_status_screen_controls(BinDlg* a1);
	void show_control_group(BinDlg* dialog, s16 firstId, s16 lastId);
	void hide_dialog(BinDlg* dialog);

	// For draw_supply_unit_info
	s32 get_supplies_used_no_args();
	u32 get_supplies_available(u8 a1, s32 a2);
	void unit_stat_act_standard(void* a1);

	// For AI_TargetEnemyProc
	bool ai_unit_can_attack(Unit* a1);

	// For draw_building_info
	s32 ui_needs_redraw();
	Unit* generic_status_update_drawn_values();
	void draw_progress_bar(BinDlg* a1);
	void draw_nuke_silo_info(BinDlg* a1);
	bool unit_is_active_transport(Unit* a1);
	bool is_queue_slot_active(Unit* unit, s32 slot);
	void draw_training_progress(BinDlg* dialog);					// 004268D0
	bool is_constructing_addon(const Unit* a1);
	bool is_attempting_protoss_build(Unit* a1);
	void draw_addon_progress(BinDlg* a1);
	void draw_tech_progress(BinDlg* a1);
	void draw_upgrade_progress(BinDlg* a1);
	bool unit_is_geyser(Unit* a1);
	void draw_gas_quantity(BinDlg* a1);
	s8 get_unit_ground_weapon(Unit* a1);
	void draw_display_buttons(BinDlg* a1);
	BinDlg* get_control_from_index(BinDlg* a1, s16 a2);
	void set_text_string(BinDlg* a1);
	void set_unit_status_string_text(BinDlg* a1);
	s32 get_unit_name(Unit* a1);
	void draw_supply_unit_info(BinDlg* dialog);						// 00427540

	// For get_unit_name
	const char* get_unit_name_stat_string(u16 unit_id); // 0047B090

	// For get_unit_strength
	s32 get_current_hp_plus_shields(Unit* a1);
	s32 get_base_strength(s32 a1, Unit* a2);
	s32 get_max_hp(Unit* a1);

	// For AI_RestorationRequirementsProc
	s32 can_weapon_target_unit(u8 a1, Unit* a2, Unit* a3);

	// For AI_EMPShieldRequirementsProc
	s32 ai_target_unit_is_worth_hitting(Unit* a1, Unit* a2);
	s32 create_splash_weapon_proc(s16 a1, s16 a2, s16 a3, s32(__fastcall* a4)(s32, s32), s32 a5);
	s32 unit_shields_clump_in_area_accumulator_proc(Unit* a1, Unit* a2);

	// For hallucinationHit
	void unit_was_hit(Unit* a1, Unit* a2, s32 a3);
	void unit_attack_notify(Unit* unit);
	Image* create_shield_overlay(s32 a1, Unit* a2);

	// For drawImageHPBar

	s32 unk_billions(s32 a1, s32 a2, s32 a3, s32 a4);
	s32 unk_billions2(s32 a1, s32 a2, s32 a3, s32 a4);
	s32 hpBarSurfaceBuffer(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5);
	s32 hpBarSurfaceBuffer2(s32 a1, s32 a2, u16 a3, s32 a4);
	s32 renderHPBar(s32 a1, s32 a2, s32* a3, s32 a4, s32 a5);
	s32 get_unit_max_energy(Unit* a1);
	s32 getHpBarDrawSizes(s32 a1, s32 a2, s32 a3, s32 a4);

	// For canCastSpell
	bool unitIsFrozen(Unit* a1);
	bool isUnitCasterNotHallucination(Unit* a1);
	b32 getActiveTileFlag(s16 y, s16 x, s32 includeFlags, s32 excludeFlags);
	Unit* getTargetUnitAbilityErrorStatString(Unit* a1, char a2, s32 a3);

	// For modifyUnitShields
	s32 SErrSetLastError(char a1);

	// For compileHealthBar
	void updateImageDrawData(Image* result);
	void playImageIscript(Image* img, u8 iscriptAnim);

	// For AI_FindTransport
	bool canEnterTransport(Unit* a1, Unit* a2);
	s32 getUnitDistanceToHalt(s32 a1, Unit* a2, s32 a3);

	// For mapDataTransfer
	s32 dataTransfer00(s32 a1, char a2, s32 a3);
	s32 dataTransfer01(u32 a1, s32 a2, u32 a3);
	s32 dataTransfer02(s32 a1, s32 a2, char a3);
	s32 dataTransfer03(s32 a1, char a2, s32 a3);
	s32 dataTransfer04(u32 a1, s32 a2, s32 a3);
	s32 dataTransfer05(s32 a1, s32 a2);
	VectorHead* vectorTransfer(char a1, char a2);
	s32 dataTransfer06(s32 a1);

	// For getRandYesSFX
	s32 randGetSFXBetween(s32 soundIdMin, s32 soundIdMax);										// 0048E940
	s16 get_region_id(s32 x, s32 y);															// 0049C9F0

	// For ai_train_unit
	char ai_training_worker(Unit* result, Unit* parent);											// 00435700
	void ai_training_overlord(Unit* result, Unit* parent);										// 00435770
	void ai_training_normal(Unit* result, Unit* parent);											// 00435DB0
	void ai_order_to_destination(Unit* unit, s32 orderId, s32 x, s32 y);							// 0043D5D0
	void ai_assign_region_to_slowest_unit(AiRegion* region);										// 00436F70

	// For ai_create_nuke
	Unit* create_unit(s16 unitType, s32 x, s32 y, s32 playerId);
	void update_unit_stats_and_finish_building(Unit* unit);
	u32 update_unit_strength_and_apply_default_orders(Unit* unit);
	void hide_unit(Unit* unit);

	// For AI_TrainingBroodling
	void ai_set_region_type(s32 regionType, AiRegion* region);
	void add_military_ai(AiRegion* region, Unit* unit, s32 always_this_region);

	// For ai_training_worker
	char add_worker_structure_ai(Unit* worker, AiTown* town);									// 00433DD0

	// For AI_AddUnitAI
	BuildingAi* create_building_ai(BuildingAiArray** buildingAI);
	void issue_order_targeting_nothing(Unit* unit, u8 orderId);
	Unit* set_secondary_order(Unit* result, char orderId);
	void ai_make_guard(s32 playerId, Unit* unit);
	bool unit_is_active_resource_depot(Unit* unit);
	u32 get_default_refinery_type(u32 playerId);

	// for ai_unit_was_hit // some of these should probably be methods in api/etc
	char ai_cancel_structure(Unit* structure);
	Unit* ai_get_actual_target(Unit* attacker, Unit* target);
	Unit* check_arbiter_cloaking_range(Unit* unit, u16 id);
	void ai_order_unit_cloaking(Unit* unit);
	void ai_order_lurker_burrow(Unit* unit);
	bool ai_is_under_attack(Unit* unit, s32 flag);
	bool unit_is_active_transport(Unit* unit);
	s32 is_unit_loaded(Unit* unit);
	s32 ai_try_return_home(Unit* unit, s32 dontIssueOrder);
	s32 ai_transport_action(Unit* unit);
	s32 ai_ask_for_help(Unit* self, Unit* attacker, s32 onlyNearbyGuards);
	s16 get_region_id_from_unit(Unit* unit);
	s32 get_enemy_air_strength_in_region(u16 regionId, s32 player);
	s32 get_enemy_strength_in_region(u16 regionId, s32 player, s32 includeAir);
	bool unit_is_performing_unbreakable_code(Unit* unit);
	bool ai_cast_spell_behavior(Unit* unit, bool isAggro);
	s32 ai_flee(Unit* unit, Unit* enemy);
	bool unit_has_ground_path(Unit* unit, s16 x, s16 y);
	bool ai_has_no_defense_build(s32 player);
	bool unit_can_see_cloaked_target(Unit* target, Unit* unit);
	void ai_respond_to_cloaked_threat(Unit* threat, Unit* unit);
	bool unit_can_attack_target(Unit* target, Unit* unit, s32 checkDetection);
	s32 ai_update_attack_target(Unit* unit, s32 acceptIfSieged, s32 acceptCritters, s32 mustReach);
	__int64 AI_PrepareToFlee(Unit* enemy, Unit* unit);

	// for iterate_building_ai
	u16 ai_remove_from_towns(Unit* worker);
	s32 ai_get_queued_count(Unit* builder, u32 unit_id);
	void prepare_guard_ai(u16 y, u32 player, u16 unit_id, u16 x);

	// For ai_send_suicide
	bool is_threat(Unit* target, Unit* unit);														// 00476930
	void prepare_suicide(Unit* unit);																	// 0043CD40
	u32 ai_attack_clear(s32 player);																	// 00439500
	bool ai_attack_manager(s32 player, s32 x, s32 y, s32 bOverrideLastAttack, s32 bFallbackAirOnly); // 0043ABB0

	// Attack extender

	// for AI_TrainAttackForce
	s32 ai_add_to_region_military(AiRegion* region, s32 unit_id, s32 priority);
	u32 ai_add_cost_to_needs(u32 unit_id, u32 playerId, u32 type);
	u32 ai_add_spending_request(u16 unit_id, u32 player, u8 type, char priority, void* value);

	// for AIRegion_GroupForAttack
	u32 ai_recalculate_region_strength(AiRegion* region);
	Unit* ai_target_enemy_in_region(AiRegion* region);
	s32 ai_move_military_to_defend(AiRegion* region);
	void ai_get_region_military_types(AiRegion* region, s32* hasGroundMilitary,
		s32* cantAttackGround, s32* cantAttackAir);
	s32 ai_can_travel_to_region(AiRegion* region1, AiRegion* region2);
	Unit* ai_pick_attack_target_no_buildings(AiRegion* region);
	u32 ai_move_military_to_other_region(AiRegion* target, AiRegion* source, u32 pullUnitsNearRegion,
		u32 sendPureSpellcasters, u32 dontSendFromBunkers);
	AiTown* ai_iterate_town_something(AiRegion* region);
	u32 ai_attack_timer_decrement(u32 player);
	s32 ai_has_forces_moving_to_attack(u32 player);

	// for ai_attack_manager
	u16 populate_regions_with_own(char* regionData, u32 player);
	void populate_regions_with_enemy(u32 player, char* regionData);
	s32 ai_get_neighboring_region_types(char* regionData);
	void ai_finish_region_accessibility(char* regionData);
	s32 ai_set_attack_grouping_region(u32 airAttack, u32 alsoVeryNear, u32 player, char* regionData,
		s32 x, s32 y, u32 alwaysOverwriteLastAttack);
	char ai_cancel_structure(Unit* structure);

	bool unit_cant_see_cloaked_target(Unit* unit, Unit* target);												// 00401D60
	bool unit_can_infest(Unit* unit);																			// 00402750
	void ai_attack_unit(Unit* unit);																			// 0043FFD0
	Unit* find_best_attack_target(Unit* unit);																	// 00443080
	bool ai_try_return_home(Unit* unit, u8 unknownByteValue);													// 00462EA0
	void set_interceptors_to_standby(Unit* unit);																		// 00465780
	Order* remove_order_from_unit_queue(Order* order, Unit* unit);											// 004742D0
	void prepend_order_targeting_unit(Unit* unit, Unit* target, u8 orderId);								// 00474A70
	bool is_target_within_min_movement_range(Unit* unit, Unit* target, u32 range);							// 004763D0
	bool ai_is_unreachable_position(Unit* unit, s32 x, s32 y);														// 00476610
	bool is_unit_in_weapon_range(Unit* unit, Unit* target);														// 00476870
	void get_weapon_begin_iscriptt(Unit* unit, IscriptAnimation::Enum anim);									// 00476ED0
	void pick_new_target_if_needed(Unit* unit);															// 00477510
	void attack_move_try_pick_target(Unit* unit, u8 orderId);															// 00477820
	bool attack_move_react_to_attack(Unit* unit, u8 orderId);															// 00478370
	bool attack_unit_main(Unit* unit);																	// 00478540
	bool is_unit_critter(Unit* unit);																			// 0047B450
	void move_to_collide(Unit* unit, Unit* target);														    // 00495400

	bool is_unit_visible(Unit* unit);																			// 004E5DB0
	bool is_target_visible(Unit* unit, Unit* target);															// 004E5E30
	u8 get_right_click_action_order(Unit* unit);																// 004E5EA0
	void set_resource_target(Unit* unit);															        // 004EB290
	void make_to_hold_position(Unit* unit);																	// 004EB5B0
	bool move_to_target(Unit* target, Unit* unit);															// 004EB720
	bool set_move_target_xy(Unit* unit, s32 x, s32 y);														// 004EB820
	s32 moveTowards(Unit* unit, Unit* target);																// 004EB900
	bool change_movement_target_move_target_2(Unit* unit, s32 x, s32 y);											// 004EB9C0

	bool is_distance_greater_than_halt_distance(Unit* unit, s32 x, s32 y, u32 distance);							// 00401240
	void check_unstack(Unit* unit);												                            // 0042D9A0
	bool begin_building(Unit* unit);																			// 00467030
	Unit* begin_gas_building(Unit* unit, u32 unit_id);															// 004678A0
	void add_overlay_below_main(Sprite* sprite, u32 imageId, u32 x_offset, u32 y_offset, u32 direction);		// 00498D70
	void replace_sprite_images(Sprite* sprite, u32 imageId, u32 imageDirection);								// 00499BB0
	void display_last_network_error_for_player(u32 playerId);										                // 0049E530
	void update_unit_strength(Unit* unit);												                    // 0049FA40
	void iscript_opcode_cases(Image* image, u32 offset_, u32 unk1, u32 unk2);								// 004D74C0
	void switch_to_warp_texture(Image* image);																	// 004D8500
	bool set_thingy_visibility_flags(Thingy* thingy);									                        // 004878F0
	Thingy* create_thingy(u32 spriteId, s16 x, s16 y, u32 playerId);		    								// 00488210
	void setUnitStatTxtErrorMsg(char* message);										                        // 0048CCB0
	void playSoundAtPos(u32 soundId, u16 x, u16 y, u32 unk1, u32 unk2);										// 0048EC10
	u32 randomize_short(u32 unknown_index);																	// 004DC4A0
	u32 IterateUnitsAtLocationTargetProc_RecallTargets(Unit* unit, Box16* coords);							// 004E8280
	void move_unit_duplicate(Unit* unit, s32 x, s32 y);														// 004EBAE0

	//// spider_mine
	Unit* SpiderMine_AcquireTarget(Unit* spiderMine);									                    // 00441270
	void instantCloak(Unit* unit);												                            // 0049B1E0
	void unburrowGeneric(Unit* unit);												                        // 004E99D0
	void burrowGeneric(Unit* unit);																			// 004E9A30

	// For orders_PlaceMine
	s32 techUseAllowed(Unit* a1, __int16 a2, s32 a3);									                    // 0046DD80
	u32 get_distance_fast(s32 a1, s32 a2, s32 a3, s32 a4);													// 0040C360
	s32 orders_HoldPosSuicidal(Unit* a1);												                    // 004EB5B0
	void turnUnit(Unit* a1, char a2);													                    // 00495F20
	void to_idle(Unit* a1);																                    // 004753A0
	s32 is_unit_movable(Unit* a1);														                    // 00401DC0
	void orders_nothing_2(Unit* a1);																			// 00475000
	bool set_move_target_xy(s32 a1, s32 a2, Unit* a3);															// 004EB820

	// For burrowGeneric
	void toggle_unit_path(Unit* unit);									                                    // 004E42A0
	char switchSpriteId(Sprite* sprite, s32 spriteId);														// 00499860

	//// building_morph
	void refundBuildingCost(u16 unit_id, u8 playerId);														// 002CE70
	void updateBuildingsAI(Unit* unit);																		// 0033FE0
	bool isValidMorph(u16 buildingId);																		// 005CC60
	void SpreadInitialCreep(u32 id, s32 x, s32 y);															// 005CE90
	void zergPlaceBuildingCntd(Unit* unit);																	// 005D410
	void stopCreepDisappearingAtBuilding(Unit* unit);												        // 007D770
	void increment_unit_scores_ex(Unit* building, s32 unk1, s32 unk2);											// 0088D50
	void play_building_completed_sound(Unit* building);															// 008F310
	void refreshSpriteData(Sprite* sprite);																	// 00983A0
	void display_last_network_error_for_player(u32 playerId);									                    // 009E530
	void replace_unit_with_type(Unit* unit, u16 newUnitId);													// 009FED0

	//// building_terran
	bool is_distance_greater_than_halt_distance(Unit* unit, s32 x, s32 y, u32 distance);							// 00401240
	void fix_target_location(Point16* coords, u32 unit_id);								                    // 00401FA0
	bool is_unit_positions_2_equal(Unit* unit);																	// 00402160
	b32 progress_build(Unit* unit, u32 hpGain, b32 showHidden);												// 004679A0
	void prepend_order_targeting_nothing(Unit* unit, Order* order, u8 orderId);								// 00474760
	void add_order_targeting_nothing(Unit* unit, Order* order, u8 orderId);									// 004749A0
	bool can_still_place_building();														                    // 0048DDA0
	void end_building_placement();															                // 0048E310
	void play_building_completed_sound(Unit* building);															// 0048F310
	void set_secondary_waypoint(Unit* unit, s32 x, s32 y);									                // 00494BB0
	void display_last_network_error_for_player(u32 playerId);									                    // 0049E530
	u32 randomize_short(u32 unknown_index);																	// 004DC4A0
	u32 rand_between(u32 min, u32 max, u32 someIndex);														// 004DC550
	void init_building(Unit* unit, b32 flag);																// 004E65E0
	bool set_move_target_xy(Unit* unit, s32 x, s32 y);									                    // 004EB820
	bool order_to_move_to_target(Unit* unit, Unit* target);														// 004EB980
	b32 unk_terrain_collision_2(Unit* unit, s32 x, s32 y);													// 004F1870

	// For updatePlacementState_Units
	bool does_building_block(Unit* unit, s32 xTile, s32 yTile);												// 00473410
	s32 is_tile_blocked_by(Unit** array, Unit* builder, s32 xTile, s32 yTile,									// 00473300
		char bDontIgnoreReacting, char bAlsoInvisible);

	// For update_placement_state_for_placement_box
	s32 can_build_on_tile_location(Unit* builder, s32 playerId, s32 xTile, s32 yTile, s32 unitType,				// 00473FB0
		s32 placementStateEntry, s32 bCheckVision, s32 bAlsoInvisible, s32 bWithoutVision);
	
	// For refresh_screen
	s32 update_placement_state_for_placement_box();															// 0048DCE0
	s32 unk_building_placement_related(s32 a1, Point16* a2);													// 0048D7F0
	s32 init_placement_box_pos(Point16* in_out);																// 0048D660
	char unk_building_placement_related_mouse(s32 a1, s32 a2);													// 0048D7B0

	//// liftland
	void setUnitPathing(Unit* unit, u8 unkPathRelated);																				// 00401360
	bool is_unit_positions_2_equal(Unit* unit);																							// 00402160
	Unit* connectAddonCheck(Unit* unit);																							// 004404A0
	void modify_building_grounded_status(Unit* unit);																				// 00463470
	void toggle_pathing_related(Unit* unit);																						// 004634E0
	void flying_building_lift_if_still_blocked(Unit* unit);																			// 00463640
	void completeAddon(Unit* unit, Unit* addon);																					// 00463D50
	void crush_immobile_units(Unit* unit);																							// 00464300
	void disconnectFromAddOn(Unit* unit);																							// 00464930
	void clear_building_from_tile_flag(Unit* unit, s32 x, s32 y);																	// 00469EC0
	void SetBuildingTileFlag(Unit* unit, s32 x, s32 y);																				// 00469F60
	void end_collision_tracking(Unit* unit);																						// 0046A560
	void start_collision_tracking(Unit* unit);																						// 0046A5A0
	void force_order_done(Unit* unit);																								// 00475420
	bool update_creep_disappearance_unit(Unit* unit);																				// 0047DF90
	void update_creep_disappearance_unit_id(u32 unit_id, u32 x, u32 y, b32 completed);												// 0047DE40
	void placebuildingProc();																										// 0048E6E0
	void playBuildingLandSound(Unit* unit);																							// 0048F5A0
	void SetUnitMovementSpeed(Unit* unit, u32 newSpeed);																			// 004951C0
	void set_shadow_y(Sprite* sprite, u8 newVerticalOffset);																		// 00498150
	void create_landing_dust(Sprite* sprite);																						// 004997E0
	void function_004C3B40(Unit* unit);																								// 004C3B40

	//// create_init_units
	b32 spreads_creep(u32 unit_id, b32 defaultReturn);																				// 00413870
	void applyCreepAtLocationFromUnitType(s32 x, s32 y, u32 unit_id);																// 004148F0
	void CreateBuildingLarva(Unit* main_building);																					// 0049D660
	b32 show_finished_unit(Unit* unit);																								// 0049EC30
	Unit* FindBestUnit(Box16* coords, u32 searchProc);																				// 004E8830

	//// larva_creep_spawn
	Unit* getUnitPointer();
	u8 spreadCreep(u32 unit_id, s32 x, s32 y);																						// 00414680
	void ai_training_overlord(Unit* main_building, Unit* larva);																		// 00435770
	void update_unit_strength(Unit* unit);																							// 0049FA40
	u32 IterateUnitsAtLocationTargetProc_LarvaCount(Unit* unit, Box16* coords);														// 004E8280
	void function_004E8DA0(Unit* unit);																								// 004E8DA0
	bool getLarvaSpawnPos(Unit* unit, Vector32* coords);																			// 004E8E10

	// TODO: move somewhere else?
	// SC Variables and Buffers used in this file
	u32* const mapTileBufWidth = (u32*)(0x006D0F08);
	u32* const mapTileBufHeight = (u32*)(0x006D0C6C);
	u16** const mapTileBuf = (u16**)(0x006D0E84);
	Vector32* const someCreepPointBuf = (Vector32*)(0x006D7578);
	typedef u32(*isTileVisible_func)(u32 x, u32 y);
	typedef bool(*setCreepRelationalData_callback)(u16* tileBackup, u8* creepOverlayVal, u32* retval, u16* tile, u32 arg);
	isTileVisible_func* const isTileVisible_ptr = (isTileVisible_func*)(0x006D0C78);

	u32 getNumSurroundingCreepTiles(u32 x, u32 y);																					// 00414290
	void setCreepRelationalData(u32 x, u32 y, setCreepRelationalData_callback callback, u32 arg);									// 00413FA0
	u32 iterateDirectionalCreepData(u32 x, u32 y, u32 callback, u32 arg);															// 00413B30

	// TODO: add helpers for these functions
	// for applyCreepAtLocationFromUnitType
	
	typedef s32 __stdcall CreepChange(s32, s32, s32, s32, s32);

	s32 modifyCreepState(s32 x_tile, s32 y_tile, CreepChange* creepChange, u32 param);												// 00413FA0
	s32 runCreepRandomizer(s16* originalTile, s32 creepTileBorders, s32* outCreepStatus, s16* currentTile, s32 unused);				// 004140A0 // "note possible desync"
	s32 unkCreepBorders(s32 a1, s32 a2);																							// 006D0C78
	u32 forEachAdjacentTile(s32(__stdcall* callback)(s32, s32, s32, s32, void*), void* param, s32 x_tile, s32 y_tile);				// 00413B30
	s32 unkCreepTile(s32 pos, s32 originalTile, s32 x_tile, s32 y_tile, void* param);												// 00414440
	char unkSpreadCreep(s32 a1, s32 a2, s16 a3);																					// 00414680

	//// burrow_tech
	void force_order_done(Unit* unit);																								// 00475420
	void targeted_order(Unit* unit, u32 unit_id, u32 unk1, u32 unk2, u32 orderId, u32 unk3, u32 unk4, u32 unk5);					// 004754F0
	void function_004E97C0(Unit* unit);																								// 004E97C0

	//// die_order
	void clearUnitSprites(Sprite* unit_sprite, Sprite* subunit_sprite);																// 00496F00
	void function_00497480(Sprite* thingySprite, u32 visibility_flags);																// 00497480
	void _UnitDestructor(Unit* unit);																								// 004A0740
	void UnitDestructor(Unit* unit);																								// 004A0990
	void hide_and_disable_unit(Unit* unit);																							// 004E6340

	//// CMDRECV_Cancel
	void CancelResearch_Proc(Unit* unit);																							// 00453E30
	void cancel_upgrade_proc(Unit* unit);																							// 00454280
	void refund_queue_slot(Unit* unit, u16 slot);																					// 00466A70
	void cancel_last_training(Unit* unit);																							// 00466E40
	void cancel_addon_proc(Unit* unit);																								// 004E66E0

	//// cmdrecv_build
	b32 order_is_allowed(Unit* unit, u16 order, u32 nationID);																		// 0046DC20
	bool place_building_message(u32 result_function_473FB0);																		// 0048D930
	bool CMDRECV_place_building_allowed(Unit* unit, u8 orderId, u16 builtUnitId);													// 0048DBD0
	void function_0048DE70(Unit* unit, u32 orderId, u16 dimensionX, u16 dimensionY, u32 builtUnitId);								// 0048DE70
	void CMDRECV_place_building(s16 x, s16 y, u8 orderId, u16 builtUnitId);															// 0048E190

	//// CMDRECV_MergeArchon
	Unit* templar_merge_partner(Unit* unit, Unit** units_list, u32 units_list_length);												// 004BFA80

	//// CMDRECV_Stop
	void parse_orders_dat_requirements();																							// 0046D450
	s32 parse_requirement_opcodes(Unit* unit, u32 orderId, u32 playerId, u32 address, u32 req_offset);								// 0046D610

	//// give_unit
	void apply_speed_upgrade_from_unit_type(Unit* unit);																			// 00454370
	void recheck_attacks(Unit* unit);																								// 00476D30
	void increment_unit_scores(Unit* unit, s32 unkScore);																			// 00488BF0
	void create_unit_selection(Unit* unit, u32 selectionIndex);																		// 004E6180
	void destroy_unit_selection(Unit* unit);																						// 004E6290
	void drop_powerup(Unit* powerup);																								// 004F3B70

	//// cast_order
	u32 get_upgraded_weapon_cooldown(Unit* unit, u8 weaponId);																		// 00475DC0
	bool is_ready_to_attack(Unit* unit, u32 weaponId);																				// 00476640
	u32 get_stat_txt_string(Unit* unit, u32 playerId, char* message);																// 0048EF30
	bool can_cast_spell(Unit* unit);																								// 00492140
	u32 ordered_spell_progress_moving(Unit* unit, b32 wasMainOrderStateZero);														// 00492620

	// For spell_order
	bool getOrderTargetErrorString(u16 a1, Unit* a2, s16 a3, Unit* a4, s16 a5, s16 a6, u16* a7);									// 00492020
	s32 spellOrder_MoveTowardsTarget(Unit* caster, u32 range);																		// 00492450

	// For canTargetSpell
	s16 canTargetSpellOnFOWUnit(s32 techId, s16 unit_id);																			// 004914B0

	void increment_unit_death_scores(Unit* unit, u8 player);																		// 00488AF0
	void initialize_empty_unit_linked_list_references(Unit* unit, u8 playerId);														// 0049E4E0
	void give_unit_helper(Unit* unit, u32 playerId, s32 unkScore);																	// 0049EFA0
	void change_unit_buttonset(Unit* unit, u16 unit_id);																				// 004E5D60
	void show_and_enable_unit(Unit* unit);																							// 004E6490

	//// tech_target_check
	bool is_infestable_unit(Unit* unit);																							// 00402210

	//// trap_hooks
	// For update_unit_strength_and_apply_default_orders
	void anim_cloak(Unit* a1, s32 a2);																								// 0049B5B0
	void refresh_unit_vision(Unit* a1);																								// 004E5F30
	void init_unit_trap_doodad(Unit* a1);																							// 004E6490
	Order* perform_another_order(u8 order_id, Order* order, Unit* unit, s16 fow_unit_id, Point16 position_xy, Unit* target);		// 004745F0
	void order(s32 a1, Unit* a2, s8 a3, Vector32* a4, Unit* a5, s32 a6);															// 00474810
	u32 get_unit_strength(Unit* a1, s32 a2, bool a3);																				// 00431800

	//// make_nydus_exit
	void play_worker_error_sound(Unit* unit, u32 errorTextIndex);																	// 0048F6C0

	//// research_upgrade_orders
	bool is_unit_upgrade_available(Unit* unit);																						// 004033D0
	void apply_speed_upgrade_from_upgrade_id(Unit* unit, u8 upgrade_id);															// 00454540
	void play_upgrade_complete_sound();																								// 0048F070
	void play_research_complete_sound();																							// 0048F150
	void set_upgrade_level(u32 playerId, u32 upgrade_id, u8 upgrade_level);															// 004CE770

	//// merge_units
	bool advance_remaining_build_time(Unit* unit);																					// 00466940
	u8 delete_pathing(Unit* unit);																									// 0046A820
	void order_immediate(Unit* unit, u8 order);																						// 00474B40
	void play_morphing_complete_sound(Unit* unit);																					// 0048F440
	void merge_status_into_unit(Unit* unit_dst, Unit* unit_src);																	// 00493180
	void anim_decloak(Unit* unit, u32 soundId);			 																			// 0049B440
	bool delayed_set_move_target_xy(Unit* unit, u16 x, u16 y);																		// 004EB960

	//// nuke_orders
	void set_all_overlay_directions_generic(Unit* unit, u32 unk);																	// 00401140
	u32 ai_return_to_nearest_base_forced(Unit* unit);																				// 0043DB50

	//// target_related

	// for is_unit_target_out_of_max_range
	s32 get_unit_max_weapon_range(Unit* a1, unsigned __int8 a2);

	// for isReadyToAttack
	u32 getDistanceToUnit(Unit* a1, Unit* a2);
	u32 distanceFromDboxCorner(Unit* a1, __int16 a2, __int16 a3);
	bool isAngleInAttackAngle(s32 a1, s32 a2, Unit* a3, unsigned __int8 a4);

	// for canCastOrderedSpellNow
	bool is_target_within_min_range(Unit* unit, u32 minRange, Unit* target);
	s32 getFlingyHaltDistance(s32 a1);

	// for ordersWatchTarget
	void unknownOrdersFunc(s32 a1, __int16 a2, __int16 a3);

	// for ordersStayInRange
	void prependOrderTargetingGround(__int8 order_id, __int16 x, __int16 y, Unit* unit);
	s32 ordersHoldPositionSuicidal(Unit* a1);
	s32 getDirectionFromPoints(s32 a1, s32 a2, s32 a3, s32 a4);
	Vector32 setResourceTarget(Unit* a1);

	// for ai_attack_unit
	void stimpackProc(Unit* unit);																									// 004554A0

	// for genericRightClickUnitProc2
	s32 isOrderValid(char a1, Unit* a2, s32 a3, Unit* a4, s32 a5);																	// 00474D90
	u16 getUnitAbilityErrorStatString(Unit* a1, char a2, s32 a3);																	// 00491E80

	// for CMDRECV_HoldPosition
	Unit* get_active_player_next_selection();																							// 0049A850
	s32 orderAllowed(Unit* a1, u16 a2, s32 a3);																						// 0046DC20
	void targetedOrder(s16 a1, s32 a2, Unit* a3, s16 a4, s32 a5, s32 a6, char a7, s32 a8, s32 a9, s32 a10);							// 004754F0

	// for check_unit_visibility
	u32 get_detecting_players(Unit* unit);																							// 00441130
	u32 iteratePlayerUnitsSomething(u32 aResult, Unit* unit);																		// 0049E8A0
	void makeSpriteVisible(Sprite* sprite);																							// 00497ED0
	s32 updateBuildingLandUnitSelection(u32 unused, Unit* selection);																// 004C3B40
	char removeSelectionCircle(Sprite* sprite);																						// 004975D0
	Image* makeSpriteInvisible(Sprite* sprite);																						// 00498A10

	//// receive_command
	void function_004756B0(Unit* unit, u8 orderId, u32 unk1, u32 unk2, u32 bCommandType);											// 004756B0
	void function_004756E0(Unit* unit, Unit* target, u32 orderId, u32 bCommandType);												// 004756E0
	u32 function_0049A410(Unit* unit, u8 orderId);																					// 0049A410
	bool CanTargetSelf(Unit* unit, u8 orderId);																						// 0049A480
	void function_0049A500(Unit* unit, u8* array_of_data);																			// 0049A500
	void function_0049A8C0(u8* array_of_data, b32 bCanBeObstructed);																// 0049A8C0

	//// load_unload_proc
	void IgnoreAllScriptAndGotoIdle(Unit* unit);																					// 00476550
	u32 canUnload(Unit* transport, Unit* loaded_unit, Point16* pos);																// 004E76C0
	void function_004E7E10(Unit* unit, u32 unk);																					// 004E7E10

	//// stats_panel_display
	void StatsNukesCount(BinDlg* dialog, u32 index);																				// 00425310
	void StatHangerCount(BinDlg* dialog, u32 index);																				// 004253D0
	void StatsShieldLevel(BinDlg* dialog, u32 index);																				// 00425510
	void StatsArmorLevel(BinDlg* dialog, u32 index);																				// 00425600
	void StatsWeaponLevel(BinDlg* dialog, u32 index, u32 weaponId);																	// 00425790
	bool StatSpidermineCount(BinDlg* dialog, u32 index);																			// 00426300

	// For get_unit_rank_string and checkHotkeyChar
	__int8 unitGetRace(Unit* a1);																									// 0047B150
	char* getStatTxtString(u16 a1);																									// 004C36F0

	// For stats_weapon_level / stats_armor_level
	char getArmorCount(Unit* unit, s32 a2);																							// 00453FC0
	void show_dialog(BinDlg* dialog);																								// 004186A0
	s32 disable_dialog(BinDlg* dialog);																								// 00418640
	void update_dialog(BinDlg* dialog);																								// 0041C400

	//// bunker_hooks
	Unit* findRandomAttackTarget(Unit* unit);																						// 00442FC0
	b32 attackApplyCooldown(Unit* unit);																							// 00478B40
	void setImageDirection(Image* image, s8 direction);																				// 004D5F80

	//// api
	void ApplySpores(Unit* unit);
	Bullet* create_bullet(u8 weaponId, Unit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction);
	void weaponBulletShot(Unit* target, Bullet* bullet, u32 dmgDivisor);
	Unit* createUnitAtPos(u16 unitType, u16 playerId, u32 x, u32 y);																// 004CD360
	Flingy* createFlingy(u32 flingy_id, u32 playerId, u32 x, u32 y);																	// 00496460

	//// unit_morph
	u8 set_archon_pathing(Unit* unit);																								// 0046A820
	bool has_supplies_for_unit(u32 unit_id, u32 playerId, b32 canShowErrorMessage);													// 0042CF70
	Unit* zerg_dual_birth(Unit* unit);																							    // 0045D910
	void order_new_unit_to_rally(Unit* unit, Unit* factory);																		// 00466F50
	void ai_training_unit(Unit* unit, Unit* creator);																				// 004A2830
	void delete_building_ai(BuildingAi* building, BuildingAiArray** buildingList);                                                  // 00404500
	void delete_worker_ai(WorkerAi* ai, WorkerAiArray** workerArray);                                                               // 00404470
	void delete_ai_town_if_needed(AiTown* town);                                                                                    // 004347E0

	//// hallucination_spell
	// for orders_Hallucination1
	bool spell_order(Unit* unit, u32 techId, u16* techEnergyCost, u32 sightRange, u32 error_message_index);							// 004926D0

	void UnitDestructor(Unit* unit);
	bool place_hallucination(Unit* unit);																							// 004F66D0

	// for createHallucination
	void apply_hallucination_stats(Unit* halUnit);																					// 004F6180
	s32 hallucinate_scarabs(Unit* a1, Unit* a2);																					// 004F65B0

	//// feedback_spell
	void replace_feedback_sprite(Unit* unit, u32 spriteId);																			// 004F6350

	//// weapon_effect
	bool isUnitDistanceWithin(Unit* target, Bullet* bullet, u32 radius);
	void doWeaponHit(Unit* target, Unit* attacker, u8 dmgDivisor);
	void AI_unit_was_hit(u32 main_target_reaction, u32 dont_call_help, Unit* attacker, Unit* target);

	// for airSplash
	Unit* findBestUnit(Box16* radius, s32 bullet, s32(*func)(Unit*, Bullet*));														// 004E8830
	s32 splashFunc(Bullet* bullet, Unit* unit);																						// 0048B570

	// For iterateLoadedUnits
	Unit* iterateLoadedUnits(Unit* unit, Unit*(__fastcall* func)(Unit*, Unit*), s32 a3);											// 004E6D00

	//// mindcontrol_spell
	void MindControlHit(Unit* attacker, Unit* target);																				// 004F6910

	//// weapon_damage
	u16 get_unit_strength(Unit* unit, bool useGroundStrength);
	Unit* createHallucination(Unit* target, u32 playerId);

	// For updateCorrosiveSpores
	u32 getCorrosiveSporesImage(Unit* unit);																						// 004F41F0

	//// recharge_shields
	void insertFirstOrder(Unit* unit, u8 orderId);																					// 004749D0
	u8 has_overlay(Unit* unit);																										// 0047B720
	bool isHangarUnit(Unit* unit);

	//// stopholdpos_orders
	Unit* MedicHeal_TargetAcquire(Unit* medic);																						// 004422A0
	u32 doMedicHeal(Unit* unit, Unit* target);																						// 00463C40
	void order_to_attack_target(Unit* unit, Unit* target, u32 unk1, u32 unk2);														// 00476FC0
	b32 attack_recent_attacker(Unit* unit);																							// 004770E0

	//// medic_orders
	b32 AI_OrderHeal(Unit* unit, Unit* target);																						// 00463530
	void function_4748E0(Unit* unit, s32 x, s32 y, u8 orderId);																		// 004748E0
	void update_target_something(Unit* unit, Unit* target, u8 orderId);																		// 00474940
	void order_interrupt(Unit* unit, Unit* target, u16 x, u16 y, u32 orderId);														// 00474C30
	void unitOrderMoveToTargetUnit(Unit* unit, Unit* target);																		// 00479FE0

	//// load_unload_orders
	b32 ai_are_on_connected_regions(Unit* unit, s32 x, s32 y);																			// 00438E70
	void ai_unload_failure(Unit* unit);																								// 0043DF30
	void insert_order_before(Unit* unit, Order* order, u8 orderId, s16 x, s16 y, Unit* target, u16 targetUnitId);					// 00474540
	void do_order(Unit* unit, u32 orderId, s32 x, s32 y, bool stopPreviousOrders);													// 00474C70
	b32 ai_update_attack_target(Unit* unit, u32 unk1, u32 unk2, u32 unk3);															// 00477160
	Unit* get_loaded_unit_from_index(Unit* transport, s32 index);																		// 004E6C40
	b32 get_first_loaded_worker(Unit* transport);																						// 004E6D90
	b32 ai_find_transport(Unit* unit, Unit** unitTargetOffset);																		// 004E6EF0
	void load_unit_proc(Unit* unit, Unit* unitToLoad);																				// 004E78E0	(Hooked)
	b32 unload_unit_proc(Unit* unit);																									// 004E7F70 (Hooked)
	void issue_order_targeting_ground(Unit* unit, u8 orderId, s16 x, s16 y);															// 00475260

	//// siege_transform
	void unitSetRetreatPoint(Unit* unit, u8 angleValue);																			// 004011A0
	bool getCloserToDestination_sub402BE0(Unit* unit);																				// 00402BE0
	void orderReturnToIdle(Unit* unit);																								// 00463770
	bool checkForCloseUnit(Unit* unit);																								// 00464290
	void playSubunitIscript(Unit* unit, IscriptAnimation::Enum anim);																// 00474380

	//// unit_train
	u32 getHPGainForRepair(Unit* unit);																								// 00402C40
	void refund_unit_train_cost(u32 unit_id, u8 playerId);																			// 0042CEC0
	void function_00432430(Unit* unit, u32 build_queue_slot);																			// 00432430
	void add_hangar_unit(Unit* main_unit, Unit* added_unit);																			// 00466300
	Unit* attemptTrainHatchUnit(Unit* trainer, u32 builtUnitID, u32 flag);															// 00468200

	//// weapon_fire
	// For progressBulletFlight
	void progressBulletMovement(Bullet* a1);																						// 0048B250
	void do_missile_damage(Bullet* bullet);																							// 0048B770

	// For bulletBehavior_Bounce
	Bullet* changeMovePos(Bullet* a1, s32 a2, s32 a3);																				// 00495240
	void progressBulletMovement_func(Bullet* a1);																					// 0048B250
	Unit* findNextBounceTarget_func(Bullet* a1);																					// 0048B1E0

	// For findNextBounceTargetProc
	bool unitIsSubunit(Unit* a1);																									// 00401D40
	bool unitIsCarrier(Unit* a1);																									// 00401470
	bool unitIsReaver(Unit* a1);																									// 00401490
	bool unitHasPathToUnit(Unit* a1, Unit* a2);																						// 0049CBB0
	s32 canBeInfested(Unit* a1, Unit* a2);																							// 00402210

	// For findNextBounceTarget
	Unit* iterateUnitsAtLocationTargetProc(Box16* a1, Unit*(__fastcall* a2)(Unit*, Unit*), Unit* a3);								// 004E8280

	// For create_bullet
	s32 initializeBullet(char playerId, Unit* parent, s16 y, Bullet* bulletResult, s32 weaponId, s32 x, s32 angle);					// 0048BEC0

	// For writeBullets
	/*
	void packBulletData(Bullet* a1, s32 a2); // 0048A9A0
	size_t _lockwrite(const void* a1, size_t a2, size_t a3, FILE* a4); // 00411931
	void* SMemAlloc(s32 a1, char* a2, s32 a3, s32 a4); // 0041006A
	s32 compressWrite(void* a1, u32 a2, FILE* a3); // 004C3450
	bool SMemFree(void* a1, char* a2, s32 a3, char a4); // 00410070
	Bullet* unpackBulletData(Bullet* a1, s32 a2); // 0048A8D0
	*/

	//// wpnspellhit
	bool can_unit_id_fit_at_xy(s32 x, s32 y, u32 unit_id);														    // 0042D810
	Unit** get_all_units_in_bounds(Box16* coords);																	// 0042FF80
	void increment_unit_kill_count(Unit* unit);																		// 004759C0
	u32 get_unit_damage_bonus(Unit* unit, u8 weaponId);																// 00475EC0
	void attack_overlay_and_notify(Unit* attacker, Unit* target, u32 weaponType, u32 direction);			        // 00479730
	Thingy* create_thingy(u32 spriteId, s16 x, s16 y, u32 playerId);												// 00488210
	u32 get_unit_bullet_damage(Bullet* bullet, Unit* target);														// 0048ACD0
	u32 get_spell_stat_string(Unit* target, u32 techId, u8 playerId);									            // 00491E80
	void disableUnitFunc(Unit* unit);																				// 00492CC0 (hooked in weapon_spellhit)
	void setAllImageGroupFlagsPal11(Sprite* sprite);																// 00497430
	void IterateUnitsAtLocationTargetProc_CorrosiveAcid(u32 attackingPlayerId, Box16* coords);						// 004E8280
	void IterateUnitsAtLocationTargetProc_Ensnare(Unit* attacker, Box16* coords);									// 004E8280
	void iterate_units_at_location_target_proc_plague(Unit* attacker, Box16* coords);									// 004E8280
	void plague_proc(Unit* unit, Unit* attacker);																	// 004F4AF0
	void IterateUnitsAtLocationTargetProc_Maelstrom(Unit* attacker, Box16* coords);									// 004E8280
	void IterateUnitsAtLocationTargetProc_StasisField(Unit* attacker, Box16* coords);								// 004E8280
	void createEnsnareOverlay(Unit* a1);																			// 004F45E0

	// For createScanner
	s32 setUnitStatTxtErrorMsg(s32 a1);																				// 0048CCB0
	s32 play_sound_from_unit(s32 soundId, Unit* unit, s32 bPlay, s32 minVolume);									// 0048ED50
	void return_all_interceptors(Unit* unit);																		// 004655F0
	char ignore_all_scripts_and_go_to_idle(Unit* unit);																// 00476550

	// For maelstromProc
	void orders_self_destructing(Unit* unit);																		// 00475710
	char set_maelstrom_timer(Unit* unit, u8 time);																	// 004553F0

	// For CBulletDamage
	s32 _fastcall lurkerSplashProc(Unit* target, Bullet* bullet);													// 0048B680
	s32 _fastcall groundSplashProc(Unit* target, Bullet* bullet);													// 0048B750
	s32 _fastcall airSplashProc(Unit* unit, Bullet* bullet);														// 0048B150
	s32 _fastcall airSplashProc2(Unit* unit, Bullet* bullet);														// 0048B730
	bool unitHasNoMainOrderType(Unit* unit);																		// 00475A50
	s32 setEventUnitComplete();																						// 004C36C0

	// For findBestUnit
	Unit** findAllUnits(Box16* coords);																				// 004308A0

	//// selection
	bool unit_is_owned_by_current_player(Unit* unit); 																	// 00401170
	bool isUnitBurrowed(Unit* unit);																				// 00402A70
	void sc_memcpy_0(u32 dest, u32 src, u32 memsize);																// 00408FD0
	void show_rally_target(Unit* unit);																				// 00468670
	bool unit_is_unselectable(u16 unit_id);																			// 0046ED80
	void function_0046F040(Unit* current_unit, Unit** unit_list, Unit* clicked_unit, u32 list_length);				// 0046F040
	u32 combine_lists(Unit* unit, Unit** unit_list_1, Unit** unit_list_2, u32 list_length);				// 0046F290
	void apply_new_selection(Unit** unit_list, u32 unit_list_length);											// 0046FA00
	u32 unit_to_unit_id(Unit* unit);																					// 0047B1D0
	bool unit_is_standard_and_movable(Unit* unit); 																	// 0047B770
	void select_building_sfx(Unit* unit);																				// 0048F910
	void create_new_unit_selections_from_list(Unit** unit_list, u32 unit_list_length);									// 0049AE40
	void cmdact_select(Unit** unit_list, u32 unit_list_length); 													// 004C0860

	//// cloak_nearby_units
	void secondaryOrder_Cloak(Unit* unit);																			// 00491790

	//// update_unit_state
	bool unitHpIsInRedZone(Unit* unit);																				// 004022C0
	void RestoreAllUnitStats(Unit* unit);																			// 00492F70 (hooked by hooks\update_status_effects.cpp)

	// For update_units
	bool is_selected_unit_group_enabled();																			// 0049A2C0
	char refresh_layers_3_and_4();																					// 0048D9A0
	void cancel_target_order();																						// 0048CA10
	s32 update_disruption_web_status();																				// 004EB2F0
	Unit* progress_dying_unit_frame(Unit* unit);																	// 004EB5E0
	void reveal_sight_at_location(s32 sightRange, u32 visionFlags, s32 ptX, s32 ptY, bool isAirUnit);				// 004806F0
	void update_visibility(Unit* unit);																				// 004EBE10
	void progress_active_unit_frame(Unit* unit);																	// 004EBC30
	void check_unit_visibility(Unit* unit);																			// 00443390
	void progress_unit_frame(Unit* unit);																			// 004ECF70
	void unit_update(Unit* unit);																					// 004EC420
	void update_unit_power();																						// 00493790

	// For update_burrowing_cloaking_units
	s32 is_unit_cloaking(Unit* unit);																				// 004916E0
	void play_sound_from_direction(Unit* unit, s32 soundId);														// 0049B5B0

	// For do_cloak
	void update_sprite_flag_on_palette(Sprite* sprite);																// 00497E80
	void show_images_hidden_by_cloak(Sprite* sprite);																// 00498170
	void update_sprite_images_for_cloak(Sprite* sprite);															// 00498A90

	// For progress_unit_frame
	char refresh_sprite(Sprite* sprite, s32 spriteId);																// 004EBBD0
	void updateUnitTimers(Unit* unit);																				// 004EC290
	void progress_order(Unit* unit);																				// 004EC4D0
	void perform_secondary_orders(Unit* result, s32 unused);														// 004ECF70
	void sprite_to_iscript_loop(Sprite* sprite);																	// 00497920

	//// select_larva

	u32 getLarvaeUnitsFromList(Unit** unitList, Unit** outputList);													// 00423190
	void select_multiple_units_from_unit_list(Unit** unitList, u32 unitsCount, b32 unk1, b32 unk2);					    // 0049AEF0

	//// uifeatures

	s32 BWFXN_PrintXY(const char* text, s32 x, s32 y);
	s32 GetColorHandle(u8 color); // Colors: 1 - last color, 2 - 8, 14 - 17, 21 - 25, 27 - 31

	// For targetedOrder
	Order* queue_new_order(Order* a1, s16 a2, s16 a3, Unit* a4, u8 a5, s32 a6, s16 a7, Vector32* a8, Unit* a9);		// 00474B90
	s32 show_stat_txt_to_player(s32 a1, s32 a2);																	// 0048CF00
	void set_order_target(u8 a1, s16 a2, s16 a3, s32 a4, s32 a5, Unit* a6, s32 a7, Vector32* a8, Unit* a9);		    // 00475470

	// For selectUnits
	s32 select_single_unit_from_id(s32 index);																		// 00496D30
	s32 select_multiple_units_from_unit_list(s32 a1, Unit** a2, u8 a3, s32 a4);										    // 0049AEF0
	s32 SNetGetTurnsInTransit(u32* turns);
	u8 sendTurn();
	s32 SErrGetLastError();
	u8 packetErrHandle(s32 result, s32 a1, const char* a3, s32 a4, s32 a5);

	//// repair_order

	void getRepairInfo(Unit* repairTarget, u32* repairCostMineral, u32* repairCostGas, s16* unkRepairInfo);			// 00466D10
	bool verifyResources(u32 mineralCost, u32 gasCost, u32 playerId);												// 00466EE0
	bool subtract_repair_resources(Unit* worker, Unit* target);															// 00466FB0

	// For isOrderValid
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
	Unit* unit_constructor(s32 x, s32 y, u16 unitType, s32 playerId, s32 unused);						// 004A06C0
	Unit* list_push_back_used_unit_entry(Unit* unit);														// 0049DCA0
	Unit* list_push_back_hidden_unit_entry(Unit* unit);														// 0049DCE0
	Sprite* unit_destructor(Unit* unit);																	// 004A0990
	void* get_image_attack_frame(void* resultArg, Sprite* sprite, s32 index, u32 overlay_type);			// 00401DF0
	void set_sprite_main_image_offset(Sprite* sprite, char x, char y);										// 00401E40

	//// psi_field_util
	bool addPsiFieldSprite(Unit* unit);

	// stim_packs
	void queue_game_command(u8* params, u32 param_length);

	// misc
	Thingy* createThingyFunc(u32 spriteId, s16 x, s16 y, u32 playerId);
	bool func_dounitscollide(Unit* first, Unit* second);
	void unburrow(Unit* unit);
	Image* CreateHealthBar(s32 a1, Sprite* a2);
	void RemoveHPBar(Image* a1);
	void refund_all_queue_slots(Unit* unit);
	void* SMemAlloc(u32 amount, char* unkFilename, u32 unkLine, u32 defaultValue);	                    // 0041006A (based on Storm.dll definition)
	void set_callback_timer(BinDlg* dialog, u32 unk1, u32 unk2, u32 unk3);			                    // 00416090
	void remove_from_selections(Unit* unit);																// 0049A7F0
	u32 fxn_interact(BinDlg* dialog, u32 data_struct_offset);
	void register_user_dialog_action(BinDlg* dialog, u32 array_of_fxnInteractFunc, u32 sizeOfArray);	// 00418100
	void enable_control(BinDlg* dialog);																// 00418E00
	void replay_stat_buttons(BinDlg* dialog);															// 00427A80
	void update_command_card();																			// 004591D0
	void refresh_button_tooltip();																		// 00459770
	void BINDLG_BlitSurface(BinDlg* dialog);															// 004C35F0
	u32 fxn_interact(BinDlg* dialog, u32 data_struct_offset);
	s32 req_check(u32 requirement_func, u16 requirement_var, u32 playerId, Unit* unit);
	void updateMineralPatchImage(Unit* mineralPatch);
	void update_image_position_offset(Image* image);

	// Identical to function @ 0x004F3AF0
	void set_resource_amount_carried(Unit* worker, u8 amountCarried, u32 chunkImageId, bool isMineral);

	// ai misc
	s32 ai_has_buildings_needing_power(u32 player);
	s32 ai_count_economy_units(u32 player, u32 unit_id);
	s32 get_supplies_remaining(u8 player, u16 unit_id);
	s32 get_supplies_used(s32 byte_data, u8 player);
	bool is_ai_allowed_to_build(s32 a1, s32 a2);
	void applyCloakingOrder(Unit* unit);																	// 00491B30

	//// use_tech
	const void issueTriTargetOrder(u8 order1, u8 order2, u8 order3);										// 0048CAF0
	s32 checkEnergyForTechUse(s32 tech_id);																	// 00423660

	//// pathfinding
	s32 isPointAtUnitBorder(s32 unused, Unit* other, Unit* self, s32 position_xy);							// 004EB720
	s32 AI_PrepareMovingTo(Unit* unit, s32 x, s32 y);														// 004E7420

	// order attack behavior
	bool hasGroundWeaponGraphic(Unit* unit);
	bool isUnreachable(Unit* target, Unit* unit);
	bool is_unit_target_out_of_max_range(Unit* unit, Unit* target);											// 00476430
	s32 moveForMeleeRange(Unit* target, Unit* unit);

	// For createUpgradeLevelTooltip
	s16 create_tooltip(char* text);																			// 004591A6
	char refresh_selection_screen();																			// 00481480

	// lobby.cpp
	
	// For initGame
	void switchAllAvailableComputerSlotsToOpen();															// 004DBBE0
	bool setLastOpenSlotToComputer(s8 race);																// 004DBB70

	// For startGame
	s32 bootReason(u8 player);																				// 004B8870
	s32 isAuthedMap(void* a1);																				// 004C4870
	void setTimerFunc();																					// 00472E00
	s32 getTickCount();																						// 004FE0C4

	// For initializeBullet
	Flingy* initializeFlingy(s32 flingy_id, s16 y, s32 x, Flingy* flingy, char player, char angle);			// 00496360
	void bulletBehaviorReassign(Bullet* bullet);															// 0048BD50
	s32 weaponBehaviorCanHitTargetCheck(Unit* target, Unit* attacker);										// 004765B0
	void appearOnTarget(Bullet* bullet, s16 x, s16 y);														// 004955C0
	Bullet* attackFlyToTarget(Bullet* bullet, s32 x, s32 y);												// 00494B70

	// For ai_progress_build
	void display_error_message_id_with_sfx(s32 playerId, s32 messageId, s32 soundId);							// 0048EE30

	// For ai_manage_building_placement
	s32 get_bunker_placement_position(s32 placementId, s32* regionId);											// 004442E0
	Unit* get_disabled_unit_in_range(s32 owner, Point16 position);												// 00440420
	void ai_get_build_location(s32 unit_id, char* placementData, s32 playerId, Point16 tryPosition,				// 004461B0
		s32 maxRange);
	void ai_placement_data_something(s8* placementData, Point16 tryPosition);									// 00443610
	u32 getTileDistance(s32 x, s32 y, s32 x2, s32 y2);														// 00444100

	// For initializeUnitCounts
	char teamGame_CountRaces(char force, char* bIsLegal, char race);										// 004857B0

	// For StatRes_MainProc
	void showSupplyInfo(BinDlg* dialog, s16 graphic, s16 mathA, u16 mathB);									// 004E53B0
	void printStatResInfo(BinDlg* dialog, s16 graphic, char** a3, char* a4, s32 bSomething);				// 004E52E0

	// For redrawSupplyInfo
	bool unitStatCond_Standard();																			// 00424980

	// For drawSupplyUnitStatus
	void draw_shield_upgrade(BinDlg* dialog, u16 statusScreenIndex);											// 00425510
	void drawArmor(BinDlg* dialog, s32 unit_id, u16 statusScreenIndex);										// 00425600

	// For progressBulletFrame
	void bulletBehaviorFly(Bullet* bullet);																	// 0048B3B0
	void bulletBehaviorFollow(Bullet* bullet);																// 0048B500
	void bulletBehaviorBounce(Bullet* bullet);																// 0048B2D0
	void bulletBehaviorPersist(Bullet* bullet);																// 0048BC70
	void bulletBehaviorInstant(Bullet* bullet);																// 0048A560
	void bulletBehaviorArea(Bullet* bullet);																// 0048B440

	// For giveUnitToPlayer
	void giveAllUnitPropertiesToPlayer(Unit* unit, s8 playerId);											// 0047C9F0
	Unit* getHiddenMiner(Unit* unit);																		// 00468A60
	bool unitHasHanger(Unit* unit);																			// identical to 004014B0
	s32 forHangarUnits(Unit* unit, Unit* (__fastcall* func)(Unit*, Unit*), s32 arg);						// 00465200
	Unit* getCreatingUnit(Unit* unit);																		// 00466A30

	// For UnitMove
	void MoveSprite(Sprite* sprite, s16 x, s16 y);															// 00497A10
	u32 updateUnitSortingInfo(u32 x_diff, Unit* unit, u32 y_diff);											// 0046A000

	// For bulletBehaviorReassign
	void playSpriteIscript(Sprite* sprite, char scriptId, s32 bPlay);										// 00499D00
	Bullet* assignWeaponData(Bullet* bullet, char behaviorType);											// 0048AA80

	// For ai_training_normal
	AiRegion* getAiRegionInfoFromUnitLoc(Unit* unit);														// 00438BF0
	void add_guard_ai_to_unit(Unit* unit);																	// 00462960

	s8 create_unit_allowed(u16 unit_id, Unit* unit, s32 playerID);												// 0046E1C0

	b32 SBmpLoadImage(const char *filename, void* palette, void* buffer, u32 buffersize, u32* width, u32* height, u32* bpp); // 00410154

	// For ai_military_should_wait_for_group
	bool ai_are_on_connected_regions(Unit* unit1, Unit* unit2);												// 00438E10
	bool ai_check_military_grouping(Unit* unit);															// 00439550

	// For remove_unit_ai
	s32 remove_from_ai_structs(Unit* unit, UnitAi* unitAI);													// 00439D60
	s32 remove_worker_or_building_ai(Unit* unit, UnitAi* unitAI);											// 00434C90
	void ai_remove_medic(Unit* unit);																		// 004477C0

	// AI utility
	void give_ai(Unit* unit);																				// 00463040
	void ai_training_simple_military(Unit* newUnit, Unit* sourceUnit);										// 0043E280

	// For delete_ai_town_if_needed
	AiTown* try_begin_delete_town(AiTown* town);																// 00434330
	AiScript* delete_aiscripts_using_town(AiTown* town);														// 0045AE20

	// For displayEstablishingShot
	void* loadInitCreditsBIN(char* a1);																		// 004D8DF0
	bool SFileOpenArchive(char* archiveName, u32 dwPriority, u32 dwFlags, void* const* handle);				// 004100B2
	bool SFileCloseArchive(void* archive);																	// 004100BE
	bool SFileOpenFileEx(void* hMPQ, const char* szFileName, u32 dwSearchScope, void** phFile);				// 004100C4
	//bool SFileCloseFile(void* hFile);																		// 004100C4

	// For ai_are_on_connected_regions
	bool ai_regions_are_connected(u32 playerId, u32 region1, u32 region2);									// 00437E70

	// For BTNSCOND_ZergBuildingRally
	u32 BTNSCOND_Rally(u32 unused, u32 player, Unit* unit);													// 00429520

	// For ai_get_build_location
	u32 ai_get_building_placement_data(u32 maxRange, u32 unit_id, char* placementData, u32 playerId, Point16 desiredPos);		// 00445D60
	char* ai_remove_unreachable_regions(char* placementData, Point16 desiredPos);												// 00443900
	u32 ai_remove_dangerous_regions(u32 unit_id, char* placementData, u32 placementId, Point16 desiredPos);						// 00443870
	u32 ai_remove_far_tiles(char* placementData, Point16 desiredPos, u32 maxRange);												// 00444E20
	u32 ai_remove_different_height(char* placementData, Point16 desiredPos);													// 004436C0
	u32 ai_placement_related_something(u16 unit_id, char* placementData, Point16 desiredPos);									// 00443E30
	void ai_remove_unfit_tiles(u32 unit_id, char* placementData, u32 playerId, Point16 desiredPos);								// 00444CE0
	u32 ai_remove_addon_places(u32 unit_id, char* placementData, u32 playerId, Point16 desiredPos);								// 00443CC0
	u32 get_turret_placement_position(char* placementData, u32 playerId, Point16 desiredPos, u16 unit_id, u32 unkFlag);			// 00443AC0
	u32 get_pylon_placement_position(char* placementData, u8 playerId, Point16 desiredPos);										// 004448D0
	u32 get_default_placement_position(u32 unit_id, char* placementData, u32 playerId, Point16 desiredPos);						// 004449A0

	// For ai_check_military_grouping
	void get_region_center_from_id(u16 regionId, u32* out_x, u32* out_y);														// 0049CAA0

	// For ignore_iscript_and_go_to_idle
	void play_idle_iscript(Sprite* sprite);																						// 00497880

	// For issue_order_targeting_ground
	void order_copy(s32 a1, Unit* a2, s8 a3, Point16* a4, Unit* a5, s32 a6);													// 00447810

	// For unit_destructor_special
	s32 removeCreepAtUnit(s32 left, s32 top, s16 unit_id);																		// 00414560

	//// For direct_sound

	// For soundThread
	void fastFileRead(s32 a1, s32 a2, char* a3, s32 fileName, s32 buffer, s32 bytesRead, char* logFileName, s32 logLine);		// 004D2D10
	s32 parseWaveFile(s32 a1, s32 a2, s32 a3);																					// 004BBD80
	bool SMemFree(void* location, char* logFileName, s32 logLine, char defaultValue);											// 00410070
	u32 getSoundDuration(s32 a1);																								// 004BB890

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
	s32 sRegLoadValue(char keyName, char valueName, char a3, char value);														// 00410076
	HANDLE beginThreadEx(s32 lpThreadAttributes, s32 dwStackSize, s32 threadId, s32 a4, s32 dwCreationFlags, s32 lpThreadId);	// 0040CD3C
	*/

	//// For ai_requests

	// For ai_perform_task
	void ai_overlord_behavior(Unit* unit);																						// 00434DF0
	void ai_nuke_silo_behavior(Unit* unit);																						// 00434D50
	bool ai_unit_construction_behavior(s32 playerId, Unit* unit, s32 someBool);													// 00433890
	char ai_plan_do_merge_morph(s32 playerId, Unit* unit, s32 someBool);															// 00435F10
	s32 ai_morph_behavior(s32 player, Unit* unit, void* unit_ai);																// 00434FF0
	void ai_train_behavior(Unit* unit, void* unit_ai, s32 player);																// 00434720
	void ai_upgrade_behavior(Unit* unit, s32 player);																			// 00434670
	s32 ai_research_behavior(Unit* unit, s32 player);																			// 0043462F - return value unused
	char ai_construct_behavior(Unit* unit, void* unit_ai, s32 player);															// 00434E70
	s32 ai_make_detectors_behavior(Unit* unit, s32 player);																		// 00435130

	// For ai_plan_do_merge_morph
	Unit* ai_get_mutalisk_for_morph(s32 player);																					// 004375F0
	s32 ai_get_high_templar_for_summon(s32 player, Unit** first, Unit** second);													// 00437410
	s32 ai_get_dark_templar_for_summon(s32 player, Unit** first, Unit** second);													// 00437320
	void assign_order_with_target(Unit* unit, Unit* target, s32 orderId);														// 004752B0
	s16 ai_reserve(s32 player, s32 someBool);																					// 00447D00
	AI_Main* ai_pop_spending_request(s32 player);																					// 004476D0
	Unit* AI_GetHydraliskForMorph(s32 player);																					// 00437570
	s32 is_unit_id_race_equal_to_unit_race(u16 unit_id, Unit* unit);																			// 00432480
	bool hasResourcesForUnit(u16 unit_id, s32 player);																			// 004487B0
	char ai_perform_task(Unit* unit);																							// 004361A0
	s32 ai_get_request_build_type(s32 player);																					// 00447590
	bool ai_train_unit(Unit* unit, u16 unit_id, char requestId, void* value);														// 004672F0
	
	// For scenario.cpp
	Unit* ai_create_unit(u32 unit_id, s32 player, s32 x, s32 y);																	// 004A0AD0
	s32 parse_aiscript_name(s32 name, s32* is_bw);																				// 0045AEF0
	AiScript* run_aiscript(Box32* area, u32 player, u32 block, u32 is_bw);													// 0045B170
	AiScript* ai_stop(AiScript* ai_script);																						// 00403380

	AiTown* aiscript_start_town(u32 x, u32 y, u32 resarea, u32 player);															// 00434220
	AiTown* aiscript_start_areatown(Box32* area, u32 resarea, u32 player);														// 00434060

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

	typedef BOOL __stdcall SFileOpenFile_Func(const char *filename, HANDLE *phFile);
	typedef BOOL __stdcall SFileCloseFile_Func(HANDLE hFile);
	typedef BOOL __stdcall SFileDdaBeginEx_Func(HANDLE directsound, DWORD flags, DWORD mask, unsigned __int32 lDistanceToMove, signed __int32 volume, signed int a6, int a7);
	typedef BOOL __stdcall SFileDdaEnd_Func(HANDLE directsound);
	typedef LONG __stdcall SFileGetFileSize_Func(HANDLE hFile, LPDWORD lpFileSizeHigh);
	typedef int __stdcall SFileReadFile_Func(HANDLE a1, void* a2, DWORD a3, LPDWORD a4, LPOVERLAPPED a5);

	extern SFileOpenFile_Func* SFileOpenFile;
	extern SFileCloseFile_Func* SFileCloseFile;
	extern SFileDdaBeginEx_Func* SFileDdaBeginEx;
	extern SFileDdaEnd_Func* SFileDdaEnd;
	extern SFileGetFileSize_Func* SFileGetFileSize;
	extern SFileReadFile_Func* SFileReadFile;

	typedef void   __cdecl playNextMusic_Func();
	typedef HANDLE __cdecl stopMusic_Func();

	extern playNextMusic_Func* playNextMusic;
	extern stopMusic_Func* stopMusic;

	void prepare_formation_movement(MovementGroup* a1, char a2);													// 0049A8C0
	Unit* find_fow_unit(__int16 ax0, __int16 a2, unsigned __int16 a3);											// 00487FD0
	void targeted_order_unit(Unit* a1, Unit* a2, int a3, char a4);												// 004756E0
	void targeted_order_ground(int a1, int a2, Unit* a3, int a4, int a5, int a6);								// 004756B0
	Unit* get_next_commanded_unit();																				// 0049A850
	void get_formation_movement_target(Unit *a1, MovementGroup* a2);												// 0049A500

	u8 get_right_click_order(u32 type, Unit* unit, s32 x, s32 y, Unit** target, s32 unit_id);

	void edit_unit_flags(Unit* unit, u16* flags);																	// 004CD480
	void unit_placement_error(const char *a1, char a2);

	void ai_build(u32 unit_id, b32 only_if_needed, u32 count, AiTown* town, u32 priority);						// 004323D0
	void ai_add_build_request(AiTown* town, s32 method, s32 count, s32 id, s32 priority, b32 only_if_needed);		// 00431F90

	// For HandleWindowMessages
	char check_last_file_error();																				// 004D28D0
	s32 has_messages_waiting(void* tag_msg, s32 b_something);													// 004D1650
	void send_mouse_move_event(void* fast_call_func);															// 004D1AE0
	s32 send_input_to_all_dialogs(DialogEvent* dlg_event);															// 00419FD0
	VectorHead* play_briefing_wav_begin();																		// 004BBE50
	void iterate_timers();																						// 00415DC0

	// For ai_train_defensive_military
	u32 ai_region_train_attack_force(AiRegion* region);															// 0043E670
	u32 ai_defend_region(AiRegion* military_region, s32 defense_region, s32 against_air, s32 region_strength);	// 0043F100
	u32 ai_add_base_unit_spending_request(u32 final_unit_id, u32 player_id, AiRegion* region, u8 priority);		// 004484A0

	bool unit_has_power(u32 id, u32 x, u32 y, u32 player);

	void play_sound_global(u32 id);																		// 004BC2C0
	VectorHead* play_sound_from_file(const char* filename, char* unused);								// 004BC490

	b32 is_valid_for(u32 a1, Unit* unit, u32 a3);
	b32 region_is_for_attack_prepare(AiRegion* region);

	u32 ai_count_units_in_town(AiTown* town, u32 unit_id, bool finished);									// 00433210 Aka AI_WaitBuild

	u32 load_scenario_singleplayer(char* title, u32 unused, char* error, char* player, u32 speed);				// 004A8590
	u32 get_ground_height_at_px(s32 y, s32 x);															// 004BD0F0
	s32 is_ai_target_related(Unit* attacker, Unit* target);												// 004A1140
	s32 get_threat_level(Unit* attacker, Unit* target);													// 00442160
	u32 add_unit_to_possible_targets(s32 threat_level, Unit* unit);										// 00440160

	void enable_doodad_state(Unit* unit);																// 0047BA00
	void disable_doodad_state(Unit* unit);																// 0047B960
	char unfreeze_unit(Unit* unit);																		// 004F62D0
	s32 dialog_set_progress_bar_value(BinDlg* dialog, s16 index, s16 value);							// 00457200

	// For vespene rework in harvest.cpp
	void set_gathering_flags(Unit* unit);																// 00468AA0
	bool get_gas_exit_point(Unit* worker, Unit* resource_node, Point16* position, s32* direction);		// 00468B00
	void move_unit(Unit* unit, s16 x, s16 y);															// 004EB9F0
	Unit* finished_mining(Unit* worker, Unit* resource_node);											// 00468E40
	Order* delete_order(Unit* unit, u8 order_id);														// 00474400
	void order_targeting_nothing(u8 order_id, Unit* unit, u32 stop_previous_orders);					// 00474D60
	void hide_unit_partial(Unit* unit, s32 hide_unit);													// 00493CA0

	void Order_BeginHarvest(Unit* node, Unit* worker);													// 00468D10
	Unit* remove_harvesters(Unit* node);																	// 00468770
	Unit* remove_from_current_harvesters(Unit* worker, Unit* node);										// 004686D0

	s32 ai_scv_repair_controller(Unit* unit, AiTown* town);												// 00433C70
	s32 ai_try_progress_spending_queue_worker(AiTown* town, Unit* unit);								// 004348C0
	bool unit_is_mineral_field(Unit* unit);																// 00468890
	void ai_auto_harvest(Unit* resource, Unit* worker, Unit* fallback);									// 00433B90
	Unit* harvest_next_nearest_resources_ex(s32 worker_count_bool, Unit* worker, Point16 worker_pos);	// 004432D0
	Unit* get_ai_refinery(AiTown* town, s32 gas_state, s32 index);										// 00432B90
	u32 ai_get_vespene_type(Unit* node, s32 player_id);													// 004326E0
	void ai_resource_need_split(u8* out_gas_percentage, u32 player, u8* out_mineral_percentage);		// 004475B0
	u32 ai_add_town_spending_request(u32 player, u32 priority, u32 unit_id, void* param);				// 00448400

	u32 get_closest_base_from_startloc(u32 player_id, u32 x, u32 y);									// 00443A20
	Unit* get_resource_area_gas(u32 resarea, Unit** gas_1, Unit** gas_2, Unit** gas_3);					// 00444460
	Unit* get_resource_area_mineral(u32 resarea);														// 004445E0
	u32 get_optimal_mineral_workers(u32 resarea, u32 player);											// 004439B0
	void ai_get_old_peons(AiTown* town, u32 count);														// 004355C0

	bool unit_is_factory(Unit* unit);																	// 0047B2E0
	bool is_unit_target_inside_min_range(Unit* unit, Unit* target);										// 004764D0
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

	s32 get_creep_spread_area(u32 unit_id, u32 x, u32 y, b32 completed, Box32* rect);							// 00413DB0

	typedef s32 ClipCreepAreaInBounds_Callback(u32, u32, u32, u32, u32, u32, u32);
	s32 clip_creep_area_in_bounds(u32 tile_x, u32 tile_y, ClipCreepAreaInBounds_Callback* callback);			// 00413D10

	s32 mark_creep_to_disappear(u32 tile_x, u32 tile_y);														// 0047DD60

	void* process_options_change(BinDlg* control);															// 004C9780
	void load_dialog(char* filename, void* function);														// 004F57A0
	BinDlg** destroy_dialog(BinDlg* control);																// 00419EA0

	typedef void main_menu_options_custom_interact_func(BinDlg* control);
	extern main_menu_options_custom_interact_func* main_menu_options_custom_interact;

	typedef bool __fastcall game_menu_dialog_interact_func(BinDlg* control, DialogEvent dlg_event);
	extern game_menu_dialog_interact_func* game_menu_dialog_interact;

	u32 stop_creep_disappearing(u32 x_tile, u32 y_tile);													// 0047D540

	void secondary_order_spawning_larva(Unit* unit);														// 004EA780

	AiTown* create_ai_town(u32 player, u32 x, u32 y);														// 00432020

	Sprite* sprite_destructor(Sprite* sprite);																// 00497B40
	UnitPath* reassign_path(Unit* unit);																	// 0042F740
	u32 initialize_unit(Unit* unit, u32 y, u32 unit_id, u32 x, u8 player_id);								// 0049F206

	bool unit_is_geyser_ex(Unit* resource_node, Unit* worker);												// 00468930
	void prepend_order(Unit* unit, u8 order_id, Point16 target_pos, Unit* target_unit);						// 00474AF0
	bool ai_is_building_safe(Unit* building);																// 00438910
	Unit* ai_find_best_repair_unit(Unit* unit);																// 00443160

	u32 ai_find_expansion(u32 player, u32 resarea, u32 amount);												// 004453E0

	SCBW_FUNC(0x0045B440, void __stdcall, ai_expand_to, AiScript* aiscript, u32 resarea, u32 block);
	SCBW_FUNC(0x004A1E50, void __fastcall, remove_unit_ai, Unit* unit, UnitAi* unit_ai);
	SCBW_FUNC(0x00487B00, Thingy* __stdcall, find_blocking_fow_resource, u32 x, u32 y, u32 radius);
	SCBW_FUNC(0x00493640, void __cdecl, create_pylon_aura);
	SCBW_FUNC(0x0040ABBE, void __fastcall, image_render_fxn_0_0, u32 screenX, u32 screenY, void* frame, Box32* rect, u32 colorData);

	b32 ai_try_expanding_to(u32 resarea);																	// 00443A00

	char get_target_acquisition_range_upgrade(Unit* unit);													// 00476000

	Unit* let_next_unit_mine(Unit* worker);																	// 00468970
	s32 begin_harvest(Unit* worker, Unit* node);															// 00468C70

	u32 get_missing_attack_force_units(AiRegion* ai_region, u32* attack_wave, u32 is_campaign);				// 00438050
	s32 ai_defender_bunker_behavior(Unit* unit, u16 region_id);												// 0043C480

	bool ai_has_incomplete_military_in_region(AiRegion* ai_region);											// 00436EB0

	void delete_military_ai(MilitaryAi* military_ai, AiRegion* ai_region);									// 004371D0

	bool ai_has_incomplete_military_in_region(AiRegion* region);											// 00436EB0

	void update_burrowing_cloaking_units();																	// 00492EB0
	s32 use_weapon_iscript(Unit* unit, u8 weapon_id, u32 flag_or_timer, char iscript_anim);					// 00476C90

	bool place_building_resource_restriction_check(u32 check_vision, u16 tile_x, u16 tile_y);				// 00473990

	Sprite* create_sprite(u32 sprite_id, u32 x, u32 y, u32 player);											// 004990F0

	void refresh_all_visible_images_at_screen_position(Sprite* sprite, u8 vision_filter);					// 00497480

	void set_tooltip_info(u32 mineral_cost, u32 gas_cost, u32 p_energy, u32 t_energy, u32 z_energy, u32 p_supply, u32 t_supply, u16 z_supply); // 00458800

	s32 bw_print_icon(Bounds* bounds, FxnDrawIconData* data, u32 right, u32 bottom);						// 0041D090
	u32 bw_print_xy(char* dest, s32 width, s32 height);														// 004202B0

	bool unit_is_in_notify_range(Unit* unit);																// 004E5AD0
	u32 reset_all_unit_attack_notify_timers_in_range(Unit* unit);											// 0048EAE0
	void do_unit_event_notification(Unit* unit, char event_type, u32 timer, bool ping, u32 message_id);		// 0045EFE0

	s32 stat_data_unit_wireframe_select_update(BinDlg* dialog, Event* event_arg);							// 00456F50
	char status_screen_update_tooltip_mouse_held(BinDlg* dialog, Event* dialog_event);						// 00457DE0
	u32 status_screen_button(BinDlg* dialog_1, BinDlg* dialog_2);											// 00458220

	void status_screen_draw_stat_button_border(BinDlg* dialog);												// 00456D30
	u32 status_screen_wireframe_coloring_related(Unit* unit);												// 004567C0
	char status_screen_wireframe_shield_related(u32 unused, Unit* unit);									// 004566B0
	u32 status_screen_wireframe_hp_related(Unit* unit);														// 00456730

	char toggle_unit_pathing_properties(Unit* unit);														// 004030D0

	u32 is_unit_at_height(Unit* unit, u32 height);															// 0045F8D0
	bool kill_units_at_location_proc(Unit* unit, KillUnitArgs* args);										// 004C7E20
	void kill_powerup(Unit* unit);																			// 004F3C60

	void send_message_something(u32 unused, const char* string);											// 004F31D0
	u32 set_ingame_input_procs();																			// 00484CC0
	void update_current_buttonset();																		// 004599A0
	bool is_unit_owned_by_player_ex(u32 player, Unit* unit);												// 0045FEF0

	char issue_place_building_order(char* unused, u32 unit_id, char order_id);								// 0048E730

	bool compare_unit_rank(Unit* unit, Unit* compare_unit);													// 0049A350

	char input_target_order_left_mouse_click(Event* event_veeq);											// 004BD500
	SCBW_FUNC(0x004845F0, void __cdecl, input_target_order_right_mouse_click);

	char input_place_building_left_mouse_click(Event* event, u32 unused, Point16 position);					// 0048E5D0
	SCBW_FUNC(0x00484600, void __cdecl, input_place_building_right_mouse_click);

	void initialize_input_procs();																			// 004D2FF0

	u16 get_minimap_cursor_pos(u16* x, u16* y);																// 004A3D70
}