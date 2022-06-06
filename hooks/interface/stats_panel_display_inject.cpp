#include "stats_panel_display.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) stats_panel_display_wrapper()
	{
		static BinDlg* dialog;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV dialog, EAX
			PUSHAD
		}
		hooks::stats_panel_display(dialog);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) draw_building_info_wrapper()
	{
		static BinDlg* dialog;
		__asm {
			MOV dialog, ECX
			PUSHAD
		}
		hooks::draw_building_info(dialog);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) get_unit_rank_string_wrapper() // 0042F370
	{
		static Unit* a1;
		static char* result;
		__asm {
			MOV a1, ESI
			PUSHAD
		}
		result = hooks::get_unit_rank_string(a1);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) draw_unit_kills_string_wrapper() // 00425DD0
	{
		static BinDlg* dialog;
		__asm {
			MOV dialog, EBX
			PUSHAD
		}
		hooks::draw_unit_kills_string(dialog);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) stats_armor_level_wrapper() // 00425600
	{
		static BinDlg* dialog;
		static s32 unused;
		static u16 buttonRelated;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV buttonRelated, AX
			POP EAX
			MOV unused, EDX
			MOV dialog, EAX
			PUSHAD
		}
		hooks::stats_armor_level(dialog, unused, buttonRelated);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) stats_weapon_level_wrapper() // 00425790
	{
		static BinDlg* dialog;
		static u16 buttonRelated;
		static u8 weaponId;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0xC]
			MOV weaponId, AL
			MOV EAX, [EBP + 0x08]
			MOV buttonRelated, AX
			POP EAX
			MOV dialog, EAX
			PUSHAD
		}
		hooks::stats_weapon_level(dialog, buttonRelated, weaponId);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) get_unit_name_wrapper() // 0047B5A0
	{
		static const char* result;
		static Unit* unit;
		__asm {
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::get_unit_name(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) set_tooltip_info_unit_case1_wrapper() { // 004594B0
		static const u32 jmp_address = 0x00459635;
		static u32 unit_id;
		__asm {
			MOV unit_id, EDI
			PUSHAD
		}
		hooks::set_tooltip_info_unit(unit_id);
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	void __declspec(naked) set_tooltip_info_unit_case5_wrapper() { // 0045952F
		static const u32 jmp_address = 0x00459635;
		static u32 unit_id;
		__asm {
			MOV unit_id, EAX
			PUSHAD
		}
		hooks::set_tooltip_info_unit(unit_id);
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	void __declspec(naked) reset_tooltip_info_wrapper_1() { // 00459360
		__asm {
			PUSHAD
		}
		hooks::reset_tooltip_info();
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) reset_tooltip_info_wrapper_2() { // 004593B0
		static const u32 jmp_address = 0x004593ED;
		__asm {
			PUSHAD
		}
		hooks::reset_tooltip_info();
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	void __declspec(naked) reset_tooltip_info_wrapper_3() { // 004597D5
		static const u32 jmp_address = 0x00459819;
		__asm {
			PUSHAD
		}
		hooks::reset_tooltip_info();
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	void __declspec(naked) set_tooltip_info_wrapper() { // 00458800
		static u16 mineral_cost;
		static u16 gas_cost;
		static u16 p_energy;
		static u16 t_energy;
		static u16 z_energy;
		static u16 p_supply;
		static u16 t_supply;
		static u16 z_supply;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV t_energy, AX
			MOV EAX, [EBP + 0x0C]
			MOV z_energy, AX
			MOV EAX, [EBP + 0x10]
			MOV p_supply, AX
			MOV EAX, [EBP + 0x14]
			MOV t_supply, AX
			MOV EAX, [EBP + 0x18]
			MOV z_supply, AX
			POP EAX
			MOV mineral_cost, AX
			MOV gas_cost, CX
			MOV p_energy, DX
			PUSHAD
		}
		hooks::set_tooltip_info(mineral_cost, gas_cost, p_energy, t_energy, z_energy, p_supply, t_supply, z_supply);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x14
		}
	}

	void __declspec(naked) draw_tooltip_with_icon_wrapper() { // 00458EF0
		static Box16* area;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV area, EAX
			POP EAX
			PUSHAD
		}
		hooks::draw_tooltip_with_icon(area);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	static void draw_tooltip_with_icon_partial(u32 index, char* buffer, u32 value) {
		if (index == 5) {
			sprintf_s(buffer, 16, "%ds", value);
		} else {
			sprintf_s(buffer, 16, "%d", value);
		}
	}

	void __declspec(naked) draw_tooltip_with_icon_partial_wrapper() { // 00458FB2
		static const u32 jmp_address = 0x00458FC5;
		static u32 index;
		static char* buffer;
		static u32 value;
		__asm {
			MOV index, ESI
			PUSH EAX
			LEA EAX, [EBP-0x2C] // This is wrong
			MOV buffer, EAX
			POP EAX
			MOV value, EDX
			PUSHAD
		}
		draw_tooltip_with_icon_partial(index, buffer, value);
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	void __declspec(naked) stat_data_unit_wireframe_selection_wrapper() { // 004583E0
		static u32 result;
		static BinDlg* dialog;
		static Event* event_arg;
		__asm {
			MOV event_arg, EDX
			MOV dialog, ECX
			PUSHAD
		}
		result = hooks::stat_data_unit_wireframe_selection(dialog, event_arg);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) get_last_queue_slot_id_wrapper() { // 0047B270
		static u16 result;
		static Unit* unit;
		__asm {
			MOV unit, EDX
			PUSHAD
		}
		result = hooks::get_last_queue_slot_id(unit);
		__asm {
			POPAD
			MOV AX, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_stats_panel_display_hooks()
	{
		jump_patch(stats_panel_display_wrapper,						0x00426C60, 1);
		jump_patch(get_unit_rank_string_wrapper,					0x0042F370, 2);
		jump_patch(draw_unit_kills_string_wrapper,					0x00425DD0, 2);
	//	jump_patch(draw_building_info_wrapper,						0x00427890, 3);
	//	jump_patch(stats_armor_level_wrapper,						0x00425600, 1);
	//	jump_patch(stats_weapon_level_wrapper,						0x00425790, 1);
		jump_patch(get_unit_name_wrapper,							0x0047B5A0, 2);
		jump_patch(set_tooltip_info_unit_case1_wrapper,				0x004594B0, 2);
		jump_patch(set_tooltip_info_unit_case5_wrapper,				0x0045952F, 4);
		jump_patch(reset_tooltip_info_wrapper_1,					0x00459360, 0);
		jump_patch(reset_tooltip_info_wrapper_2,					0x004593B0, 0);
		jump_patch(reset_tooltip_info_wrapper_3,					0x004597D5, 0);
		jump_patch(set_tooltip_info_wrapper,						0x00458800, 4);
	//	jump_patch(draw_tooltip_with_icon_wrapper,					0x00458EF0, 1);
	//	jump_patch(stat_data_unit_wireframe_selection_wrapper,		0x004583E0, 3);
		jump_patch(get_last_queue_slot_id_wrapper,					0x0047B270, 2);
		
	//	call_patch(draw_tooltip_with_icon_partial_wrapper,          0x00458FB2, 1);
	}
}