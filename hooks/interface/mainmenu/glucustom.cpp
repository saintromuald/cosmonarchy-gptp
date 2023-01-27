#include "glucustom.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include "hook_tools.h"

namespace hooks
{
	void display_establishing_shot() // 004D9200, hooked by X405
	{
		// This is the original original function which loads the establishing shots in campaigns:
		if (!*g_multiplayer_mode && !(*g_cheat_state & CheatFlags::NoGlues) && *campaign_index)
		{
			for (s32 i = 0; i < 65; i++)
			{
				if (campaignMissions[i].mission_index == *campaign_index)
				{
					helper::load_init_credits_bin(campaignMissions[i].mission_name);
				}
			}
		}

		// This addition enables establishing shots in custom scenarios:
		else if (*campaign_index == 0 && current_map_filename)
		{
			helper::SFileOpenArchive(current_map_filename, 0, 0, map_archive_handle);
			void* handle;
			bool establishingShotExists = helper::SFileOpenFileEx(*map_archive_handle, "rez\\est.txt", 0, &handle);
			if (handle)
			{
				helper::SFileCloseFile(handle);
			}

			if (establishingShotExists)
			{
				helper::load_init_credits_bin("est");
			}

			if (*map_archive_handle)
			{
				helper::SFileCloseArchive(*map_archive_handle);
			}
		}
	}

	// Hooked to fix crash after removing game speed settings. -Pr0nogo
	void cmdact_game_speed(BinDlg* control) // 00481EF0
	{
		BinDlg* control_var = control;
		if (*last_control_index == -2)
		{
			if (control->control_type)
			{
				control = control->parent;
			}
			BinDlg* child = control->child_dialog;
			if (child)
			{
				while (child->index != 1)
				{
					child = child->next;
					if (!child)
					{
						goto LABEL_7;
					}
				}
			}
			else
			{
			LABEL_7:
				child = 0;
			}
			if (child && !(child->flags & 2))
			{
				char min_value = child->scrollDialog0x36.scrollMinValue;
				u8 game_command = 15;
				helper::queue_game_command(&game_command, 2u);
			}
		}
		else
		{
			*cpu_throttle = *temp_cpu_throttle;
		}
		helper::process_options_change(control_var);
		char interacting_bin = (*bin_interact)-- - 1;
		if (*bin_interact)
		{
			*bin_interact = interacting_bin - 1;
			*last_bindlg_menu_function = helper::main_menu_options_custom_interact;
			helper::load_dialog("rez\\options.bin", helper::game_menu_dialog_interact);
		}
		else
		{
			helper::destroy_dialog(control_var);
		}
	}

	// Hooked to fix crash after removing save and load dialogs.
	// Currently identical to IDA. -Pr0nogo
	void hide_dialog(BinDlg* dialog) // 00418700
	{
		if (dialog->flags & 8)
		{
			DialogEvent dlg_event = {};
			dlg_event.current.y = g_mouse->y;
			dlg_event.number_order = 14;
			dlg_event.user = 14;
			dlg_event.field_4 = 0;
			dlg_event.current.x = g_mouse->x;
			if (((u32(__fastcall*)(BinDlg*, DialogEvent*, s32, s32))dialog->fxn_interact)(dialog, &dlg_event, 14, 0))
			{
				dlg_event.current.x = g_mouse->x;
				dlg_event.number_order = 14;
				dlg_event.user = 6;
				dlg_event.field_4 = 0;
				dlg_event.current.y = g_mouse->y;
				((void(__fastcall*)(BinDlg*, DialogEvent*, s32, s32))dialog->fxn_interact)(dialog, &dlg_event, 6, 0);
				u32 dialog_flags = dialog->flags;
				if (!(dialog_flags & 1))
				{
					dialog->flags = dialog_flags | 1; // CTRL_UPDATE
					helper::update_dialog(dialog);
				}
			}
		}
	}

	// DarkenedFantasies - fix score screen bugs with extended color IDs \\

	void __declspec(naked) sub_41D160() {
		__asm {
			PUSH    EBP
			MOV     EBP, ESP
			PUSH    ESI
			MOVZX	SI, AL/* this seems to be no longer needed
			CMP     SI, [ECX]
			JB      short loc_41D16F
			XOR		AL, AL

		loc_41D16F :*/
			MOVZX   EAX, AX//formerly AL
			MOVZX   SI, byte ptr[ECX + EAX * 0x8 + 0x6]
			LEA     EAX, [ECX + EAX * 0x8 + 0x6]
			PUSH    EDI
			MOV     EDI, [EBP + 0x8]
			MOV		[EDI], SI
			MOVZX   SI, byte ptr[EAX + 0x1]
			MOV     EDI, [EBP + 0xC]
			MOV		[EDI], SI
			MOV     SI, [ECX + 0x2]
			MOV     EDI, [EBP + 0x10]
			MOV		[EDI], SI
			MOV     SI, [ECX + 0x4]
			MOV     EDI, [EBP + 0x14]
			MOV		[EDI], SI
			MOVZX   SI, byte ptr[EAX + 0x3]
			MOV		[EDX + 0x2], SI
			MOVZX   SI, byte ptr[EAX + 0x2]
			MOV     EAX, [EAX + 0x4]
			TEST    EAX, EAX
			POP     EDI
			JNS     short loc_41D1C3
			AND		EAX, 0x7FFFFFFF
			ADD     ESI, 0x100

		loc_41D1C3 :
			ADD     EAX, ECX
			MOV		[EDX], SI
			MOV		[EDX + 0x4], EAX
			POP     ESI
			POP     EBP
			RETN    0x10
		}
	}

	const u32 DrawDialogImage = 0x004EF440;
	void __declspec(naked) sub_4EF600() {
		__asm {
			PUSH    EBP
			MOV     EBP, ESP
			SUB     ESP, 0x18
			PUSH    EDI
			LEA     EAX, [EBP - 0x4]
			PUSH    EAX
			LEA     ECX, [EBP + 0xA]
			PUSH    ECX
			MOV		ECX, ds: [0x006D5E30]
			LEA     EDX, [EBP - 0xC]
			PUSH    EDX
			LEA     EAX, [EBP - 0x8]
			PUSH    EAX
			MOV     AX, [EBP + 0x8]//formerly AL
			LEA     EDX, [EBP - 0x18]
			CALL    sub_41D160
			MOV     ECX, [EBP + 0xC]
			MOV     EDX, [EBP - 0x8]
			MOV     EAX, [EBP + 0x10]
			MOV     EDI, [EBP - 0xC]
			ADD     EDX, ECX
			ADD     EDI, EAX
			MOVSX   EAX, word ptr[ESI]
			MOVZX   ECX, DX
			CMP     ECX, EAX
			MOV		[EBP - 0x10], CX
			JG      short loc_4EF64A
			MOV		[EBP - 0x10], AX

		loc_4EF64A :
			MOVSX   EAX, word ptr[ESI + 0x2]
			PUSH    EBX
			MOVZX   EBX, DI
			CMP     EBX, EAX
			MOV		[EBP - 0xE], BX
			JG      short loc_4EF65E
			MOV		[EBP - 0xE], AX

		loc_4EF65E :
			MOVZX   EAX, word ptr[EBP + 0xA]
			ADD     EAX, ECX
			MOVSX   ECX, word ptr[ESI + 0x4]
			CMP     EAX, ECX
			MOV     word ptr[EBP - 0xC], AX
			JL      short loc_4EF674
			MOV     word ptr[EBP - 0xC], CX

		loc_4EF674 :
			MOVZX   ECX, word ptr[EBP - 0x4]
			LEA     EAX, [ECX + EBX]
			MOVSX   ECX, word ptr[ESI + 0x6]
			CMP     EAX, ECX
			POP     EBX
			MOV     word ptr[EBP - 0xC + 0x2], AX
			JL      short loc_4EF68C
			MOV     word ptr[EBP - 0xC + 0x2], CX

		loc_4EF68C :
			MOVSX   EAX, word ptr[EBP - 0xE]
			MOVSX   ECX, DI
			SUB     EAX, ECX
			MOV     ECX, 0x0
			SETS    CL
			MOVSX   EDX, DX
			DEC     ECX
			AND		ECX, EAX
			MOVSX   EAX, word ptr[EBP - 0x10]
			SUB     EAX, EDX
			MOV     EDX, 0x0
			SETS    DL
			DEC     EDX
			AND		EAX, EDX
			MOV     EDX, [EBP + 0x14]
			PUSH    EDX
			PUSH    ECX
			PUSH    EAX
			LEA     EAX, [EBP - 0x18]
			PUSH    EAX
			LEA     ECX, [EBP - 0x10]
			CALL	DrawDialogImage
			POP     EDI
			MOV     ESP, EBP
			POP     EBP
			RETN    0x10
		}
	}

	void __declspec(naked) sub_4EF710() {
		__asm {
			PUSH    EBP
			MOV     EBP, ESP
			SUB     ESP, 0x10
			MOV     EDX, [EBP + 0x10]
			PUSH    EBX
			MOV     EBX, ECX
			MOV     ECX, [EAX]
			MOV     EAX, [EAX + 0x4]
			PUSH    ESI
			MOV		[EBP - 0xC], EAX
			MOV     EAX, [EBP + 0x14]
			ADD     EDX, ECX
			PUSH    EAX
			MOV		[EBP - 0x10], ECX
			MOV     ECX, [EBP + 0xC]
			MOV     word ptr[EBP - 0xC], DX
			MOV     EDX, [EBP + 0x8]
			PUSH    ECX
			PUSH    EDX
			PUSH    EBX
			LEA     ESI, [EBP - 0x10]
			CALL    sub_4EF600
			MOV     ECX, [EBP + 0x8]
			MOV     ESI, [EBP + 0x10]
			LEA     EAX, [ECX + EDI]
			MOV		[EBP - 0x8], EAX
			MOV     EAX, ESI
			SUB     EAX, EDI
			INC     BX // formerly BL
			DEC     EAX
			CDQ
			IDIV    EDI
			SUB     word ptr[EBP - 0xC], DI
			MOV		[EBP - 0x4], BX	// formerly BL
			TEST    EAX, EAX
			JLE     short loc_4EF78F
			MOV     EBX, EAX

		loc_4EF766 :
			MOV     EAX, [EBP + 0x14]
			MOV     ECX, [EBP + 0xC]
			MOV     EDX, [EBP - 0x8]
			PUSH    EAX
			MOV     EAX, dword ptr[EBP - 0x4]
			PUSH    ECX
			PUSH    EDX
			PUSH    EAX
			LEA     ESI, [EBP - 0x10]
			CALL    sub_4EF600
			MOV     ECX, [EBP - 0x8]
			ADD     ECX, EDI
			DEC     EBX
			MOV		[EBP - 0x8], ECX
			JNZ     short loc_4EF766
			MOV     ECX, [EBP + 0x8]
			MOV     ESI, [EBP + 0x10]

		loc_4EF78F :
			MOV     DX, [EBP - 0x4] // formerly DL
			ADD     word ptr[EBP - 0xC], DI
			INC     DX // formerly DL
			CMP     ESI, EDI
			MOV		[EBP - 0x4], DX // formerly DL
			JL      short loc_4EF7CB
			MOVSX   EDX, word ptr[EBP - 0x10]
			LEA     EAX, [ECX + EDI]
			CMP     EDX, EAX
			MOV     word ptr[EBP - 0x10], DX
			JG      short loc_4EF7B2
			MOV     word ptr[EBP - 0x10], AX

		loc_4EF7B2 :
			MOV     EDX, [EBP + 0x14]
			MOV     EAX, [EBP + 0xC]
			PUSH    EDX
			SUB     ECX, EDI
			ADD     ECX, ESI
			PUSH    EAX
			PUSH    ECX
			MOV     ECX, dword ptr[EBP - 0x4]
			PUSH    ECX
			LEA     ESI, [EBP - 0x10]
			CALL    sub_4EF600

		loc_4EF7CB :
			POP     ESI
			POP     EBX
			MOV     ESP, EBP
			POP     EBP
			RETN    0x10
		}
	}

	const char pIcons_grp[] = "pIcons.grp";
	const u32 LoadGraphic = 0x004D2B30;
	const u32 loadMenu_gluScore = 0x004B4B92;
	void __declspec(naked) loadMenu_gluScore_pIcons() {
		__asm {
			LOC_4B4B80_pIcons:
				MOV		CL, [EAX + 0x0059B628]
				MOV		[EBP + EAX - 0x10C], CL
				INC		EAX
				TEST	CL, CL
				JNZ		short LOC_4B4B80_pIcons
				LEA		EDI, [EBP - 0x10C]
				DEC		EDI
				LEA		ESP, [ESP]	// For some reason this opcode had an offset of +0x0.

			LOC_4B4BA0 :
				MOV		AL, [EDI + 0x1]
				INC		EDI
				TEST	AL, AL
				JNZ		short LOC_4B4BA0
				MOV		EDX, dword ptr ds : pIcons_grp
				MOV		EAX, dword ptr ds : pIcons_grp + 0x4
				MOV		CX, word ptr ds : pIcons_grp + 0x8
				MOV		[EDI], EDX
			//	MOV		DL, byte ptr ds : aIscore_grp + 0xA
				MOV		[EDI + 0x4], EAX
				MOV		[EDI + 0x8], CX
				PUSH	0x560
				MOV		[EDI + 0xA], 0x0	// formerly DL, would grab the string's null terminator from the above commented-out line.
				PUSH	0x005034D8			// PUSH offset 0x005034D8//"Starcraft\SWAR\lang\gluScore.cpp"
				XOR		EDX, EDX
				LEA		ECX, [EBP - 0x10C]
				CALL	LoadGraphic
				MOV		ds : [0x0059B72C] , EAX
				XOR		EAX, EAX

			LOC_4B4B80_iScore :
				MOV		CL, [EAX + 0x0059B628]
				MOV		[EBP + EAX - 0x10C], CL
				INC		EAX
				TEST	CL, CL
				JNZ		short LOC_4B4B80_iScore
				JMP		loadMenu_gluScore
		}
	}

	void __declspec(naked) get_color_handle() {
		__asm {
			CMP     AL, 0x1
			JNE     offset isNewColor
			CMP     dword ptr ds : [0x006CE0E8] , offset tFontGame_ImageData + 0x130//Use saved font pColor if savedFontColor is also pColor.
			JNE     offset skip_0
			ADD     dword ptr ds : [0x006CE0E8] , 8
			skip_0 :
			MOV     EAX, ds : [0x006CE0E8]//<Set savedFontColor to currentFontColor if <01>.
			MOV     ds : [0x006CE0C0] , EAX
			RETN

			isNewColor :
			CMP     AL, 0x0B
				JE      offset isPlayerColor_0
				CMP     AL, 0x14
				JE      offset isPlayerColor_0
				MOV     ECX, ds : [0x006CE0C0]//Save currentFontColor to savedFontColor.
				MOV     ds : [0x006CE0E8] , ECX
				MOVZX   EAX, AL
				DEC     AL
				CMP     AL, 0xA//No tfont offset adjustment for <02> through <0A> tags.
				JB      offset off_2
				CMP     AL, 0x1F//tfont offsets for <0B> through <1F> tags.
				JB      offset off_1
				SUB     AL, 0x77//tfont offsets for <8F> through <9F> tags.
				CMP     AL, 0x22//Decrement 1 more if above <99>.
				JB      offset off_2
				DEC     AL
				JMP     offset off_2

				off_1 :
			SUB     AL, ds : [EAX + 0x004200AC]

				off_2 :
				DEC     AL
				LEA     EAX, [EAX * 8 + tFontGame_ImageData]//Set new font color to currentFontColor.
				MOV     ds : [0x006CE0C0] , EAX
				RETN

				isPlayerColor_0 :
			MOV     CL, [ESI + 1]
				CMP     CL, 0x08
				JBE     offset isPlayerColor_2
				SUB     CL, 0x0E
				CMP     CL, 0x03
				JBE     offset isPlayerColor_1
				ADD[ESP], 6
				RETN

				isPlayerColor_1 :
			ADD     CL, 0x09

				isPlayerColor_2 :
				INC     ESI
				MOV     AL, CL
				DEC     AL
				MOV     ECX, ds : [0x006CE0C0]//Save currentFontColor to savedFontColor.
				MOV     ds : [0x006CE0E8] , ECX
				MOV     EDX, offset tFontGame_ImageData + 0x130//Set player-colored font to currentFontColor.
				MOV     ds : [0x006CE0C0] , EDX
				MOVZX   EAX, AL
				LEA     EAX, [EAX * 8 + 0x00581D74]//Unit color table
				MOV     EBX, [EDX + 1]
				MOV[EDX + 9], EBX

				loop_0 :
			ADD     EAX, 2
				INC     EDX
				MOV     BL, [EAX]
				MOV[EDX], BL
				CMP     EDX, offset tFontGame_ImageData + 0x134
				JNE     offset loop_0
				RETN

		}
	}

	const u32 writeFontApplyColour_1FE01 = 0x0041FE01;
	const u32 writeFontApplyColour_2000A = 0x0042000A;
	void __declspec(naked) writeFontApplyColour_patch() {
		__asm {
			//CL is -1 from actual character ASCII code to naturally exclude <00>.
			CMP     CL, 0x1F//Include <01> to <1F>.
			JB      offset off_3
			CMP     CL, 0x9E//Exclude <A0> to <FF>.
			JA      offset off_1FE01
			CMP     CL, 0x8E//Exclude <20> to <8E>.
			JB      offset off_1FE01
			CMP     CL, 0x98//Exclude <99>.
			JE      offset off_1FE01
			JMP     offset off_4

			off_3 :
			CMP     byte ptr ds : [ECX + 0x004200AC] , 0x1//Exclude the non-color tags (newlines, aligns, etc).
				JE      offset off_1FE01
				MOV     CL, ds : [0x006D5DE0]//dword_6D5DE0
				TEST    CL, CL
				JNE     offset off_2000A
				CMP     dword ptr ds : [0x006CE0C0] , offset tFontGame_ImageData[0x18]//Make grey color override subsequent tags.
				JE      off_2000A

				off_4 :
			CALL    get_color_handle

				off_2000A :
			JMP     writeFontApplyColour_2000A

				off_1FE01 :
			JMP     writeFontApplyColour_1FE01
		}
	}

	const u32 StatHangarCount_25477 = 0x00425477;
	void __declspec(naked) StatHangarCount_patch_25471() {
		__asm {
			PUSH    EDX
			XOR     BL, BL
			MOV     EDX, ds: [0x00597248]//activePortraitUnit
			MOV     EAX, [EDX + 0x64]//unit->id
			CMP     AX, 0x48//Carrier
			JE      offset loc_isCarrier
			CMP     AX, 0x52//Gantrithor
			JNE     offset loc_notCarrier
			loc_isCarrier :
			ADD     BL, [EDX + 0xC9]//unit->inHangarCount
				loc_notCarrier :
				ADD     BL, [EDX + 0xC8]//unit->outHangarCount
				MOV     AX, 0x26
				CMP     BL, 0xA
				JAE     offset dont_dec
				DEC     AX
				dont_dec :
			MOV     word ptr[ESI + 0x24], AX
				POP     EDX
				JMP     StatHangarCount_25477
		}
	}
	
	const u32 load_dialog = 0x004F57A0;
	void __declspec(naked) RestartGameMenu() {
		__asm {
			MOV		dword ptr ds : [0x006D1234] , 0x004C9530//last_bindlg_menu_function = CMDACT_RestartGame
			MOV		EDX, 0x004CA240//game_menu_dialog_interact
			MOV		ECX, 0x0050294C//"rez\restart.bin"
			JMP		load_dialog
		}
	}

	const u32 sub_490300_903D2 = 0x004903D2;
	void __declspec(naked) sub_490300_patch_903CA() {
		__asm {
			LEA		EDX, [ECX * 4 + 0x0057EEE9]//playerTable[playerId].race
			CMP		byte ptr[EDX], 7//Check if race is Inactive.
			JE		offset off_0
			DEC		EDX//playerTable[playerId].type
			CMP		byte ptr[EDX], 3//Check if type is neither Human or Computer (Players of type 0 (Unused) have already been filtered out).
			JBE		offset off_0//Zero-flag is naturally set if the previous CMP is equal, so don't need to CMP again to set it.
			CMP		EDX, EDX//Set zero-flag if player type is 4+.

			off_0 :
			JMP sub_490300_903D2
		}
	};

	void __declspec(naked) sub_4AABB0_new() {
		__asm {
			MOV		EAX, ds: [0x0057F0B4]//g_multiplayer_mode
			TEST	AL, AL
			JE		offset off_1
			MOV		CL, ds : [0x00596874]//game_data.bAlliesEnabled
			CMP		CL, AL
			JNE		offset off_0
			MOV		CL, ds : [0x00596877]//game_data.tournamentModeEnabled
			TEST	CL, CL
			JE		offset off_1

			off_0 :
			XOR		EAX, EAX

				off_1 :
			RETN
		}
	}

	const u32 sub_490E90_90EC8 = 0x00490EC8;
	void __declspec(naked) sub_490E90_patch_90EC3() {
		__asm {
			MOV		EAX, ds: [0x0057F0B4]//g_multiplayer_mode
			OR		EAX, ds : [0x006D0F14]//g_is_in_replay
			SHL		EAX, 3
			OR		EAX, 0x20000000
			PUSH	EAX
			JMP		sub_490E90_90EC8
		}
	}

	void __declspec(naked) UpdateOKButton2() {
		__asm {
			PUSH	EBP
			MOV		EBP, ESP
			CMP		word ptr[EAX + 0x22], 0
			JE		offset off_0
			MOV		EAX, [EAX + 0x32]

			off_0:
			MOV		EAX, [EAX + 0x42]
				TEST	EAX, EAX
				MOV		ECX, 0xFFFFFFFC//button id

				off_1 :
				CMP[EAX + 0x20], CX
				JE		offset off_2
				MOV		EAX, [EAX]
				TEST	EAX, EAX
				JNE		offset off_1
				POP		EBP
				RETN	8

				off_2:
			MOVZX	ECX, word ptr[EBP + 8]
				MOV		DX, ds : [ECX * 2 + 0x00512768]//OKButtonGraphicIndex
				MOV		ECX, [EBP + 0xC]
				MOV[EAX + 0x24], DX
				OR[EAX + 0x18], ECX
				POP		EBP
				RETN	8
		}
	}

	const char minimap_sp[] = "rez\\minimap_sp.bin";
	void __declspec(naked) minimap_style() {
		__asm {
			PUSH[ESP]//Move the return address up the stack.
			MOV		AL, ds: [0x006D0F14]//g_is_in_replay
			TEST	AL, AL
			JNE		offset off_0
			MOV		AL, ds : [0x0057F0B4]//g_multiplayer_mode
			TEST	AL, AL
			JNE		offset off_0
			MOV		dword ptr[ESP + 4], offset minimap_sp//Store "minimap_sp.bin" address in the previous stack position.
			RET

			off_0 :
			MOV		dword ptr[ESP + 4], 0x00504070//Store "minimap.bin" address in the previous stack position.
				RET
		}
	}

	const u32 minimap_dlg_Create_A526E = 0x004A526E;
	const u32 GetActivePlayerCount = 0x004C40F0;
	void __declspec(naked) minimap_dlg_Create_patch_A5242() {
		__asm {
			MOV		ECX, ds: [0x006D0F14]//g_is_in_replay
			TEST	ECX, ECX
			MOV		CL, 6
			JNE		offset off_0
			MOV		AL, ds : [0x0057F0B4]//g_multiplayer_mode
			SHL		EAX, CL
			XOR		CL, CL

			off_0 :
			MOV		DX, ds : [ECX * 2 + 0x0051276A]//OKButtonGraphicIndex
				ADD		EDX, EAX
				MOV[ESI + 0x24], DX
				CALL	GetActivePlayerCount
				MOV		ECX, ds : [0x0057F0B4]//g_multiplayer_mode
				XOR		EAX, ECX
				JMP		minimap_dlg_Create_A526E
		}
	}
}