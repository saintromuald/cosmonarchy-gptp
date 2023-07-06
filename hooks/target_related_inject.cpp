#include "target_related.h"
#include "hook_tools.h"
#include "SCBW/structures.h"

namespace
{
	void __declspec(naked) is_unit_in_weapon_range_wrapper() { // 00476870
		static Unit* target;
		static Unit* attacker;
		static bool result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV attacker, EAX
			POP EAX
			MOV target, EAX
			PUSHAD
		}
		result = hooks::is_unit_in_weapon_range(target, attacker);
		__asm {
			POPAD
			MOVZX EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) generic_right_click_unit_proc_2_wrapper() // 00455E00
	{
		static s32 result;
		static Unit* unit;
		static s32 a2;
		static s16 a3;
		static Unit* target;
		static s16 unitID;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV a3, AX
			MOV EAX, [EBP + 0xC]
			MOV target, EAX
			MOV EAX, [EBP + 0x10]
			MOV unitID, AX
			POP EAX
			MOV a2, EDX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::generic_right_click_unit_proc_2(unit, a2, a3, target, unitID);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0xC
		}
	}

	void __declspec(naked) cmdrecv_hold_position_wrapper() // 004C20C0
	{
		static Unit* result;
		static s32 a1;
		__asm
		{
			MOV a1, EDI
			PUSHAD
		}
		result = hooks::cmdrecv_hold_position(a1);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unit_can_see_cloaked_target_wrapper() // 00401D60
	{
		static Unit* target;
		static Unit* unit;
		static u32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			MOV target, EAX
			PUSHAD
		}
		result = hooks::unit_can_see_cloaked_target(target, unit);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) is_target_visible_wrapper() // 004E5E30
	{
		static u32 result;
		static Unit* target;
		static Unit* unit;
		__asm
		{
			MOV target, EAX
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::is_target_visible(target, unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) is_unit_visible_wrapper() // 004E5DB0
	{
		static u32 result;
		static Unit* unit;
		__asm
		{
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::is_unit_visible(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) check_unit_visibility_wrapper() // 00443390
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::check_unit_visibility(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) is_unit_cloaking_wrapper() // 004916E0
	{
		static s32 result;
		static Unit* unit;
		__asm
		{
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::is_unit_cloaking(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unit_can_attack_target_wrapper() // 00476730
	{
		static u32 result;
		static Unit* target;
		static Unit* attacker;
		static u32 check_detection;
		__asm
		{
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV check_detection, EAX
			POP EAX
			MOV target, EBX
			MOV attacker, ESI
			POP EBP
			PUSHAD
		}
		result = hooks::unit_can_attack_target(target, attacker, check_detection);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN 0x04
		}
	}

	void __declspec(naked) ai_attack_unit_wrapper() // 0043FFD0
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			PUSHAD
		}
		hooks::ai_attack_unit(unit);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) return_interceptor_if_cannot_attack_wrapper() { // 00465810
		static Unit* hangar_unit;
		static s32 result;
		__asm {
			MOV hangar_unit, EAX
			PUSHAD
		}
		result = hooks::return_interceptor_if_cannot_attack(hangar_unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) find_fow_target_wrapper() { // 00487A10
		static Thingy* result;
		static u16 x_pos;
		static u16 y_pos;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV y_pos, AX
			POP EAX
			MOV x_pos, BX
			PUSHAD
		}
		result = hooks::find_fow_target(x_pos, y_pos);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) is_better_target_wrapper() { // 004769D0
		static bool result;
		static Unit* self;
		static Unit* auto_target;
		static Unit* compare_unit;
		__asm {
			MOV compare_unit, EDI
			MOV auto_target, EAX
			MOV self, ECX
			PUSHAD
		}
		result = hooks::is_better_target(self, auto_target, compare_unit);
		__asm {
			POPAD
			MOVZX AL, result
			RETN
		}
	}

	void __declspec(naked) do_targeted_order_wrapper() { // 0046F5B0
		static void* unused;
		static u32 sound_offset;
		static u32 pos_x;
		static u32 pos_y;
		static Unit* target;
		static s32 fow_unit_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV pos_x, EAX
			MOV EAX, [EBP + 0x0C]
			MOV pos_y, EAX
			MOV EAX, [EBP + 0x10]
			MOV target, EAX
			MOV EAX, [EBP + 0x14]
			MOV fow_unit_id, EAX
			POP EAX
			MOV sound_offset, ESI
			MOV unused, ECX
			PUSHAD
		}
		hooks::do_targeted_order(unused, sound_offset, pos_x, pos_y, target, fow_unit_id);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) fow_refinery_check_wrapper() { // 004769D0
		static b32 result;
		static Unit* self;
		static u16 fow_id;
		__asm {
			MOV fow_id, CX
			MOV self, EAX
			PUSHAD
		}
		result = hooks::fow_refinery_check(self, fow_id);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) pick_new_target_if_needed_wrapper() { // 00477510
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			PUSHAD
		}
		hooks::pick_new_target_if_needed(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) change_direction_to_move_waypoint_wrapper() { // 00495840
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		hooks::change_direction_to_move_waypoint(unit);
		__asm {
			POPAD
			RETN
		}
	}

	// Patches \\

	const u32 GetClosestReachable_2F155 = 0x0042F155;
	const u32 GetClosestReachable_2F281 = 0x0042F281;
	void __declspec(naked) get_closest_reachable_patch_2F147() {
		__asm {
			MOV		EDI, [ESI + 0xC]
			TEST	EDI, EDI	// attacker_unit != NULL
			JE		offset loc_2F281
			MOV		EAX, [EDI + 0xC]
			TEST	EAX, EAX	// attacker_unit->sprite != NULL
			JE		offset loc_2F281
			JMP		GetClosestReachable_2F155

			loc_2F281 :
			JMP		GetClosestReachable_2F281
		}
	}

	const u32 MFixCollision_6C002 = 0x0046C002;
	const u32 MFixCollision_6C01D = 0x0046C01D;
	void __declspec(naked) fix_collision_patch_6BFFD() {
		__asm {
			MOV		EAX, [EDI + 0x14]
			TEST	EAX, EAX	// unit->move_target.unit != NULL
			JE		offset loc_6C01D
			MOV		ECX, [EAX + 0xC]
			TEST	ECX, ECX	// unit->move_target.unit->sprite != NULL
			JE		offset loc_6C01D
			JMP		MFixCollision_6C002

			loc_6C01D :
			JMP		MFixCollision_6C01D
		}
	}
}

namespace hooks
{
	void inject_target_hooks()
	{
	// Removes can not attack status checks from some update functions, it was done by iquare - not exactly sure what was the intention.
	// Currently it doesn't seem to do anything. -Veeq7
	//	nops(0x004EB176, 8);
	//	nops(0x004EB316, 2);

		jump_patch(is_unit_in_weapon_range_wrapper,					0x00476870, 2);
	//	jump_patch(generic_right_click_unit_proc_2_wrapper,			0x00455E00, 1);
	//	jump_patch(cmdrecv_hold_position_wrapper,					0x004C20C0, 0);
	//	jump_patch(unit_can_see_cloaked_target_wrapper,				0x00401D60, 4);
	//	jump_patch(is_target_visible_wrapper,						0x004E5E30, 4);
	//	jump_patch(is_unit_visible_wrapper,							0x004E5DB0, 1);
		jump_patch(check_unit_visibility_wrapper,					0x00443390, 3);
	//	jump_patch(is_unit_cloaking_wrapper,						0x004916E0, 0);
		jump_patch(unit_can_attack_target_wrapper,					0x00476730, 5);
		jump_patch(ai_attack_unit_wrapper,							0x0043FFD0, 1);
		jump_patch(return_interceptor_if_cannot_attack_wrapper,		0x00465810, 1);
		jump_patch(find_fow_target_wrapper,							0x00487A10, 5);
		jump_patch(is_better_target_wrapper,						0x004769D0, 0);
	//	jump_patch(do_targeted_order_wrapper,						0x0046F5B0, 1); // Crashes
		jump_patch(fow_refinery_check_wrapper,						0x00474500, 5);
		jump_patch(pick_new_target_if_needed_wrapper,				0x00477510, 1);
		jump_patch(change_direction_to_move_waypoint_wrapper,		0x00495840, 3);

		// Patches \\

		// Fix dying Lakizilisks updating vision while their spines are still in play. -DarkenedFantasies
		jump_patch(get_closest_reachable_patch_2F147, 0x0042F147);
		jump_patch(fix_collision_patch_6BFFD, 0x0046BFFD);
	}

	void inject_target_related_hooks()
	{
		inject_target_hooks();
	}
}