#pragma once
#include "BW/api.h"
namespace hooks {
	u32 get_max_hp(s32 id, s32 player);																			// 0040140B
	u32 get_max_shields(s32 id, s32 player);																	// internal

	u32 get_max_vitality(Unit* unit);																			// 00402710
	void attack_overlay_and_notify(Unit* victim, Unit* attacker, u8 weapon_id, u8 direction);					// 00479730
	b32 ai_target_enemy_proc(Unit* target, Unit* unit);															// 00440A20
	u32 calculate_unit_strength(u32 unit_id, u32 weapon_id);													// 00431270
	u32 get_unit_strength(Unit* unit, s32 unused, bool is_ground);												// 00431800
	b32 ai_restoration_requirements_proc(Unit* caster, Unit* target);											// 004407E0
	b32 ai_emp_shield_requirements_proc(Unit* target, Unit* caster);											// 004413D0
	void update_building_construct_shields_amount(Unit* unit);													// 0047B6A0
	u32 get_unit_bullet_damage(Unit* victim, Bullet* bullet);													// 0048ACD0
	b32 __stdcall modify_unit_shields(Unit* unit, s32 new_amount);												// 004C5A20
	b32 ai_find_best_transport(Unit* unit, Unit* result_unit);													// 004E6EF0

	// Needs hooking:
	void __stdcall stats_panel_draw_health_shields(BinDlg* dialog);												// 004263E0
	void draw_unit_info(BinDlg* a1);																			// 00426190
	void draw_image_hp_bar(s32 g_screen_x, s32 g_screen_y, GrpFrame* pFrame, Box32* grpRect, s32 colorData);	// 0047A820
	s32 map_data_transfer(char a1, s32 a2, s32 a3);																// 004EAFE0

	void inject_max_hp_hooks();
}