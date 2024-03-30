#include "selection_extender.h"
#include "hooks/limits/ccmu.h"
#include "hook_tools.h"
#include "BW/structures.h"
#include "BW/bwdata.h"
#include "BW/api.h"
#include "config.h"
#include "trigger_system/trigger_system.h"

//C++
//Commented patches are intended for functions that have been hooked in CMBW; edit hooks to reflect changes.

static u32 tmp;

graphics::Font* grp_wire_Font;
extern u8 tFontGame_ImageData[];
void init_grp_wire_font(const char* font_name) {
	const u32 load_font = 0x0041FD10;

	__asm {
		PUSH	0x004DAB00//AllocateFont
		MOV		EAX, font_name
		CALL	load_font
		MOV		grp_wire_Font, EAX
	}
}

const u32 BTNSCOND_TrainingFighter_28E52 = 0x00428E52;
const u32 create_unit_allowed = 0x0046E1C0;
void __declspec(naked) BTNSCOND_TrainingFighter_patch_28E20() {
	__asm {
		loc_428E20:
		MOV		ESI, [EDI + ccmu_extender.new_client_selection_group]
		TEST	ESI, ESI
		JE		offset loc_428E41
		MOV		DX, [EBP + 8]
		CMP		[ESI + 0x64], DX
		JNE		offset loc_428E52
		TEST	EAX, EAX
		JNE		offset loc_428E41
		MOV		EAX, [EBP - 4]
		PUSH	EBX
		CALL	create_unit_allowed

		loc_428E41:
		ADD		EDI, 4
		CMP		EDI, SelectionLimit_dword
		JB		offset loc_428E20
		POP		EDI
		POP		ESI
		POP		EBX
		MOV		ESP, EBP
		POP		EBP
		RETN	4

		loc_428E52:
		JMP		BTNSCOND_TrainingFighter_28E52
	}
}

/*const u32 does_accept_right_click_commands = 0x004556D0;
const u32 unit_can_move = 0x00455660;
const u32 CMDACT_RightClick_56425 = 0x00456425;
void __declspec(naked) CMDACT_RightClick_patch_563D0() {
	__asm {
		loc_4563D0:
		MOV		EAX, [ESI * 4 + ccmu_extender.new_client_selection_group]
		TEST	EAX, EAX
		JE		offset loc_4563E4
		CALL	does_accept_right_click_commands
		TEST	EAX, EAX
		JNE		offset loc_4563F0

		loc_4563E4:
		INC		ESI
		CMP		ESI, SelectionLimit
		JL		offset loc_4563D0
		POP		ESI
		POP		EDI
		MOV		ESP, EBP
		POP		EBP
		RET

		loc_4563F0:
		CMP		ESI, SelectionLimit
		JNL		offset loc_456488
		XOR		ESI, ESI
		ALIGN 4

		loc_456400:
		MOV		EAX, [ESI * 4 + ccmu_extender.new_client_selection_group]
		TEST	EAX, EAX
		JE		offset loc_456414
		CALL	unit_can_move
		TEST	EAX, EAX
		JNE		offset loc_456420

		loc_456414:
		INC		ESI
		CMP		ESI, SelectionLimit
		JL		offset loc_456400

		loc_456488:
		POP		ESI
		POP		EDI
		MOV		ESP, EBP
		POP		EBP
		RET

		loc_456420:
		CMP		ESI, SelectionLimit
		JNL		offset loc_456488
		JMP		CMDACT_RightClick_56425
	}
}

const u32 input_game_right_mouse_click_5656A = 0x0045656A;
void __declspec(naked) input_game_right_mouse_click_patch_56540() {
	__asm {
		loc_456540:
		MOV		EAX, [ESI * 4 + ccmu_extender.new_client_selection_group]
		TEST	EAX, EAX
		JE		offset loc_456554
		CALL	does_accept_right_click_commands
		TEST	EAX, EAX
		JNE		offset loc_456561

		loc_456554:
		INC		ESI
		CMP		ESI, SelectionLimit
		JL		offset loc_456540

		loc_456625:
		POP		ESI
		POP		EBX
		POP		EDI
		MOV		ESP, EBP
		POP		EBP
		RET

		loc_456561:
		CMP		ESI, SelectionLimit
		JNL		offset loc_456625
		JMP		input_game_right_mouse_click_5656A
	}
}

const u32 input_game_right_mouse_click_565E6 = 0x004565E6;
void __declspec(naked) input_game_right_mouse_click_patch_565C0() {
	__asm {
		loc_4565C0:
		MOV		EAX, [EBX * 4 + ccmu_extender.new_client_selection_group]
		TEST	EAX, EAX
		JE		offset loc_4565D4
		CALL	unit_can_move
		TEST	EAX, EAX
		JNE		offset loc_4565E1

		loc_4565D4:
		INC		EBX
		CMP		EBX, SelectionLimit
		JL		offset loc_4565C0

		loc_456625:
		POP		ESI
		POP		EBX
		POP		EDI
		MOV		ESP, EBP
		POP		EBP
		RET

		loc_4565E1:
		CMP		EBX, SelectionLimit
		JGE		offset loc_456625
		JMP		input_game_right_mouse_click_565E6
	}
}*/

const u32 SetFontHandle = 0x0041FB30;
const u32 bw_print_xy = 0x004202B0;
const u32 stat_data_unit_wireframe_select_update_5704F = 0x0045704F;
void __declspec(naked) stat_data_unit_wireframe_select_update_patch_57046() {
	__asm {
		MOV		EAX, [EDI + 0x14]//dialog->pszText
		TEST	EAX, EAX
		JE		loc_457046
		MOV		ECX, grp_wire_Font
		CALL	SetFontHandle

		LEA		EAX, [tFontGame_ImageData + 0x140]//Special grp_wire_Font color.
		MOV		ds:[0x006CE0C0], EAX//currentFontColor
		MOV		byte ptr ds:[0x006CE0C4], 0//Set font spacing to 0.

		MOV		EAX, [EDI + 4]//dialog->bounds.left
		MOVSX	EDX, word ptr [EDI + 6]//dialog->bounds.top
		MOV		ECX, [EDI + 8]//dialog->bounds.right; dialog->bounds.bottom
		MOVSX	ESI, AX
		MOV		dword ptr ds:[0x006CE0C8], EAX//drawTextLeft; drawTextTop
		MOV		EAX, [EDI + 0x14]//dialog->pszText
		PUSH	EDX
		MOV		byte ptr ds:[0x006CE110], 0x44//printXYAlignment
		MOV		dword ptr ds:[0x006CE0CC], ECX//drawTextRight; drawTextBottom
		CALL	bw_print_xy
		XOR		ECX, ECX
		CALL	SetFontHandle

		loc_457046:
		POP		EDI
		POP		ESI
		MOV		byte ptr ds:[0x0050CE91], 0xD0
		JMP		stat_data_unit_wireframe_select_update_5704F
	}
}

const u32 statdataMouseOverInteract_57D93 = 0x00457D93;
void __declspec(naked) statdataMouseOverInteract_patch_57046() {
	__asm {
		MOVZX	EAX, byte ptr g_config.always_show_condensed_selection
		TEST	EAX, EAX
		JNE		offset isMultiSelect
		MOV		EAX, GroupWireframesCap
		CMP		byte ptr ds : [0x0059723D], AL//clientSelectionCount
		JBE		offset isSingleSelect

		isMultiSelect:
		CMP		word ptr[ECX], 1765
		JBE		offset stringDoesNotExist
		MOVZX	EAX, word ptr[ECX + 2 * 1766]
		ADD		EAX, ECX
		JMP		statdataMouseOverInteract_57D93

		isSingleSelect:
		CMP		word ptr[ECX], 808
		JBE		offset stringDoesNotExist
		MOVZX	EAX, word ptr[ECX + 2 * 809]
		ADD		EAX, ECX
		JMP		statdataMouseOverInteract_57D93

		stringDoesNotExist:
		MOV		EAX, 0x00501B7D//"\x00"
		JMP		statdataMouseOverInteract_57D93
	}
}

/*
const u32 select_dual_birth_units_5D079 = 0x0045D079;
void __declspec(naked) select_dual_birth_units_patch_5D041() {
	__asm {
		MOV		EBP, ESP
		SUB		ESP, SelectionLimit_dword
		PUSH	ESI
		PUSH	EDI
		MOV		ECX, SelectionLimit
		MOV		ESI, offset ccmu_extender.new_active_selection[0]
		LEA		EDI, [EBP - SelectionLimit_dword]
		REPE MOVSD
		XOR		EDI, EDI
		XOR		EAX, EAX
		ALIGN 4

		loc_45D060:
		CMP		[EBP + EAX * 4 - SelectionLimit_dword], EDI
		JE		offset loc_45D074
		INC		EAX
		CMP		EAX, SelectionLimit
		JL		offset loc_45D060

		loc_45D06C:
		POP		EDI
		POP		ESI
		MOV		ESP, EBP
		POP		EBP
		RETN	4

		loc_45D074:
		CMP		EAX, SelectionLimit
		JNL		offset loc_45D06C
		JMP		select_dual_birth_units_5D079
	}
}

const u32 sub_46F040 = 0x0046F040;
const u32 sort_all_units_6F226 = 0x0046F226;
void __declspec(naked) sort_all_units_patch_6F206() {
	__asm {
		CMP		EAX, SelectionLimit
		JL		offset loc_46F217
		PUSH	ECX
		MOV		ECX, [EBP + 0xC]
		PUSH	ESI
		CALL	sub_46F040
		JMP		sort_all_units_6F226

		loc_46F217:
		MOV		ECX, [EBP + 0xC]
		MOV		[ECX + EAX * 4], ESI
		INC		EAX
		MOV		[EBP - 4], EAX
		JMP		sort_all_units_6F226
	}
}*/

const u32 combine_lists_6F2BF = 0x0046F2BF;
void __declspec(naked) combine_lists_patch_6F2AD() {
	__asm {
		CMP		ESI, SelectionLimit
		JL		offset loc_46F2BF
		POP		ESI
		MOV		EAX, SelectionLimit
		POP		EBX
		MOV		ESP, EBP
		POP		EBP
		RETN	0xC

		loc_46F2BF:
		JMP		combine_lists_6F2BF
	}
}

const u32 sub_46F040 = 0x0046F040;
void __declspec(naked) combine_lists_patch_6F320() {
	__asm {
		loc_46F320:
		TEST	ESI, ESI
		MOV		EBX, [EBP + 8]
		MOV		EAX, ESI
		JE		offset loc_46F33A
		MOV		ECX, [EBX]
		ALIGN 4

		loc_46F330:
		DEC		EAX
		CMP		[EDI + EAX * 4], ECX
		JE		offset loc_46F366
		TEST	EAX, EAX
		JNE		offset loc_46F330

		loc_46F33A:
		CMP		EDX,  SelectionLimit
		JL		offset loc_46F356
		MOV		EAX, [EBP + 0x10]
		MOV		ECX, [EBX]
		PUSH	EAX
		MOV		EAX, [EBP + 0xC]
		PUSH	ECX
		LEA		ECX, [EDI + ESI * 4]
		CALL	sub_46F040
		MOV		EDX, [EBP - 4]
		JMP		offset loc_46F366

		loc_46F356:
		MOV		EAX, [EBX]
		MOV		[EDI + EDX * 4], EAX
		MOV		EAX, [EBP + 0xC]
		INC		EDX
		INC		EAX
		MOV		[EBP - 4], EDX
		MOV		[EBP + 0xC], EAX

		loc_46F366:
		MOV		EAX, [EBP - 8]
		ADD		EBX, 4
		DEC		EAX
		MOV		[EBP + 8], EBX
		MOV		[EBP - 8], EAX
		JNE		offset loc_46F320
		POP		ESI
		MOV		EAX, EDX
		POP		EBX
		MOV		ESP, EBP
		POP		EBP
		RETN	0xC
	}
}

/*const u32 find_all_units = 0x004308A0;
const u32 get_selected_units_in_box_6FA6A = 0x0046FA6A;
void __declspec(naked) get_selected_units_in_box_patch_6FA41() {
	__asm {
		MOV		EBP, ESP
		SUB		ESP, SelectionLimit_dword_2
		PUSH	EBX
		PUSH	ESI
		MOVZX	ESI, byte ptr ds:[0x00596A28]//IS_KEYCODE_USED + 10
		PUSH	EDI
		XOR		EAX, EAX
		MOV		ECX, SelectionLimit
		LEA		EDI, [EBP - SelectionLimit_dword]//this
		REPE STOSD
		MOV		EAX, [EBP + 8]//coords
		PUSH	EAX
		CALL	find_all_units
		PUSH	0
		LEA		ECX, [EBP - SelectionLimit_dword]//this
		JMP		get_selected_units_in_box_6FA6A
	}
}

const u32 get_selected_units_in_box_6FAB9 = 0x0046FAB9;
void __declspec(naked) get_selected_units_in_box_patch_6FAAC() {
	__asm {
		LEA		EDI, [EBP - SelectionLimit_dword_2]//buffer
		PUSH	EBX
		LEA		EAX, [EBP - SelectionLimit_dword]//this
		REPE MOVSD
		PUSH	EAX
		LEA		EDI, [EBP - SelectionLimit_dword_2]//buffer
		JMP		get_selected_units_in_box_6FAB9
	}
}

const u32 unit_to_unit_id = 0x0047B1D0;
const u32 select_single_unit_from_id = 0x00496D30;
const u32 select_multiple_units_from_unit_list = 0x0049AEF0;
const u32 get_selected_units_in_box_6FB0A = 0x0046FB0A;
void __declspec(naked) get_selected_units_in_box_patch_6FAD4() {
	__asm {
		CMP		EBX, 1
		JNE		offset loc_46FAF4
		MOV		AL, ds:[0x00596A2A]//IS_KEYCODE_USED + 12
		TEST	AL, AL
		JE		offset loc_46FAF4
		MOV		ESI, [EBP - SelectionLimit_dword]
		CALL	unit_to_unit_id
		PUSH	EAX
		CALL	select_single_unit_from_id
		TEST	EAX, EAX
		JNE		offset loc_46FB0A

		loc_46FAF4:
		PUSH	1
		PUSH	1
		LEA		ECX, [EBP - SelectionLimit_dword]
		PUSH	ECX
		PUSH	EBX
		CALL	select_multiple_units_from_unit_list
		CMP		EBX, 1
		JNE		offset loc_46FB2C
		MOV		ESI, [EBP - SelectionLimit_dword]

		loc_46FB0A:
		JMP		get_selected_units_in_box_6FB0A

		loc_46FB2C:
		POP		EDI
		POP		ESI
		POP		EBX
		MOV		ESP, EBP
		POP		EBP
		RETN	4
	}
}

const u32 get_selected_units_at_point_6FB46 = 0x0046FB46;
void __declspec(naked) get_selected_units_at_point_patch_6FB41() {
	__asm {
		MOV		EBP, ESP
		SUB		ESP, SelectionLimit_dword_2 + 12
		JMP		get_selected_units_at_point_6FB46
	}
}

const u32 get_selected_units_at_point_6FB9D = 0x0046FB9D;
void __declspec(naked) get_selected_units_at_point_patch_6FB98() {
	__asm {
		LEA		EDI, [EBP - newDestUnit]
		REPE STOSD
		JMP		get_selected_units_at_point_6FB9D
	}
}

const u32 get_all_units_in_bounds = 0x0042FF80;
const u32 get_selected_units_at_point_6FCAB = 0x0046FCAB;
void __declspec(naked) get_selected_units_at_point_patch_6FC9A() {
	__asm {
		LEA		EDI, [EBP - newStackChar]
		LEA		EAX, [EBP - 0xC]
		REPE MOVSD
		CALL	get_all_units_in_bounds
		PUSH	EBX
		LEA		ECX, [EBP - newDestUnit]
		JMP		get_selected_units_at_point_6FCAB
	}
}

const u32 get_selected_units_at_point_6FCDD = 0x0046FCDD;
void __declspec(naked) get_selected_units_at_point_patch_6FCD6() {
	__asm {
		LEA		EAX, [EBP - newDestUnit]
		PUSH	EAX
		LEA		EDI, [EBP - newStackChar]
		JMP		get_selected_units_at_point_6FCDD
	}
}

const u32 get_selected_units_at_point_6FD13 = 0x0046FD13;
void __declspec(naked) get_selected_units_at_point_patch_6FD00() {
	__asm {
		loc_46FD00:
		MOV		EAX, [ECX]
		TEST	EAX, EAX
		JE		offset loc_46FD13
		MOV		[EBP + EDI * 4 - newDestUnit], EAX
		INC		EDI
		ADD		ECX, 4
		CMP		EDI, SelectionLimit
		JL		offset loc_46FD00

		loc_46FD13:
		JMP		get_selected_units_at_point_6FD13
	}
}

const u32 get_selected_units_at_point_6FD27 = 0x0046FD27;
const u32 get_selected_units_at_point_6FE95 = 0x0046FE95;
void __declspec(naked) get_selected_units_at_point_patch_6FD1B() {
	__asm {
		CMP		EDI, SelectionLimit
		JNL		offset loc_46FE95
		MOV		ECX, [EBP - newDestUnit]
		JMP		get_selected_units_at_point_6FD27

		loc_46FE95:
		JMP		get_selected_units_at_point_6FE95
	}
}

const u32 get_selected_units_at_point_6FD6B = 0x0046FD6B;
void __declspec(naked) get_selected_units_at_point_patch_6FD5F() {
	__asm {
		MOV		[EBP + EDI * 4 - newDestUnit], EBX
		PUSH	1
		PUSH	1
		INC		EDI
		LEA		EBX, [EBP - newDestUnit]
		JMP		get_selected_units_at_point_6FD6B
	}
}

const u32 get_selected_units_at_point_6FD8D = 0x0046FD8D;
void __declspec(naked) get_selected_units_at_point_patch_6FD84() {
	__asm {
		LEA		EDX, [EBP + EAX * 4 - newSrc]
		PUSH	EDX
		LEA		EAX, [EBP + EAX * 4 - newDestUnit]
		JMP		get_selected_units_at_point_6FD8D
	}
}

const u32 get_selected_units_at_point_6FDAD = 0x0046FDAD;
const u32 get_selected_units_at_point_6FDC2 = 0x0046FDC2;
void __declspec(naked) get_selected_units_at_point_patch_6FD9B() {
	__asm {
		MOV		[EBP + EDI * 4 - newDestUnit], ESI
		JNE		offset loc_46FDC2
		MOV		AL, ds:[0x00596A2A]//IS_KEYCODE_USED + 12
		TEST	AL, AL
		JE		offset loc_46FDC2
		MOV		ESI, [EBP - newDestUnit]
		JMP		get_selected_units_at_point_6FDAD

		loc_46FDC2:
		JMP		get_selected_units_at_point_6FDC2
	}
}

*/const u32 create_new_unit_selections_from_list = 0x0049AE40;/*
const u32 get_selected_units_at_point_6FDCE = 0x0046FDCE;
void __declspec(naked) get_selected_units_at_point_patch_6FDC3() {
	__asm {
		LEA		EAX, [EBP - newDestUnit]
		CALL	create_new_unit_selections_from_list
		LEA		ECX, [EBP - newDestUnit]
		JMP		get_selected_units_at_point_6FDCE
	}
}

const u32 get_selected_units_at_point_6FE41 = 0x0046FE41;
void __declspec(naked) get_selected_units_at_point_patch_6FE3C() {
	__asm {
		LEA		EDX, [EBP - newDestUnit]
		PUSH	EDX
		PUSH	EAX
		JMP		get_selected_units_at_point_6FE41
	}
}

const u32 get_selected_units_at_point_6FE6A = 0x0046FE6A;
void __declspec(naked) get_selected_units_at_point_patch_6FE65() {
	__asm {
		PUSH	1
		LEA		EBX, [EBP - newDestUnit]
		JMP		get_selected_units_at_point_6FE6A
	}
}*/

const u32 command_save_hotkey_group_96602 = 0x00496602;
const u32 command_save_hotkey_group_966A4 = 0x004966A4;
void __declspec(naked) command_save_hotkey_group_patch_965E1() {
	u32 player;
	u32 group;

	__asm {
		PUSH	EDI
		MOVZX	EDX, AL
		PUSHAD
		MOV		EBP, ESP
		MOV		player, EBX
		MOV		group, EDX
	}

	tmp = (u32) &ccmu_extender.new_saved_selections[player][group];//[0];

	__asm {
		POPAD
		TEST	ECX, ECX
		JNE		loc_496695
		MOV		EAX, tmp
		MOV		ESI, EBX
		JMP		command_save_hotkey_group_96602

		loc_496695:
		MOV		EDI, tmp
		MOV		EAX, EBX
		JMP		command_save_hotkey_group_966A4
	}
}

const u32 command_save_hotkey_group_966CC = 0x004966CC;
void __declspec(naked) command_save_hotkey_group_patch_96653() {
	__asm {
		MOV		EAX, tmp
		ADD		EAX, 4
		ALIGN 4

		loc_496660:
		MOV		ECX, [EAX - 4]
		TEST	ECX, ECX
		JE		offset loc_4966CC
		CMP		dword ptr [EAX], 0
		JE		offset loc_4966B7
		MOV		ECX, [EAX + 4]
		TEST	ECX, ECX
		JE		offset loc_4966BA
		MOV		ECX, [EAX + 8]
		TEST	ECX, ECX
		JE		offset loc_4966BF
		MOV		ECX, [EAX + 0xC]
		TEST	ECX, ECX
		JE		offset loc_4966C4
		MOV		ECX, [EAX + 0x10]
		TEST	ECX, ECX
		JE		offset loc_4966C9
		ADD		EDI, 6
		ADD		EAX, 0x18
		CMP		EDI, SelectionLimit
		JL		offset loc_496660

		loc_4966CC:
		JMP		command_save_hotkey_group_966CC

		loc_4966B7:
		INC		EDI
		JMP		command_save_hotkey_group_966CC

		loc_4966BA:
		ADD		EDI, 2
		JMP		command_save_hotkey_group_966CC

		loc_4966BF:
		ADD		EDI, 3
		JMP		command_save_hotkey_group_966CC

		loc_4966C4:
		ADD		EDI, 4
		JMP		command_save_hotkey_group_966CC

		loc_4966C9:
		ADD		EDI, 5
		JMP		command_save_hotkey_group_966CC
	}
}

const u32 unit_is_standard_and_movable = 0x0047B770;
void __declspec(naked) command_save_hotkey_group_patch_966CC() {
	u32 player;
	u32 selection_count;
	u32* saved_selections;
	u32 selection_limit;
	u32 index;

	__asm {
		PUSH	EBP
		MOV		EBP, ESP
		SUB		ESP, 0x10
		MOV		player, EBX
		MOV		selection_count, EDI
	}

	saved_selections = (u32*)tmp;

	tmp = (u32) &ccmu_extender.new_players_selections[player];//[0];

	index = 0;
	selection_limit = SelectionLimit;

	do {
		u32 saved_unit = saved_selections[index];

		if (!saved_unit)
			break;

		Unit* unit = Unit::get_unit_from_index(saved_unit & 0xFFF);

		if (unit->main_order_id == OrderId::Die || unit->unique_id != saved_unit >> 12)
		{
			saved_selections[index] = saved_selections[selection_count - 1];
			saved_selections[selection_count - 1] = NULL;
			--selection_count;
		}

		++index;
	} while (--selection_limit);

	__asm {
		MOV		EBX, selection_count
		MOV		ESP, EBP
		POP		EBP
		MOV		EDI, EBX
		MOV		EBX, tmp
		MOV		dword ptr [EBP - 0xC], 0
		MOV		[EBP - 0x10], EBX

	loc_4966E2:
		MOV		ECX, [EBX]
		TEST	ECX, ECX
		JE		offset loc_496788
		MOVZX	EAX, byte ptr [ECX + 0x4C]//unit->player
		CMP		EAX, ds:[0x00512678]//ACTIVE_NATION_ID
		JNE		offset loc_496788
		MOV		ESI, ECX
		SUB		ESI, offset ccmu_extender.unit_table[0]
		MOV		EAX, 0x86186187
		MUL		ESI
		SUB		ESI, EDX
		SHR		ESI, 1
		ADD		ESI, EDX
		SHR		ESI, 8
		INC		ESI
		CMP		ESI, UnitLimit
		JA		offset loc_49676F
		MOVZX	EBX,  byte ptr [ECX + 0xA5]//unit->unique_id
		SHL		EBX, 0xC//unit "unique identifier" bitshift
		OR		EBX, ESI
		JE		offset loc_49676F
		MOV		EAX, [EBP + 8]
		TEST	EAX, EAX
		JNE		offset loc_496759
		XOR		EAX, EAX
		TEST	EDI, EDI
		JLE		offset loc_49674E
		MOV		EDX, [EBP - 4]
		ALIGN 4

	loc_496740:
		CMP		[EDX], EBX
		JE		offset loc_49676F
		INC		EAX
		ADD		EDX, 4
		CMP		EAX, EDI
		JL		offset loc_496740
		TEST	EDI, EDI

	loc_49674E:
		JE		offset loc_496759
		CALL	unit_is_standard_and_movable
		TEST	EAX, EAX
		JE		offset loc_49676F

	loc_496759:
		MOV		EAX, [EBP - 4]
		MOV		[EAX + EDI * 4], EBX
		INC		EDI
		CMP		EDI, SelectionLimit
		JNL		offset loc_496788

	loc_49676F:
		MOV		EAX, [EBP - 0xC]
		MOV		EBX, [EBP - 0x10]
		INC		EAX
		ADD		EBX, 4
		CMP		EAX, SelectionLimit
		MOV		[EBP - 0xC], EAX
		MOV		[EBP - 0x10], EBX
		JL		offset loc_4966E2

	loc_496788:
		POP		EDI
		POP		ESI
		POP		EBX
		MOV		ESP, EBP
		POP		EBP
		RETN	4
	}
}

/*const u32 centerviewUnitGroup_96907 = 0x00496907;
void __declspec(naked) centerviewUnitGroup_patch_967F7() {
	u32 player;
	u32 group;

	__asm {
		PUSHAD
		MOV		EBP, ESP
		MOV		player, EAX
		MOV		group, ECX
	}

	tmp = (u32) &ccmu_extender.new_saved_selections[player][group];//[1];

	__asm {
		POPAD
		MOV		EAX, tmp
		ADD		EAX, 4
		PUSH	EDI
		XOR		EDX, EDX
		XOR		EDI, EDI
		PUSH	EBX
		MOV		[EBP - 4], EDX
		PUSH	ESI
		ALIGN 4

		loc_496813:
		CMP		[EAX - 4], EDI
		JE		offset loc_496852
		CMP		[EAX], EDI
		JE		offset loc_49683D
		CMP		[EAX + 4], EDI
		JE		offset loc_496840
		CMP		[EAX + 8], EDI
		JE		offset loc_496845
		CMP		[EAX + 0xC], EDI
		JE		offset loc_49684A
		CMP		[EAX + 0x10], EDI
		JE		offset loc_49684F
		ADD		EDX, 6
		ADD		EAX, 0x18
		CMP		EDX, SelectionLimit
		JL		offset loc_496813
		JMP		offset loc_496852

		loc_49683D:
		INC		EDX
		JMP		offset loc_496852

		loc_496840:
		ADD		EDX, 2
		JMP		offset loc_496852

		loc_496845:
		ADD		EDX, 3
		JMP		offset loc_496852

		loc_49684A:
		ADD		EDX, 4
		JMP		offset loc_496852

		loc_49684F:
		ADD		EDX, 5
		JMP		offset loc_496852

		loc_496852:
		MOV		ECX, tmp
		MOV		[EBP - 4], EDX
		MOV		[EBP - 0xC], EDI
		MOV		[EBP - 8], EDI
		MOV		[EBP - 0x10], EDI
		MOV		[EBP - 0x14], ECX
		ALIGN 4

		loc_496870:
		MOV		EAX, [ECX]
		TEST	EAX, EAX
		JE		offset loc_496907
		MOV		EDX, EAX
		AND		EDX, UnitIDMask
		IMUL	EDX, 0x150
		MOV		ESI, [EDX + ccmu_extender.unit_table - 0x144]
		TEST	ESI, ESI
		LEA		ECX, [EDX + ccmu_extender.unit_table - 0x150]
		JE		offset loc_4968EE
		MOV		DL, [ECX + 0x4D]
		TEST	DL, DL
		JNE		offset loc_4968A5
		CMP		byte ptr [ECX + 0x4E], 1
		JE		offset loc_4968EE

		loc_4968A5:
		MOVZX	EBX,  byte ptr [ECX + 0xA5]
		SAR		EAX, 0xC//unit "unique identifier" bitshift
		CMP		EBX, EAX
		JNE		offset loc_4968EE
		TEST	DL, DL
		JNE		offset loc_4968BD
		CMP		byte ptr [ECX + 0x4E], 1
		JE		offset loc_4968EE

		loc_4968BD:
		TEST	byte ptr [ESI + 0xE], 0x20
		JNE		offset loc_4968EE
		CALL	unit_is_standard_and_movable
		TEST	EAX, EAX
		JNE		offset loc_4968D2
		CMP		dword ptr [EBP - 4], 1
		JG		offset loc_4968EE

		loc_4968D2:
		MOV		ECX, [ECX + 0xC]
		MOVSX	EAX, word ptr [ECX + 0x14]
		MOVSX	ECX, word ptr [ECX + 0x16]
		MOV		ESI, [EBP - 0xC]
		MOV		EDX, [EBP - 8]
		ADD		ESI, EAX
		ADD		EDX, ECX
		MOV		[EBP - 0xC], ESI
		MOV		[EBP - 8], EDX
		INC		EDI

		loc_4968EE:
		MOV		EAX, [EBP - 0x10]
		MOV		ECX, [EBP - 0x14]
		INC		EAX
		ADD		ECX, 4
		CMP		EAX, SelectionLimit
		MOV		[EBP - 0x10], EAX
		MOV		[EBP - 0x14], ECX
		JL		offset loc_496870

		loc_496907:
		JMP		centerviewUnitGroup_96907
	}
}*/

const u32 add_to_control_group_969BA = 0x004969BA;
void __declspec(naked) add_to_control_group_patch_96952() {
	u32 player;
	u32 group;

	__asm {
		PUSHAD
		MOV		EBP, ESP
		MOV		player, ESI
		MOV		group, EBX
	}

	tmp = (u32)&ccmu_extender.new_saved_selections[player][group];//[1];

	__asm {
		POPAD
		MOV		EAX, tmp
		ADD		EAX, 4
		PUSH	EDI
		XOR		EDI, EDI
		MOV		[EBP - 4], EDI//count
		MOV		[EBP - 0x10], EBX//group
		ALIGN 4

		loc_496970:
		MOV		ECX, [EAX - 4]
		TEST	ECX, ECX
		JE		offset loc_4969BA
		CMP		dword ptr [EAX], 0
		JE		offset loc_4969A5
		MOV		ECX, [EAX + 4]
		TEST	ECX, ECX
		JE		offset loc_4969A8
		MOV		ECX, [EAX + 8]
		TEST	ECX, ECX
		JE		offset loc_4969AD
		MOV		ECX, [EAX + 0xC]
		TEST	ECX, ECX
		JE		offset loc_4969B2
		MOV		ECX, [EAX + 0x10]
		TEST	ECX, ECX
		JE		offset loc_4969B7
		ADD		EDI, 6
		ADD		EAX, 0x18
		CMP		EDI, SelectionLimit
		JL		offset loc_496970

		loc_4969BA:
		JMP		add_to_control_group_969BA

		loc_4969A5:
		INC		EDI
		JMP		add_to_control_group_969BA

		loc_4969A8:
		ADD		EDI, 2
		JMP		add_to_control_group_969BA

		loc_4969AD:
		ADD		EDI, 3
		JMP		add_to_control_group_969BA

		loc_4969B2:
		ADD		EDI, 4
		JMP		add_to_control_group_969BA

		loc_4969B7:
		ADD		EDI, 5
		JMP		add_to_control_group_969BA
	}
}

const u32 is_human_player_on_same_team = 0x0049A110;
const u32 create_dashed_selection = 0x00499A10;
const u32 add_to_control_group_96AEC = 0x00496AEC;
void __declspec(naked) add_to_control_group_patch_969D9() {
	Unit* unit;
	u32 player;
	u32 group;
	u32 slot;

	__asm {
		PUSHAD
		MOV		EBP, ESP
		MOV		player, ESI
		MOV		group, EBX
	}

	tmp = (u32)&ccmu_extender.new_saved_selections[player][group];//[0];

	__asm {
		POPAD
		XOR		EBX, EBX
		XOR		EDI, EDI//Inline "slot" var and distinguish saved_selection index from it.

		loc_4969E0 :
		MOV		EAX, tmp
			MOV		EAX, [EAX + EBX * 4]
			TEST	EAX, EAX
			JE		offset loc_496ABB
			MOV		ECX, EAX
			AND		ECX, UnitIDMask
			IMUL	ECX, 0x150
			MOV		EDX, [ECX + ccmu_extender.unit_table - 0x144]//new_unit_table[-1].sprite
			TEST	EDX, EDX
			LEA		ECX, [ECX + ccmu_extender.unit_table - 0x150]//new_unit_table[-1]
			MOV[EBP - 0xC], EDX//sprite_xd
			JE		offset loc_496ABB
			MOV		DL, [ECX + 0x4D]//unit->main_order_id
			TEST	DL, DL
			JNE		offset loc_496A34
			CMP		byte ptr[ECX + 0x4E], 1//unit->main_order_state
			JE		offset loc_496ABB

			loc_496A34 :
		MOVZX	EDX, byte ptr[ECX + 0xA5]//unit->unique_id
			SAR		EAX, 0xC//unit "unique identifier" bitshift
			CMP		EDX, EAX
			JNE		offset loc_496ABB
			MOVZX	EAX, byte ptr[ECX + 0x4C]//unit->player
			CMP		EAX, ds: [0x00512678]//ACTIVE_NATION_ID
			MOV[EBP - 0x14], EAX//reference_player
			JNE		offset loc_496ADD//Changed from loc_496ABB to prevent culling mind controlled units from the ctrl group (for CMBW).
			MOV		EAX, [EBP - 0xC]//sprite_xd
			TEST	byte ptr[EAX + 0xE], 0x20//sprite_xd->flags & CSprite_Flags::Hidden
			JNE		offset loc_496ADD//Changed from loc_496ABB to prevent culling units inside transports from the ctrl group.
			CALL	unit_is_standard_and_movable
			TEST	EAX, EAX
			JNE		offset loc_496A69
			CMP		dword ptr[EBP - 4], 1//count
			JG		offset loc_496ADD//Changed from loc_496ABB to prevent culling units affected by stasis/lockdown/maelstrom from the ctrl group.

			loc_496A69 :
		CMP		ESI, 8
			JNL		offset loc_496AB5
			CMP		EDI, SelectionLimit
			JNL		offset loc_496AB5
			TEST	EDI, EDI
			JLE		offset loc_496A83
			CALL	unit_is_standard_and_movable
			TEST	EAX, EAX
			JE		offset loc_496AB5

			loc_496A83 :
		PUSHAD
			MOV		EBP, ESP
			MOV		unit, ECX
			MOV		player, ESI
			MOV		slot, EDI
	}

	ccmu_extender.new_players_selections[player][slot] = unit;

	__asm {
		POPAD
		MOV		EAX, ESI
		CALL	is_human_player_on_same_team
		TEST	EAX, EAX
		JE		offset loc_496AB5
		MOV		ECX, [EBP - 0x14]//reference_player
		MOV		ESI, [EBP - 0xC]//sprite_xd
		XOR		EDX, EDX
		MOV		DL, [ECX + 0x00581D6A]//g_selection_circle_color
		PUSH	EDX
		CALL	create_dashed_selection
		MOV		ESI, ds: [0x0051267C]//ACTIVE_PLAYER_ID

	loc_496AB5 :
		INC		EDI
		JMP		offset loc_496ADD

	loc_496ABB :
		MOV		EAX, [EBP - 4]//count
		DEC		EAX
		MOV[EBP - 4], EAX//count
		MOV		ECX, tmp
		MOV		EDX, [ECX + EAX * 4]
		MOV[ECX + EBX * 4], EDX//ccmu_extender.new_saved_selections[player][group][slot] = ccmu_extender.new_saved_selections[player][group][count];
		MOV		dword ptr[ECX + EAX * 4], 0//ccmu_extender.new_saved_selections[player][group][count] = NULL;
		CMP		EBX, EAX
		JL		offset loc_4969E0
		MOV		EBX, [EBP - 0x10]//group
		JMP		add_to_control_group_96AEC

	loc_496ADD:
		INC		EBX
		CMP		EBX, [EBP - 4]//count
		JL		offset loc_4969E0
		MOV		EBX, [EBP - 0x10]//group
		JMP		add_to_control_group_96AEC
	}
}

const u32 selectUnitGroup_96B81 = 0x00496B81;
void __declspec(naked) selectUnitGroup_patch_96B41() {
	u32 player;
	u32 group;

	__asm {
		MOV		EBP, ESP
		SUB		ESP, SelectionLimit_dword + 12
		MOV		EAX, ds:[0x00512688]//g_local_human_id
		MOVZX	ECX, [EBP + 8]
		PUSHAD
		MOV		EBP, ESP
		MOV		player, EAX
		MOV		group, ECX
	}

	tmp = (u32) &ccmu_extender.new_saved_selections[player][group];//[0];

	__asm {
		POPAD
		MOV		EDX, tmp
		PUSH	ESI
		XOR		ESI, ESI
		PUSH	EDI
		MOV		EAX, EDX

		loc_496B67:
		CMP		dword ptr [EAX], 0
		JE		offset loc_496B75
		INC		ESI
		ADD		EAX, 4
		CMP		ESI, SelectionLimit
		JL		offset loc_496B67

		loc_496B75:
		XOR		EAX, EAX
		TEST	ESI, ESI
		MOV		ECX, SelectionLimit
		LEA		EDI, [EBP - 8 - SelectionLimit_dword]
		JMP		selectUnitGroup_96B81
	}
}

const u32 selectUnitGroup_96BFF = 0x00496BFF;
void __declspec(naked) selectUnitGroup_patch_96BFA() {
	__asm {
		MOV		[EBP + EAX * 4 - 8 - SelectionLimit_dword], ECX
		INC		EAX
		JMP		selectUnitGroup_96BFF
	}
}

const u32 selectUnitGroup_96CB5 = 0x00496CB5;
void __declspec(naked) selectUnitGroup_patch_96CA3() {
	__asm {
		LEA		EAX, [EBP - 8 - SelectionLimit_dword]
		CALL	create_new_unit_selections_from_list
		MOV		EDI, [EBP - 4]
		XOR		ESI, ESI
		TEST	EDI, EDI
		LEA		EBX, [EBP - 8 - SelectionLimit_dword]
		JMP		selectUnitGroup_96CB5
	}
}

const u32 selectUnitGroup_96CEB = 0x00496CEB;
void __declspec(naked) selectUnitGroup_patch_96CE6() {
	__asm {
		LEA		EDX, [EBP - 8 - SelectionLimit_dword]
		XOR		EAX, EAX
		JMP		selectUnitGroup_96CEB
	}
}

const u32 select_single_unit_from_id_96D67 = 0x00496D67;
void __declspec(naked) select_single_unit_from_id_patch_96D61() {
	__asm {
		SUB		EDI, SelectionLimit_dword
		MOV		[EBP - 8], EDI
		JMP		select_single_unit_from_id_96D67
	}
}

const u32 create_unit_selection = 0x004E6180;
const u32 create_all_selection_circles_99AAE = 0x00499AAE;
void __declspec(naked) create_all_selection_circles_patch_99A70() {
	__asm {
		loc_499A70:
		MOV		ESI, [EBX * 4 + ccmu_extender.new_active_selection]
		TEST	ESI, ESI
		JE		offset loc_499AAE
		MOVZX	EAX, word ptr [ESI + 0x64]
		TEST	byte ptr [EAX * 4 + 0x00664080], 0x10//unitsdat_base_property
		PUSH	EBX
		JE		offset loc_499AA1
		MOV		EAX, [ESI + 0x70]
		CALL	create_unit_selection
		MOV		EDX, [ESI + 0x70]
		MOVZX	ECX, BL
		MOV		[ECX * 4 + ccmu_extender.new_active_selection], EDX
		JMP		offset loc_499AA8

		loc_499AA1:
		MOV		EAX, ESI
		CALL	create_unit_selection

		loc_499AA8:
		INC		EBX
		CMP		EBX, SelectionLimit
		JL		offset loc_499A70

		loc_499AAE:
		JMP		create_all_selection_circles_99AAE
	}
}

const u32 illegal_team_check = 0x00485710;
const u32 create_selection_circle = 0x004D7070;
const u32 create_all_selection_circles_99B8E = 0x00499B8E;
void __declspec(naked) create_all_selection_circles_patch_99B23() {
	__asm {
		loc_499B23:
		MOV		EAX, [EDI]
		TEST	EAX, EAX
		JE		offset loc_499B8E
		MOVZX	ECX, byte ptr [EAX + 0x4C]
		MOV		DL, [ECX + 0x00581D6A]//g_selection_circle_color
		MOV		ESI, [EAX + 0xC]
		MOV		BL, [ESI + 0xE]
		MOV		byte ptr [EBP - 0x14], DL//color
		CALL	illegal_team_check
		XOR		ECX, ECX
		MOV		CL, BL
		MOVZX	EAX, AL
		DEC		EAX
		SHR		ECX, 1
		AND		ECX, 3
		CMP		ECX, EAX
		JE		offset loc_499B7F
		MOV		AL, BL
		AND		AL, 0xFE
		ADD		AL, 2
		XOR		AL, BL
		AND		AL, 6
		XOR		AL, BL
		MOV		[ESI + 0xE], AL
		AND		AL, 6
		CMP		AL, 2
		JA		offset loc_499B7F
		MOV		EDX, [EBP - 0x14]//color
		PUSH	0x23B//imageID
		PUSH	EDX//spriteID
		MOV		EAX, ESI//a1
		CALL	create_selection_circle
		TEST	EAX, EAX
		JNE		offset loc_499B7F
		AND		byte ptr [ESI + 0xE], 0xF9

		loc_499B7F:
		MOV		EAX, [EBP - 4]//iterations
		INC		EAX
		ADD		EDI, 4
		CMP		EAX, SelectionLimit
		MOV		[EBP - 4], EAX//iterations
		JL		offset loc_499B23

		loc_499B8E:
		JMP		create_all_selection_circles_99B8E
	}
}

void __declspec(naked) remove_from_selection_2_patch_9A185() {
	u32 player;
	u32 slot;
	
	__asm {
		PUSHAD
		MOV		EBP, ESP
		MOV		player, EDI
	}

	tmp = (u32) &ccmu_extender.new_players_selections[player];//[1];

	__asm {
		POPAD
		MOV		ECX, tmp
		ADD		ECX, 4
		PUSH	EBX
		MOV		EBX, SelectionLimit

		loc_49A197:
		MOV		EDX, [ECX - 4]
		TEST	EDX, EDX
		JE		offset loc_49A20B
		CMP		EDX, ESI
		JNE		offset loc_49A1A4
		MOV		EBX, EAX

		loc_49A1A4:
		MOV		EDX, [ECX]
		TEST	EDX, EDX
		JE		offset loc_49A1F6
		CMP		EDX, ESI
		JNE		offset loc_49A1B1
		LEA		EBX, [EAX + 1]

		loc_49A1B1:
		MOV		EDX, [ECX + 4]
		TEST	EDX, EDX
		JE		offset loc_49A1F9
		CMP		EDX, ESI
		JNE		offset loc_49A1BF
		LEA		EBX, [EAX + 2]

		loc_49A1BF:
		MOV		EDX, [ECX + 8]
		TEST	EDX, EDX
		JE		offset loc_49A1FE
		CMP		EDX, ESI
		JNE		offset loc_49A1CD
		LEA		EBX, [EAX + 3]

		loc_49A1CD:
		MOV		EDX, [ECX + 0xC]
		TEST	EDX, EDX
		JE		offset loc_49A203
		CMP		EDX, ESI
		JNE		offset loc_49A1DB
		LEA		EBX, [EAX + 4]

		loc_49A1DB:
		MOV		EDX, [ECX + 0x10]
		TEST	EDX, EDX
		JE		offset loc_49A208
		CMP		EDX, ESI
		JNE		offset loc_49A1E9
		LEA		EBX, [EAX + 5]

		loc_49A1E9:
		ADD		EAX, 6
		ADD		ECX, 0x18
		CMP		EAX, SelectionLimit
		JL		offset loc_49A197
		JMP		offset loc_49A20B

		loc_49A1F6:
		INC		EAX
		JMP		offset loc_49A20B

		loc_49A1F9:
		ADD		EAX, 2
		JMP		offset loc_49A20B

		loc_49A1FE:
		ADD		EAX, 3
		JMP		offset loc_49A20B

		loc_49A203:
		ADD		EAX, 4
		JMP		offset loc_49A20B

		loc_49A208:
		ADD		EAX, 5
		JMP		offset loc_49A20B

		loc_49A20B:
		TEST	ESI, ESI
		JE		offset loc_49A24C
		TEST	EAX, EAX
		JE		offset loc_49A24C
		CMP		EBX, SelectionLimit
		JE		offset loc_49A24C
		DEC		EAX
		CMP		EBX, EAX
		JGE		offset loc_49A23B
		PUSHAD
		MOV		EBP, ESP
		MOV		player, EDI
		MOV		slot, EBX
	}

	tmp = (u32)&ccmu_extender.new_players_selections[player][slot];

	__asm {
		POPAD
		MOV		EDI, tmp
		LEA		ESI, [EDI + 4]//&ccmu_extender.new_players_selections[player][slot + 1]
		MOV		ECX, EAX
		SUB		ECX, EBX
		REPE MOVSD
		MOV		EDI, [EBP + 8]

		loc_49A23B:
		PUSHAD
		MOV		EBP, ESP
		MOV		player, EDI
		MOV		slot, EAX
	}

	ccmu_extender.new_players_selections[player][slot] = NULL;

	__asm {
		POPAD

		loc_49A24C:
		POP		EBX
		POP		EDI
		POP		ESI
		POP		EBP
		RETN	4
	}
}

const u32 remove_selection_circle_from_sprite = 0x00497590;
void __declspec(naked) clear_selection_patch_9A749() {
	u32 player;

	__asm {
		PUSHAD
		MOV		EBP, ESP
		MOV		player, ESI
	}

	tmp = (u32)&ccmu_extender.new_players_selections[player];//[0];

	__asm {
		POPAD
		CALL	is_human_player_on_same_team
		TEST	EAX, EAX
		JNE		loc_49A768
		MOV		EDI, tmp
		MOV		ECX, SelectionLimit
		REPE STOSD
		POP		EDI
		POP		ESI
		RET

		loc_49A768:
		MOV		ESI, tmp
		XOR		EDI, EDI
		ALIGN 4

		loc_49A776:
		MOV		EAX, [ESI]
		TEST	EAX, EAX
		JE		offset loc_49A7B2
		MOV		EAX, [EAX + 0xC]
		TEST	EAX, EAX
		JE		offset loc_49A7B2
		MOV		DL, [EAX + 0xE]
		MOV		CL, DL
		SHR		CL, 1
		TEST	CL, 3
		JE		offset loc_49A7A3
		DEC		CL
		SHL		CL, 1
		XOR		CL, DL
		AND		CL, 6
		XOR		CL, DL
		TEST	CL, 6
		MOV		[EAX + 0xE], CL
		JA		offset loc_49A7A3
		CALL	remove_selection_circle_from_sprite

		loc_49A7A3:
		MOV		dword ptr [ESI], 0
		INC		EDI
		ADD		ESI, 4
		CMP		EDI, SelectionLimit
		JL		offset loc_49A776

		loc_49A7B2:
		POP		EDI
		POP		ESI
		RET
	}
}

const u32 removeFromSelections = 0x0049A7F0;
const u32 get_active_player_first_selection_9A898 = 0x0049A898;
void __declspec(naked) get_active_player_first_selection_patch_9A857() {
	u32 player;
	u32 slot;

	__asm {
		CMP		BL, SelectionLimit//192-limit: g_unit_iterator_pos (0x006284B6) is a BYTE. Technically it appears to be a WORD, but is only referenced as a byte ptr thoughout the exe.
		JB		offset loc_49A860
		XOR		EAX, EAX
		POP		EBX
		RET

		loc_49A860:
		MOV		EAX, ds:[0x0051267C]//ACTIVE_PLAYER_ID
		MOVZX	ECX, BL
		PUSHAD
		MOV		EBP, ESP
		MOV		player, EAX
		MOV		slot, ECX
	}

	tmp = (u32) &ccmu_extender.new_players_selections[player][slot];

	__asm {
		POPAD
		PUSH	ESI
		PUSH	EDI

		loc_49A870:
		MOV		EAX, tmp
		MOV		EAX, [EAX]//unit
		TEST	EAX, EAX
		JE		offset loc_49A898
		MOV		ECX, [EAX + 0xC]//unit->sprite
		TEST	ECX, ECX
		JE		offset loc_49A88F
		MOV		CL, [EAX + 0x4D]//unit->main_order_id
		TEST	CL, CL
		JNE		offset loc_49A898
		CMP		byte ptr [EAX + 0x4E], 1//unit->main_order_state
		JNE		offset loc_49A898

		loc_49A88F:
		MOV		EDI, EAX
		CALL	removeFromSelections
		JMP		offset loc_49A870

		loc_49A898:
		JMP		get_active_player_first_selection_9A898
	}
}

const u32 add_to_player_selection_9AF8E = 0x0049AF8E;
void __declspec(naked) add_to_player_selection_patch_9AF89() {
	__asm {
		CMP		EBX, SelectionLimit
		JGE		offset loc_49AFB2
		JMP		add_to_player_selection_9AF8E

		loc_49AFB2:
		XOR		EAX, EAX
		RET
	}
}

const u32 add_to_player_selection_patch_9AFC2 = 0x0049AFC2;
void __declspec(naked) add_to_player_selection_patch_9AFB5() {
	Unit* unit;
	u32 player;
	u32 slot;
	
	__asm {
		PUSHAD
		MOV		EBP, ESP
		MOV		unit, ESI
		MOV		player, EDI
		MOV		slot, EBX
	}
	
	ccmu_extender.new_players_selections[player][slot] = unit;

	__asm {
		POPAD
		JMP		add_to_player_selection_patch_9AFC2
	}
}

const u32 destroy_unit_selection = 0x004E6290;
const u32 select_unit_initial_9B70D = 0x0049B70D;
void __declspec(naked) select_unit_initial_patch_9B6A0() {
	__asm {
		loc_49B6A0:
		MOV		ESI, [EBX * 4 + ccmu_extender.new_active_selection]
		TEST	ESI, ESI
		JE		offset loc_49B70D
		MOV		EAX, ds:[0x006D0F14]//g_is_in_replay
		TEST	EAX, EAX
		JNZ		loc_49B6C0
		MOVZX	EAX, byte ptr [ESI + 0x4C]//unit - >player
		CMP		EAX, ds:[0x00512684]//g_local_nation_id
		JE		offset loc_49B707

		loc_49B6C0:
		MOVZX	ECX, word ptr [ESI + 0x64]//unit - >id
		TEST	byte ptr [ECX * 4 + 0x00664080], 0x10//units_dat::Subunit
		JE		offset loc_49B6D3
		MOV		EAX, [ESI + 0x70]//unit - >subunit
		JMP		offset loc_49B6D5

		loc_49B6D3:
		MOV		EAX, ESI

		loc_49B6D5:
		CALL	destroy_unit_selection
		MOVZX	EDX, word ptr [ESI + 0x64]//unit - >id
		TEST	byte ptr [EDX * 4 + 0x00664080], 0x10//units_dat::Subunit
		PUSH	EBX
		JE		offset loc_49B700
		MOV		EAX, [ESI + 0x70]//unit - >subunit
		CALL	create_unit_selection
		MOV		ECX, [ESI + 0x70]//unit - >subunit
		MOVZX	EAX, BL
		MOV		[EAX * 4 + ccmu_extender.new_active_selection], ECX
		JMP		offset loc_49B707

		loc_49B700:
		MOV		EAX, ESI
		CALL	create_unit_selection

		loc_49B707:
		INC		EBX
		CMP		EBX, SelectionLimit
		JB		offset loc_49B6A0

		loc_49B70D:
		JMP		select_unit_initial_9B70D
	}
}

const u32 remove_selection_circle_image = 0x004D4FA0;
const u32 select_unit_initial_9B846 = 0x0049B846;
void __declspec(naked) select_unit_initial_patch_9B770() {
	__asm {
		loc_49B770:
		MOV		EBX, [EDI]
		TEST	EBX, EBX
		JE		offset loc_49B846
		MOV		EAX, ds:[0x006D0F14]//g_is_in_replay
		TEST	EAX, EAX
		JNE		offset loc_49B793
		MOVZX	ECX, byte ptr [EBX + 0x4C]//unit - >player
		CMP		ECX, ds:[0x00512684]//g_local_nation_id
		JE		offset loc_49B833

		loc_49B793:
		MOV		EDX, [EBX + 0xC]//unit - >sprite
		MOV		CL, [EDX + 0xE]//sprite - >flags
		MOV		AL, CL
		SHR		AL, 1
		TEST	AL, 3
		JE		offset loc_49B7DD
		DEC		AL
		SHL		AL, 1
		XOR		AL, CL
		AND		AL, 6
		XOR		AL, CL
		TEST	AL, 6
		MOV		[EDX + 0xE], AL
		JA		offset loc_49B7DD
		MOV		ESI, [EDX + 0x20]//sprite - >image - >tail
		TEST	ESI, ESI
		JE		offset loc_49B7DD
		ALIGN 4

		loc_49B7C0:
		MOV		AX, [ESI + 8]
		CMP		AX, 0x23B
		JB		offset loc_49B7D0
		CMP		AX, 0x244
		JBE loc_49B7D8

		loc_49B7D0:
		MOV		ESI, [ESI]
		TEST	ESI, ESI
		JNE		offset loc_49B7C0
		JMP		offset loc_49B7DD

		loc_49B7D8:
		CALL	remove_selection_circle_image

		loc_49B7DD:
		MOVZX	EDX, byte ptr [EBX + 0x4C]
		MOV		AL, [EDX + 0x00581D6A]//g_selection_circle_color
		MOV		ESI, [EBX + 0xC]
		MOV		BL, [ESI + 0xE]
		MOV		byte ptr [EBP - 0x14], AL//color
		CALL	illegal_team_check
		XOR		EDX, EDX
		MOV		DL, BL
		MOVZX	ECX, AL
		DEC		ECX
		SHR		EDX, 1
		AND		EDX, 3
		CMP		EDX, ECX
		JE		offset loc_49B833
		MOV		AL, BL
		AND		AL, 0xFE
		ADD		AL, 2
		XOR		AL, BL
		AND		AL, 6
		XOR		AL, BL
		MOV		[ESI + 0xE], AL
		AND		AL, 6
		CMP		AL, 2
		JA		offset loc_49B833
		MOV		EAX, [EBP - 0x14]//color
		PUSH	0x23B//imageID
		PUSH	EAX//spriteID
		MOV		EAX, ESI//a1
		CALL	create_selection_circle
		TEST	EAX, EAX
		JNE		offset loc_49B833
		AND		byte ptr [ESI + 0xE], 0xF9

		loc_49B833:
		MOV		EAX, [EBP - 8]//iterations_3
		INC		EAX
		ADD		EDI, 4
		CMP		EAX, SelectionLimit
		MOV		[EBP - 8], EAX//iterations_3
		JB		offset loc_49B770

		loc_49B846:
		JMP		select_unit_initial_9B846
	}
}

const u32 remove_from_client_selection_3_9F7A6 = 0x0049F7A6;
void __declspec(naked) remove_from_client_selection_3_patch_9F7A1() {
	__asm {
		MOV		EBP, ESP
		SUB		ESP, SelectionLimit_dword
		JMP		remove_from_client_selection_3_9F7A6
	}
}

const u32 remove_from_client_selection_3_9F7DB = 0x0049F7DB;
void __declspec(naked) remove_from_client_selection_3_patch_9F7C0() {
	__asm {
		loc_49F7C0:
		MOV		ECX, [EBX * 4 + ccmu_extender.new_active_selection]
		TEST	ECX, ECX
		JE		offset loc_49F7D1
		INC		EBX
		CMP		EBX, SelectionLimit
		JL		offset loc_49F7C0

		loc_49F7D1:
		MOV		ECX, EBX
		MOV		ESI, offset ccmu_extender.new_active_selection[0]
		LEA		EDI, [EBP - SelectionLimit_dword]
		JMP		remove_from_client_selection_3_9F7DB
	}
}

const u32 remove_from_client_selection_3_9F7F0 = 0x0049F7F0;
void __declspec(naked) remove_from_client_selection_3_patch_9F7E6() {
	__asm {
		LEA		EAX, [EBP + EAX * 4 - SelectionLimit_dword]
		MOV		EDX, EAX
		ADD		EDX, 4
		PUSH	EDX
		PUSH	EAX
		JMP		remove_from_client_selection_3_9F7F0
	}
}

const u32 remove_from_client_selection_3_9F802 = 0x0049F802;
void __declspec(naked) remove_from_client_selection_3_patch_9F7FB() {
	__asm {
		LEA		EAX, [EBP - SelectionLimit_dword]
		MOV		[EBP + EBX * 4 - SelectionLimit_dword], ESI
		JMP		remove_from_client_selection_3_9F802
	}
}

/*void __declspec(naked) removeFromSelections_patch_BF8C0() {
	__asm {
		XOR		EAX, EAX

		loc_4BF8C2:
		MOV		ECX, [EAX * 4 + ccmu_extender.new_local_selection_group]
		TEST	ECX, ECX
		JE		offset loc_4BF923
		MOV		ECX, [EAX * 4 + ccmu_extender.new_local_selection_group + 4]
		TEST	ECX, ECX
		JE		offset loc_4BF90E
		MOV		ECX, [EAX * 4 + ccmu_extender.new_local_selection_group + 8]
		TEST	ECX, ECX
		JE		offset loc_4BF911
		MOV		ECX, [EAX * 4 + ccmu_extender.new_local_selection_group + 0xC]
		TEST	ECX, ECX
		JE		offset loc_4BF916
		MOV		ECX, [EAX * 4 + ccmu_extender.new_local_selection_group + 0x10]
		TEST	ECX, ECX
		JE		offset loc_4BF91B
		MOV		ECX, [EAX * 4 + ccmu_extender.new_local_selection_group + 0x14]
		TEST	ECX, ECX
		JE		offset loc_4BF920
		ADD		EAX, 6
		CMP		EAX, SelectionLimit
		JL		offset loc_4BF8C2
		JMP		offset loc_4BF923

		loc_4BF90E:
		INC		EAX
		JMP		offset loc_4BF923

		loc_4BF911:
		ADD		EAX, 2
		JMP		offset loc_4BF923

		loc_4BF916:
		ADD		EAX, 3
		JMP		offset loc_4BF923

		loc_4BF91B:
		ADD		EAX, 4
		JMP		offset loc_4BF923

		loc_4BF920:
		ADD		EAX, 5

		loc_4BF923:
		XOR		ECX, ECX
		TEST	EAX, EAX
		JLE		offset loc_4BF959
		ALIGN 4

		loc_4BF930:
		CMP		[ECX * 4 + ccmu_extender.new_local_selection_group], EDX
		JE		offset loc_4BF93F
		INC		ECX
		CMP		ECX, EAX
		JL		offset loc_4BF930
		RET

		loc_4BF93F:
		MOV		EDX, [EAX * 4 + ccmu_extender.new_local_selection_group - 4]
		DEC		EAX
		MOV		[ECX * 4 + ccmu_extender.new_local_selection_group], EDX
		MOV		dword ptr [EAX * 4 + ccmu_extender.new_local_selection_group], 0

		loc_4BF959:
		RET
	}
}*/

const u32 CMDRECV_ShiftDeselect_BFB4F = 0x004BFB4F;
void __declspec(naked) CMDRECV_ShiftDeselect_patch_BFB49() {
	__asm {
		MOV		CL, [EDX + 1]//192-limit: selection count is a BYTE in the cmdact/cmdrecv buffer.
		CMP		CL, SelectionLimit
		JMP		CMDRECV_ShiftDeselect_BFB4F
	}
}

const u32 cmdact_select_C08C0 = 0x004C08C0;
void __declspec(naked) cmdact_select_patch_C0863() {
	__asm {
		SUB		ESP, 3 * (SelectionLimit_word + 2) + 8
		PUSH	EBX
		PUSH	ESI
		PUSH	EDI

		MOV		ECX, 3 * (SelectionLimit + 1)
		MOV		EAX, 0
		LEA		EDI, [EBP - 0xA]
		STD
		REPE STOSW
		CLD

		MOV		EBX, [EBP + 8]//count
		MOV		EDI, [EBP + 0xC]//units
		XOR		EDX, EDX
		TEST	EBX, EBX
		JMP		cmdact_select_C08C0
	}
}

const u32 cmdact_select_C0902 = 0x004C0902;
const u32 cmdact_select_C08F7 = 0x004C08F7;
void __declspec(naked) cmdact_select_patch_C08E0() {
	__asm {
		loc_4C08E0:
		MOV		ECX, [EAX * 4 + ccmu_extender.new_local_selection_group]
		TEST	ECX, ECX
		JE		offset loc_4C0902
		CMP		ECX, ESI
		JE		offset loc_4C08F7
		INC		EAX
		CMP		EAX, SelectionLimit
		JB		offset loc_4C08E0

		loc_4C0902:
		JMP		cmdact_select_C0902

		loc_4C08F7:
		JMP		cmdact_select_C08F7
	}
}

const u32 cmdact_select_C095E = 0x004C095E;
void __declspec(naked) cmdact_select_patch_C0959() {
	__asm {
		MOV		word ptr [EBP + ECX * 2 - buffer1_stack], AX
		JMP		cmdact_select_C095E
	}
}

const u32 cmdact_select_C09EB = 0x004C09EB;
void __declspec(naked) cmdact_select_patch_C09E6() {
	__asm {
		MOV		word ptr [EBP + ECX * 2 - buffer2_stack], AX
		JMP		cmdact_select_C09EB
	}
}

const u32 cmdact_select_C0A6C = 0x004C0A6C;
void __declspec(naked) cmdact_select_patch_C0A67() {
	__asm {
		MOV		word ptr [EBP + EDI * 2 - buffer3_stack], AX
		JMP		cmdact_select_C0A6C
	}
}

const u32 queue_game_command = 0x00485BD0;
void __declspec(naked) cmdact_select_patch_C0A71() {
	__asm {
		LEA		ECX, [EBP - buffer3_type]
		LEA		EDX, [EAX + EAX + 2]//size
		MOV		byte ptr [EBP - buffer3_type], 0x9
		MOV		byte ptr [EBP - buffer3_size], AL
		CALL	queue_game_command
		POP		EDI
		POP		ESI
		POP		EBX
		MOV		ESP, EBP
		POP		EBP
		RETN	8
	}
}

void __declspec(naked) cmdact_select_patch_C0A8D() {
	__asm {
		TEST	EDX, EDX
		JE		offset loc_4C0AA4
		MOV		byte ptr [EBP - buffer2_size], DL
		LEA		EDX, [EDX + EDX + 2]//size
		LEA		ECX, [EBP - buffer2_type]
		MOV		byte ptr [EBP - buffer2_type], 0xB
		CALL	queue_game_command

		loc_4C0AA4:
		MOV		EAX, [EBP - 4]
		TEST	EAX, EAX
		JE		offset loc_4C0ABE
		LEA		ECX, [EBP - buffer1_type]
		LEA		EDX, [EAX + EAX + 2]//size
		MOV		byte ptr [EBP - buffer1_type], 0xA
		MOV		byte ptr [EBP - buffer1_size], AL
		CALL	queue_game_command

		loc_4C0ABE:
		POP		EDI
		POP		ESI
		POP		EBX
		MOV		ESP, EBP
		POP		EBP
		RETN	8
	}
}

const u32 cmdrecv_shift_select_C25E9 = 0x004C25E9;
const u32 cmdrecv_shift_select_C2747 = 0x004C2747;
void __declspec(naked) cmdrecv_shift_select_patch_C256D() {
	u32 player;

	__asm {
		CMP		DL, SelectionLimit//192-limit: selection count is a BYTE in the cmdact/cmdrecv buffer.
		JNA		offset loc_4C2576
		POP		ESI
		MOV		ESP, EBP
		POP		EBP
		RETN	4

		loc_4C2576:
		MOV		EAX, ds:[0x0051267C]//ACTIVE_PLAYER_ID
		PUSHAD
		MOV		EBP, ESP
		MOV		player, EAX
	}

	tmp = (u32) &ccmu_extender.new_players_selections[player];

	__asm {
		POPAD
		MOV		EAX, tmp
		ADD		EAX, 4
		XOR		ECX, ECX
		PUSH	EDI
		ALIGN 4

		loc_4C2590:
		MOV		EDI, [EAX - 4]
		TEST	EDI, EDI
		JE		offset loc_4C25DA
		CMP		dword ptr [EAX], 0
		JE		offset loc_4C25C5
		MOV		EDI, [EAX + 4]
		TEST	EDI, EDI
		JE		offset loc_4C25C8
		MOV		EDI, [EAX + 8]
		TEST	EDI, EDI
		JE		offset loc_4C25CD
		MOV		EDI, [EAX + 0xC]
		TEST	EDI, EDI
		JE		offset loc_4C25D2
		MOV		EDI, [EAX + 0x10]
		TEST	EDI, EDI
		JE		offset loc_4C25D7
		ADD		ECX, 6
		ADD		EAX, 0x18
		CMP		ECX, SelectionLimit
		JL		offset loc_4C2590
		JMP		offset loc_4C25DA

		loc_4C25C5:
		INC		ECX
		JMP		offset loc_4C25DA

		loc_4C25C8:
		ADD		ECX, 2
		JMP		offset loc_4C25DA

		loc_4C25CD:
		ADD		ECX, 3
		JMP		offset loc_4C25DA

		loc_4C25D2:
		ADD		ECX, 4
		JMP		offset loc_4C25DA

		loc_4C25D7:
		ADD		ECX, 5

		loc_4C25DA:
		MOVZX	EAX, DL
		LEA		EDX, [EAX + ECX]
		CMP		EDX, SelectionLimit
		JA		offset loc_4C2747//Changed from JG to JA for unsigned comparison.
		JMP		cmdrecv_shift_select_C25E9

		loc_4C2747:
		JMP		cmdrecv_shift_select_C2747
	}
}

const u32 cmdrecv_shift_select_C267C = 0x004C267C;
const u32 cmdrecv_shift_select_C26CE = 0x004C26CE;
void __declspec(naked) cmdrecv_shift_select_patch_C2650() {
	__asm {
		MOV		EAX, tmp
		XOR		ECX, ECX

		loc_4C2662:
		MOV		EDX, [EAX + ECX * 4]
		TEST	EDX, EDX
		JE		offset loc_4C267C
		CMP		EDX, ESI
		JE		offset loc_4C2675
		INC		ECX
		CMP		ECX, SelectionLimit
		JB		offset loc_4C2662

		loc_4C267C:
		JMP		cmdrecv_shift_select_C267C

		loc_4C2675:
		LEA		EAX, [EAX + ECX * 4]
		TEST	EAX, EAX
		JE		offset loc_4C267C
		JMP		cmdrecv_shift_select_C26CE
	}
}

const u32 cmdrecv_shift_select_C268A = 0x004C268A;
void __declspec(naked) cmdrecv_shift_select_patch_C2685() {
	__asm {
		CMP		EDI, SelectionLimit
		JAE		offset loc_4C26CE//Changed from JGE to JAE for unsigned comparison.
		JMP		cmdrecv_shift_select_C268A

		loc_4C26CE:
		JMP		cmdrecv_shift_select_C26CE
	}
}

const u32 cmdrecv_shift_select_C26BD = 0x004C26BD;
void __declspec(naked) cmdrecv_shift_select_patch_C26AB() {
	Unit* unit;
	u32 player;
	u32 slot;

	__asm {
		MOV		EAX, ds:[0x0051267C]//ACTIVE_PLAYER_ID
		PUSHAD
		MOV		EBP, ESP
		MOV		unit, ESI
		MOV		player, EAX
		MOV		slot, EDI
	}

	ccmu_extender.new_players_selections[player][slot] = unit;

	__asm {
		POPAD
		MOV		EAX, tmp
		ADD		EAX, 4
		JMP		cmdrecv_shift_select_C26BD
	}
}

const u32 cmdrecv_select_C275E = 0x004C275E;
void __declspec(naked) cmdrecv_select_patch_C2757() {
	__asm {
		MOV		ESI, [EBP + 8]
		CMP		byte ptr [ESI + 1], SelectionLimit//192-limit: selection count is a BYTE in the cmdact/cmdrecv buffer.
		JMP		cmdrecv_select_C275E
	}
}

const u32 cmdrecv_select_C27FA = 0x004C27FA;
const u32 cmdrecv_select_C27F3 = 0x004C27F3;
void __declspec(naked) cmdrecv_select_patch_C27D3() {
	u32 player;

	__asm {
		PUSHAD
		MOV		EBP, ESP
		MOV		player, EDI
	}

	tmp = (u32) &ccmu_extender.new_players_selections[player];

	__asm {
		POPAD
		MOV		EAX, tmp
		XOR		ECX, ECX
		ALIGN 4

		loc_4C27E0:
		MOV		EDX, [EAX + ECX * 4]
		TEST	EDX, EDX
		JE		offset loc_4C27FA
		CMP		EDX, ESI
		JE		offset loc_4C27F3
		INC		ECX
		CMP		ECX, SelectionLimit
		JB		offset loc_4C27E0

		loc_4C27FA:
		JMP		cmdrecv_select_C27FA

		loc_4C27F3:
		JMP		cmdrecv_select_C27F3
	}
}

const u32 update_building_land_unit_selection_C3B46 = 0x004C3B46;
void __declspec(naked) update_building_land_unit_selection_patch_C3B41() {
	__asm {
		MOV		EBP, ESP
		SUB		ESP, SelectionLimit_dword
		JMP		update_building_land_unit_selection_C3B46
	}
}

const u32 update_building_land_unit_selection_C3B5F = 0x004C3B5F;
void __declspec(naked) update_building_land_unit_selection_patch_C3B5A() {
	__asm {
		MOV		[EBP + ESI * 4 - SelectionLimit_dword], EAX
		INC		ESI
		JMP		update_building_land_unit_selection_C3B5F
	}
}

const u32 update_building_land_unit_selection_C3B76 = 0x004C3B76;
void __declspec(naked) update_building_land_unit_selection_patch_C3B6B() {
	__asm {
		LEA		EAX, [EBP - SelectionLimit_dword]
		CALL	create_new_unit_selections_from_list
		LEA		EAX, [EBP - SelectionLimit_dword]
		JMP		update_building_land_unit_selection_C3B76
	}
}

const u32 sub_4D68C0 = 0x004D68C0;
const u32 InitializeImageArray_D69E3 = 0x004D69E3;
void __declspec(naked) InitializeImageArray_patch_D69A0() {
	__asm {
		loc_4D69A0:
		CMP		dword ptr ds:[0x0057EB6C], offset ccmu_extender.new_selection_hp_bars[0]
		JNE		offset loc_4D69B2
		MOV		ds:[0x0057EB6C], ESI

		loc_4D69B2:
		MOV		dword ptr [ESI], offset ccmu_extender.new_selection_hp_bars[0]
		MOV		EAX, [ccmu_extender.new_selection_hp_bars + 4]
		MOV		[ESI + 4], EAX
		MOV		EAX, [ccmu_extender.new_selection_hp_bars + 4]
		CMP		EAX, EBX
		JE		offset loc_4D69CB
		MOV		[EAX], ESI

		loc_4D69CB:
		MOV		EAX, EDI
		MOV		ECX, ESI
		MOV		[ccmu_extender.new_selection_hp_bars + 4], ESI
		CALL	sub_4D68C0
		INC		EDI
		ADD		ESI, 0x40
		CMP		EDI, SelectionLimit
		JB		offset loc_4D69A0
		JMP		InitializeImageArray_D69E3
	}
}

const u32 load_game_core_EEDDC = 0x004EEDDC;
void __declspec(naked) load_game_core_patch_EEDCB() {
	u32 player;

	__asm {
		PUSHAD
		MOV		EBP, ESP
		MOV		player, EAX
	}

	tmp = (u32) &ccmu_extender.new_players_selections[player];

	__asm {
		POPAD
		MOV		ESI, tmp
		MOV		ECX, SelectionLimit
		JMP		load_game_core_EEDDC
	}
}

void __declspec(naked) unit_is_owned_by_current_player_new() {
	__asm {
		PUSH	EBP
		MOV		EBP, ESP
		MOV		EAX, ds:[0x006D0F14]//g_is_in_replay
		TEST	EAX, EAX
		MOV		EAX, [EBP + 8]//pUnit
		MOVZX	ECX, byte ptr [EAX + 0x4C]//unit->player
		JNE		offset is_in_replay
		CMP		ECX, ds:[0x00512684]//g_local_nation_id
		JNE		offset loc_401194

		is_same_player:
		MOV		EAX, 1
		POP		EBP
		RETN	4

		is_in_replay:
		CMP		[EBX + 0x4C], CL
		JE		offset is_same_player

		loc_401194:
		XOR		EAX, EAX
		POP		EBP
		RETN	4
	}
}

const u32 combine_lists_6F2D6 = 0x0046F2D6;
void __declspec(naked) combine_lists_patch_6F2D1() {
	static b32 is_witness_or_in_replay;
	__asm {
		PUSHAD
	}
	is_witness_or_in_replay = globals->players[*g_local_human_id].witness_mode | *g_is_in_replay;
	__asm {
		POPAD
		MOV	EAX, is_witness_or_in_replay
		JMP	combine_lists_6F2D6
	}
}

namespace hooks {
	void inject_selection_extender() {
		//CMDACT_Stimpack (0x004234D0)
		//memory_patch(0x004234D5, &ccmu_extender.new_client_selection_group);
		//memory_patch(0x004234F0, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//unit_CanCloak (0x00423540)
		memory_patch(0x00423556, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004235CD, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//TechUseCheckForGroupEnergyCost (0x00423660)
		memory_patch(0x00423683, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004236A9, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSACT_AttackMove (0x00424380)
		memory_patch(0x00424385, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004243DD, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//UnitStatCond_Powerup (0x00424520)
		memory_patch(0x00424528, &ccmu_extender.new_selGroupHp);

		//storeSelectionGroupHPAndType (0x00424540)
		memory_patch(0x00424542, &ccmu_extender.new_selGroupType[1]);
		memory_patch(0x00424552, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00424565, &ccmu_extender.new_selGroupHp);
		memory_patch(0x0042457C, &ccmu_extender.new_client_selection_group[1]);
		memory_patch(0x0042458F, &ccmu_extender.new_selGroupHp[1]);
		memory_patch(0x004245A5, &ccmu_extender.new_client_selection_group[2]);
		memory_patch(0x004245B8, &ccmu_extender.new_selGroupHp[2]);
		memory_patch(0x004245CF, &ccmu_extender.new_client_selection_group[3]);
		memory_patch(0x004245E2, &ccmu_extender.new_selGroupHp[3]);
		memory_patch(0x004245F9, &ccmu_extender.new_client_selection_group[4]);
		memory_patch(0x0042460C, &ccmu_extender.new_selGroupHp[4]);
		memory_patch(0x00424623, &ccmu_extender.new_client_selection_group[5]);
		memory_patch(0x00424636, &ccmu_extender.new_selGroupHp[5]);

		//isSelGroupUpdated (0x00424660)
		memory_patch(0x00424663, &ccmu_extender.new_selGroupHp);
		memory_patch(0x00424668, &ccmu_extender.new_selGroupType);
		memory_patch(0x0042466D, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00424691, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//unitStatCond_Standard (0x00424980)
		memory_patch(0x00424987, &ccmu_extender.new_selGroupHp);

		//transport_status_update_drawn_values (0x00424FC0)
		memory_patch(0x00424FCA, &ccmu_extender.new_selGroupHp[2]);
		memory_patch(0x0042514E, &ccmu_extender.new_selGroupHp);

		//unit_stat_condition_building (0x00425180)
		memory_patch(0x00425187, &ccmu_extender.new_selGroupHp);

		//UnitStatCond_overlord (0x00425900)
		memory_patch(0x00425901, &ccmu_extender.new_selGroupHp);

		//sub_425960 (0x00425960) -- hooked
	//	memory_patch(0x004259C3, &ccmu_extender.new_client_selection_group);
	//	memory_patch(0x004259C9, &ccmu_extender.new_client_selection_group[GroupWireframesCap]);

		//UnitStatAct_Powerup (0x00426EE0)
		memory_patch(0x00426F1E, &ccmu_extender.new_selGroupHp);

		//unit_stat_act_standard (0x00426F50)
		memory_patch(0x00426F5E, &ccmu_extender.new_selGroupHp);

		//UnitStatAct_CarrierReaver (0x004274A0)
		memory_patch(0x004274CE, &ccmu_extender.new_selGroupHp);

		//unit_stat_action_building (0x00427890)
		//memory_patch(0x0042789D, &ccmu_extender.new_selGroupHp);

		//btnscond_worker_is_building (0x00428310)
		memory_patch(0x00428311, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00428327, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_LurkerStop (0x00428360)
		memory_patch(0x00428373, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004283A5, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_Movement (0x004283C0)
		memory_patch(0x004283C1, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004283DA, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_HasCargo (0x00428480)
		memory_patch(0x00428481, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00428499, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_NoCargo (0x004284B0)
		memory_patch(0x004284B1, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004284C9, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_HasCargoTerran (0x00428560)
		memory_patch(0x00428571, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00428587, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_NoCargoTerran (0x004285A0)
		memory_patch(0x004285B1, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004285C7, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_CanRepair (0x004285E0)
		memory_patch(0x004285E1, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004285F7, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_SCVCanAttack (0x00428610)
		memory_patch(0x00428611, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00428627, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_SCVCanStop (0x00428640)
		memory_patch(0x00428641, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00428657, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_SCVCanMove (0x00428670)
		memory_patch(0x00428671, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00428687, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_HasScarabs (0x004286A0)
		memory_patch(0x004286A1, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004286CA, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_HasInterceptors (0x004286E0)
		memory_patch(0x004286E1, &ccmu_extender.new_client_selection_group);
		memory_patch(0x0042870E, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_ZergHasCargo (0x00428730)
		memory_patch(0x00428734, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00428753, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_ZergNoCargo (0x00428780)
		memory_patch(0x00428784, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004287A3, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_TankMove (0x00428860)
		memory_patch(0x00428861, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00428880, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_TerranAdvanced (0x00428990)
		//memory_patch(0x004289A1, &ccmu_extender.new_client_selection_group);
		//memory_patch(0x004289B7, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_TerranBasic (0x00428A10)
		//memory_patch(0x00428A21, &ccmu_extender.new_client_selection_group);
		//memory_patch(0x00428A37, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//btnscond_can_move_special_case (0x00428D40)
		//memory_patch(0x00428D41, &ccmu_extender.new_client_selection_group);
		//memory_patch(0x00428D81, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//btnscond_can_move (0x00428DA0)
		//memory_patch(0x00428DA1, &ccmu_extender.new_client_selection_group);
		//memory_patch(0x00428DE7, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_TrainingFighter (0x00428E00)
		jump_patch(BTNSCOND_TrainingFighter_patch_28E20, 0x00428E1B);

		//BTNSCOND_HasUnit (0x00428EA0)
		memory_patch(0x00428EA2, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00428EBA, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_Stationary (0x00428ED0)
		memory_patch(0x00428ED2, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00428F13, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//btnscond_can_attack (0x00428F30)
		//memory_patch(0x00428F32, &ccmu_extender.new_client_selection_group);
		//memory_patch(0x00428F7E, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_BattleOrders (0x00428FA0)
		memory_patch(0x00428FA4, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00428FC3, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_HasRoom (0x00428FF0)
		memory_patch(0x00428FF3, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00429053, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_IsBurrowed (0x00429070)
		memory_patch(0x0042907D, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004290C3, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_CanBurrow (0x004290F0)
		memory_patch(0x00429100, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00429149, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_IsSieged (0x00429170)
		memory_patch(0x00429188, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004291AB, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_IsUnsieged (0x004291C0)
		memory_patch(0x004291D8, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004291FB, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_IsCloaked (0x00429210)
		memory_patch(0x0042921D, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00429298, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_CanCloak (0x004292C0)
		memory_patch(0x004292CD, &ccmu_extender.new_client_selection_group);
		memory_patch(0x0042934A, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_IsCloaked_0 (0x00429370)
		memory_patch(0x00429380, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004293BE, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_CanCloak_0 (0x004293E0)
		memory_patch(0x004293F0, &ccmu_extender.new_client_selection_group);
		memory_patch(0x00429430, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//BTNSCOND_HasSpidermines (0x00429470)
		memory_patch(0x00429486, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004294B3, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//show_right_click_error_if_needed (0x00455A00)
		//memory_patch(0x00455A07, &ccmu_extender.new_client_selection_group[SelectionLimit - 1]);
		//memory_patch(0x00455BBA, &ccmu_extender.new_client_selection_group);

		//CMDACT_RightClick (0x004563A0)
		//jump_patch(CMDACT_RightClick_patch_563D0, 0x004563D0, 2);

		//input_game_right_mouse_click (0x004564E0)
		//memory_patch(0x004564FD, &ccmu_extender.new_local_selection_group);
		//jump_patch(input_game_right_mouse_click_patch_56540, 0x00456540, 2);
		//jump_patch(input_game_right_mouse_click_patch_565C0, 0x004565BB);

		//stat_data_unit_wireframe_select (0x00456F50)
		jump_patch(stat_data_unit_wireframe_select_update_patch_57046, 0x00457046, 4);

		//statdataMouseOverInteract (0x00457CE0)
		jump_patch(statdataMouseOverInteract_patch_57046, 0x00457D87);

		//updateButtonSetEx (0x00458BC0)
		//memory_patch(0x00458BD7, &ccmu_extender.new_client_selection_group);
		//memory_patch(0x00458C8F, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//select_dual_birth_units (0x0045D040)
		//jump_patch(select_dual_birth_units_patch_5D041, 0x0045D041);

		//sort_all_units (0x0046F0F0)
		//jump_patch(sort_all_units_patch_6F206, 0x0046F206);
		//memory_patch(0x0046F1B3, (u32)0x754C468A);//MOV AL, [ESI + 0x4C]
		//memory_patch(0x0046F1B7, (u8)0x0D);//JNE 0x0046F1C5

		//combine_lists (0x0046F290)
		jump_patch(combine_lists_patch_6F2AD, 0x0046F2AD);
		jump_patch(combine_lists_patch_6F320, 0x0046F320);
		memory_patch(0x0046F2D9, (u8)0x2D);//JNE 0x0046F307
		jump_patch(combine_lists_patch_6F2D1, 0x0046F2D1);

		//do_targeted_order (0x0046F5B0)
		memory_patch(0x0046F657, &ccmu_extender.new_client_selection_group);
		memory_patch(0x0046F7E8, &ccmu_extender.new_client_selection_group[SelectionLimit]);

		//get_selected_units_in_box (0x0046FA40)
		//jump_patch(get_selected_units_in_box_patch_6FA41, 0x0046FA41);
		//memory_patch(0x0046FA98, &ccmu_extender.new_active_selection);
		//memory_patch(0x0046FAA3, SelectionLimit);
		//memory_patch(0x0046FAA8, &ccmu_extender.new_active_selection);
		//jump_patch(get_selected_units_in_box_patch_6FAAC, 0x0046FAAC, 2);
		//jump_patch(get_selected_units_in_box_patch_6FAD4, 0x0046FAD4);

		//get_selected_units_at_point (0x0046FB40)
		//jump_patch(get_selected_units_at_point_patch_6FB41, 0x0046FB41);
		//memory_patch(0x0046FB8F, SelectionLimit);
		//jump_patch(get_selected_units_at_point_patch_6FB98, 0x0046FB98);
		//memory_patch(0x0046FBAB, &ccmu_extender.new_active_selection);
		//memory_patch(0x0046FC91, SelectionLimit);
		//memory_patch(0x0046FC96, &ccmu_extender.new_active_selection);
		//jump_patch(get_selected_units_at_point_patch_6FC9A, 0x0046FC9A, 1);
		//jump_patch(get_selected_units_at_point_patch_6FCD6, 0x0046FCD6, 2);
		//memory_patch(0x0046FCF7, &ccmu_extender.new_active_selection);
		//jump_patch(get_selected_units_at_point_patch_6FD00, 0x0046FD00, 1);
		//jump_patch(get_selected_units_at_point_patch_6FD1B, 0x0046FD1B, 4);
		//jump_patch(get_selected_units_at_point_patch_6FD5F, 0x0046FD5F, 1);
		//jump_patch(get_selected_units_at_point_patch_6FD84, 0x0046FD84);
		//jump_patch(get_selected_units_at_point_patch_6FD9B, 0x0046FD9B);
		//jump_patch(get_selected_units_at_point_patch_6FDC3, 0x0046FDC3, 3);
		//jump_patch(get_selected_units_at_point_patch_6FE3C, 0x0046FE3C);
		//jump_patch(get_selected_units_at_point_patch_6FE65, 0x0046FE65);
		//call_patch(unit_is_owned_by_current_player_new, 0x0046FD35);
		//call_patch(unit_is_owned_by_current_player_new, 0x0046FD52);

		//command_save_hotkey_group (0x004965D0)
		jump_patch(command_save_hotkey_group_patch_965E1, 0x004965E1, 1);
		jump_patch(command_save_hotkey_group_patch_96653, 0x00496653, 1);
		memory_patch(0x004966AD, SelectionLimit);
		jump_patch(command_save_hotkey_group_patch_966CC, 0x004966CC, 1);

		//centerviewUnitGroup (0x004967E0)
		//jump_patch(centerviewUnitGroup_patch_967F7, 0x004967F7, 1);

		//add_to_control_group (0x00496940)
		jump_patch(add_to_control_group_patch_96952, 0x00496952, 1);
		jump_patch(add_to_control_group_patch_969D9, 0x004969D9, 2);

		//selectUnitGroup (0x00496B40)
		jump_patch(selectUnitGroup_patch_96B41, 0x00496B41);
		jump_patch(selectUnitGroup_patch_96BFA, 0x00496BFA);
		jump_patch(selectUnitGroup_patch_96CA3, 0x00496CA3, 3);
		jump_patch(selectUnitGroup_patch_96CE6, 0x00496CE6);

		//select_single_unit_from_id (0x00496D30)
		memory_patch(0x00496D43, sizeof(ccmu_extender.new_saved_selections[0]));
		memory_patch(0x00496D51, &ccmu_extender.new_saved_selections[1][0][1]);
		jump_patch(select_single_unit_from_id_patch_96D61, 0x00496D61, 1);

		//create_all_selection_circles (0x00499A60)
		jump_patch(create_all_selection_circles_patch_99A70, 0x00499A70, 2);
		jump_patch(create_all_selection_circles_patch_99B23, 0x00499B23, 1);

		//remove_from_selection_2 (0x0049A170)
		jump_patch(remove_from_selection_2_patch_9A185, 0x0049A185, 1);

		//is_selected_unit_group_enabled (0x0049A2C0)
		memory_patch(0x0049A2C7, &ccmu_extender.new_active_selection);
		memory_patch(0x0049A305, &ccmu_extender.new_active_selection[SelectionLimit]);

		//clear_selection (0x0049A740)
		jump_patch(clear_selection_patch_9A749, 0x0049A749);

		//get_active_player_first_selection (0x0049A850)
		jump_patch(get_active_player_first_selection_patch_9A857, 0x0049A857);

		//create_new_unit_selections_from_list (0x0049AE40)
		//memory_patch(0x0049AE49, &ccmu_extender.new_active_selection);
		//memory_patch(0x0049AE77, &ccmu_extender.new_active_selection[SelectionLimit]);
		//memory_patch(0x0049AE95, &ccmu_extender.new_active_selection);
		//memory_patch(0x0049AEB9, &ccmu_extender.new_active_selection);
		//memory_patch(0x0049AEC9, &ccmu_extender.new_active_selection);

		//add_to_player_selection (0x0049AF80)
		jump_patch(add_to_player_selection_patch_9AF89, 0x0049AF89);
		jump_patch(add_to_player_selection_patch_9AFB5, 0x0049AFB5, 1);

		//select_unit_initial (0x0049B690)
		jump_patch(select_unit_initial_patch_9B6A0, 0x0049B6A0, 2);
		memory_patch(0x0049B729, &ccmu_extender.new_players_selections);
		jump_patch(select_unit_initial_patch_9B770, 0x0049B76B);

		//remove_from_client_selection_3 (0x0049F7A0)
		jump_patch(remove_from_client_selection_3_patch_9F7A1, 0x0049F7A1);
		jump_patch(remove_from_client_selection_3_patch_9F7C0, 0x0049F7C0, 2);
		jump_patch(remove_from_client_selection_3_patch_9F7E6, 0x0049F7E6);
		jump_patch(remove_from_client_selection_3_patch_9F7FB, 0x0049F7FB, 2);

		//removeFromSelections (0x0049A7F0)
		//jump_patch(removeFromSelections_patch_BF8C0, 0x0049A839);
		//jump_patch(removeFromSelections_patch_BF8C0, 0x0049A846);

		//CMDRECV_ShiftDeselect (0x004BFB40)
		jump_patch(CMDRECV_ShiftDeselect_patch_BFB49, 0x004BFB49, 1);

		//CMDACT_HotkeyUnit (0x004C07B0)
		memory_patch(0x004C07DC, SelectionLimit);
		memory_patch(0x004C07E1, &ccmu_extender.new_local_selection_group);
		memory_patch(0x004C07F4, &ccmu_extender.new_local_selection_group);

		//cmdact_select (0x004C0860)
		jump_patch(cmdact_select_patch_C0863, 0x004C0863);
		jump_patch(cmdact_select_patch_C08E0, 0x004C08D8, 3);
		memory_patch(0x004C08FA, &ccmu_extender.new_local_selection_group);
		jump_patch(cmdact_select_patch_C0959, 0x004C0959);
		memory_patch(0x004C096F, &ccmu_extender.new_local_selection_group);
		memory_patch(0x004C0977, SelectionLimit);
		jump_patch(cmdact_select_patch_C09E6, 0x004C09E6);
		memory_patch(0x004C09FE, SelectionLimit);
		memory_patch(0x004C0A03, &ccmu_extender.new_local_selection_group);
		memory_patch(0x004C0A11, &ccmu_extender.new_local_selection_group);
		jump_patch(cmdact_select_patch_C0A67, 0x004C0A67);
		jump_patch(cmdact_select_patch_C0A71, 0x004C0A71, 2);
		jump_patch(cmdact_select_patch_C0A8D, 0x004C0A8D, 2);

		//cmdrecv_shift_select (0x004C2560)
		jump_patch(cmdrecv_shift_select_patch_C256D, 0x004C256D, 4);
		jump_patch(cmdrecv_shift_select_patch_C2650, 0x004C2650);
		jump_patch(cmdrecv_shift_select_patch_C2685, 0x004C2685);
		jump_patch(cmdrecv_shift_select_patch_C26AB, 0x004C26AB);

		//cmdrecv_select (0x004C2750)
		jump_patch(cmdrecv_select_patch_C2757, 0x004C2757, 2);
		jump_patch(cmdrecv_select_patch_C27D3, 0x004C27D3, 1);

		//cmdrecv_save_game (0x004C2910)
		memory_patch(0x004C2D1E, sizeof(ccmu_extender.new_players_selections));
		memory_patch(0x004C2D23, &ccmu_extender.new_players_selections);

		//update_selected_unit_data (0x004C38B0)
		memory_patch(0x004C38B4, SelectionLimit);
		memory_patch(0x004C38B9, &ccmu_extender.new_active_selection);
		memory_patch(0x004C38BE, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004C38D4, &ccmu_extender.new_client_selection_group);
		memory_patch(0x004C38F4, &ccmu_extender.new_client_selection_group[SelectionLimit]);
		memory_patch(0x004C3911, &ccmu_extender.new_client_selection_group[1]);
		memory_patch(0x004C3917, &ccmu_extender.new_client_selection_group);

		//update_building_land_unit_selection (0x004C3B40)
		jump_patch(update_building_land_unit_selection_patch_C3B41, 0x004C3B41);
		memory_patch(0x004C3B4A, &ccmu_extender.new_active_selection);
		jump_patch(update_building_land_unit_selection_patch_C3B5A, 0x004C3B5A);
		memory_patch(0x004C3B64, &ccmu_extender.new_active_selection[SelectionLimit]);
		jump_patch(update_building_land_unit_selection_patch_C3B6B, 0x004C3B6B, 3);

		//setup_HUD (0x004C3BB0)
		memory_patch(0x004C3C38, SelectionLimit);
		memory_patch(0x004C3C3D, &ccmu_extender.new_client_selection_group);

		//unpack_all_player_selection_groups (0x004CEDA0)
		memory_patch(0x004CEDA3, &ccmu_extender.new_players_selections);
		memory_patch(0x004CEDA8, sizeof(ccmu_extender.new_players_selections) / 4);

		//pack_all_player_selection_groups (0x004CEE00)
		memory_patch(0x004CEE04, &ccmu_extender.new_players_selections);
		memory_patch(0x004CEE09, sizeof(ccmu_extender.new_players_selections) / 4);

		//load_game_full (0x004CFEF0)
		memory_patch(0x004D013A, sizeof(ccmu_extender.new_players_selections));
		memory_patch(0x004D013F, &ccmu_extender.new_players_selections);

		//compile_health_bar (0x004D6010)
		memory_patch(0x004D603F, &ccmu_extender.new_active_selection);

		//sub_4D68C0 (0x004D68C0)
		memory_patch(0x004D68CA, &ccmu_extender.new_hp_bar_grps);

		//InitializeImageArray (0x004D6930)
		memory_patch(0x004D693F, &ccmu_extender.new_selection_hp_bars);
		memory_patch(0x004D6952, sizeof(ccmu_extender.new_selection_hp_bars) / 4);
		memory_patch(0x004D6959, sizeof(ccmu_extender.new_selection_circles) / 4);
		memory_patch(0x004D695E, &ccmu_extender.new_selection_circles);
		memory_patch(0x004D697D, &ccmu_extender.new_selection_hp_bars);
		memory_patch(0x004D6998, &ccmu_extender.new_selection_hp_bars[1]);
		jump_patch(InitializeImageArray_patch_D69A0, 0x004D69A0, 5);
		memory_patch(0x004D69E4, &ccmu_extender.new_selection_circles);
		memory_patch(0x004D69F3, &ccmu_extender.new_selection_circles[1]);
		memory_patch(0x004D69F8, SelectionLimit - 1);
		memory_patch(0x004D69FD, &ccmu_extender.new_selection_circles);
		memory_patch(0x004D6A12, &ccmu_extender.new_selection_circles[0].link.next);
		memory_patch(0x004D6A1B, &ccmu_extender.new_selection_circles[0].link.next);
		memory_patch(0x004D6A26, &ccmu_extender.new_selection_circles[0].link.next);

		//StatRes_MainProc (0x004E5640)
		//memory_patch(0x004E5692, &ccmu_extender.new_client_selection_group);

		//hotkeyRemapping (0x004EEC30)
		memory_patch(0x004EEC74, sizeof(ccmu_extender.new_saved_selections) / 4);
		memory_patch(0x004EEC79, &ccmu_extender.new_saved_selections);

		//load_game_core (0x004EED10)
		memory_patch(0x004EED14, SelectionLimit);
		memory_patch(0x004EED19, &ccmu_extender.new_active_selection);
		memory_patch(0x004EED20, sizeof(ccmu_extender.new_players_selections) / 4);
		memory_patch(0x004EED25, &ccmu_extender.new_players_selections);
		memory_patch(0x004EED2C, &ccmu_extender.new_local_selection_group);
		memory_patch(0x004EED31, &ccmu_extender.new_local_selection_group[1]);
		memory_patch(0x004EED36, &ccmu_extender.new_local_selection_group[2]);
		jump_patch(load_game_core_patch_EEDCB, 0x004EEDCB, 1);
		memory_patch(0x004EEDDD, &ccmu_extender.new_active_selection);
		memory_patch(0x004EEDE6, sizeof(ccmu_extender.new_players_selections) / 4);
		memory_patch(0x004EEDEB, &ccmu_extender.new_players_selections);
	}
}