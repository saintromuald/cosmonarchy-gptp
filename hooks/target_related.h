#pragma once
#include "SCBW/api.h"
#include "SCBW/scbwdata.h"
namespace hooks
{
	//	s32 __fastcall setUnderDisruptionWeb(s32 a1);		
	//	s32 setUnderDisruptionWeb(Unit* a1);
	//	s32 update_disruption_web_status();

	s32 generic_right_click_unit_proc_2(Unit* unit, s32 a2, s16 a3, Unit* target, s16 unitID);		// 00455E00
	Unit* cmdrecv_hold_position(s32 a1);															// 004C20C0
	u32 unit_can_see_cloaked_target(Unit* target, Unit* unit);										// 00401D60
	u32 is_target_visible(Unit* target, Unit* unit);												// 004E5E30
	u32 is_unit_visible(Unit* unit);																// 004E5DB0
	void check_unit_visibility(Unit* unit);															// 00443390
	s32 is_unit_cloaking(Unit* unit);																// 004916E0
	void update_unit_visibility(Unit* unit);														// Extracted part of check_unit_visibility
	bool unit_can_attack_target(Unit* target, Unit* unit, int check_detection);						// 00476730
	s32 order_attack_behavior(Unit *a1);															// 00478540
	void ai_attack_unit(Unit* unit);																// 0043FFD0
	s32 return_interceptor_if_cannot_attack(Unit* hangar_unit);										// 00465810
	Thingy* find_fow_target(u32 x_pos, u32 y_pos);													// 00487A10

	void inject_target_related_hooks();
}