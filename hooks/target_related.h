#pragma once
#include "BW/api.h"
#include "BW/bwdata.h"

namespace hooks {
	bool is_unit_in_weapon_range(Unit* target, Unit* attacker);															// 00476870
	Unit* cmdrecv_hold_position(s8* buffer);																			// 004C20C0
	u32 unit_can_see_cloaked_target(Unit* target, Unit* unit);															// 00401D60
	u32 is_unit_visible(Unit* unit);																					// 004E5DB0
	void check_unit_visibility(Unit* unit);																				// 00443390
	s32 is_unit_cloaking(Unit* unit);																					// 004916E0
	void update_unit_visibility(Unit* unit);																			// Extracted part of check_unit_visibility
	bool unit_can_attack_target(Unit* target, Unit* unit, int check_detection);											// 00476730
	void ai_attack_unit(Unit* unit);																					// 0043FFD0
	s32 return_interceptor_if_cannot_attack(Unit* hangar_unit);															// 00465810
	Thingy* find_fow_target(u32 x_pos, u32 y_pos);																		// 00487A10
	bool is_better_target(Unit* self, Unit* auto_target, Unit* compare_unit);											// 004769D0
	void do_targeted_order(void* unused, u32 sound_offset, u32 pos_x, u32 pos_y, Unit* target_arg, u32 fow_unit_id);	// 0046F5B0
	b32 fow_refinery_check(Unit* self, u16 fow_id);																		// 00474500
	void pick_new_target_if_needed(Unit* unit);																			// 00477510
	void change_direction_to_move_waypoint(Unit* unit);																	// 00495840
	void orders_turret_guard(Unit* unit);																				// 004777F0
	void get_closest_point_of_target(Unit* unit, s32* out_x, s32* out_y);												// 004762C0
	Unit* unit_order_move_to_target_reset_order_state(Unit* unit, u8 order_id, Unit* target);							// 00479FA0
	b32 is_target_within_min_range(Unit* unit, u32 min_range, Unit* target);											// 00430F10
	u32 is_target_visible(Unit* target, Unit* unit);																	// 004E5E30
	b32 is_unit_enemy(Unit* self, Unit* target);																		// 0047B910
	b32 is_ready_to_attack(Unit* unit, u8 weapon_id);																	// 00476640
	Unit* ai_choose_target(Unit** target_array, u32 player, Unit* unit, u32 region_id, b32 is_ground);						// 00431990
	u8 get_air_weapon(Unit* unit);																						// 00475840

	void inject_target_related_hooks();
}