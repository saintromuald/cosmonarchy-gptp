#include "glucustom.h"
#include <BW/api.h>
#include <BW/structures.h>
#include <BW/bwdata.h>
#include "hook_tools.h"

namespace hooks
{
	void display_establishing_shot() // 004D9200, hooked by X405
	{
		// This is the original original function which loads the establishing shots in campaigns
		if (!*g_multiplayer_mode && !(*g_cheat_state & CheatFlags::NoGlues) && *campaign_index) {
			for (s32 i = 0; i < 65; i++)
			{
				if (campaignMissions[i].mission_index == *campaign_index) {
					helper::load_init_credits_bin(campaignMissions[i].mission_name);
				}
			}
		} else if (*campaign_index == 0 && g_current_map_filename) { // This addition enables establishing shots in custom scenarios
			helper::SFileOpenArchive(g_current_map_filename, 0, 0, map_archive_handle);
			HANDLE* handle;
			bool shot_exists = helper::sfile_open_file_ex(*map_archive_handle, "rez\\est.txt", 0, &handle);
			if (handle) {
				helper::SFileCloseFile(handle);
			}
			if (shot_exists) {
				helper::load_init_credits_bin("est");
			}
			if (*map_archive_handle) {
				helper::SFileCloseArchive(*map_archive_handle);
			}
		}
	}

	// Hooked to fix crash after removing game speed settings. -Pr0nogo
	void cmdact_game_speed(BinDlg* control) // 00481EF0
	{
		helper::process_options_change(control);
		char interacting_bin = (*bin_interact)-- - 1;
		if (*bin_interact) {
			*bin_interact = interacting_bin - 1;
			*last_bindlg_menu_function = helper::main_menu_options_custom_interact;
			helper::load_dialog("rez\\options.bin", helper::game_menu_dialog_interact);
		} else {
			helper::destroy_dialog(control);
		}
	}

	// Hooked to fix crash after removing save and load dialogs.
	// Currently identical to IDA. -Pr0nogo
	void hide_dialog(BinDlg* dialog) // 00418700
	{
		if (dialog->flags & BinDlgFlags::Visible) {
			DialogEvent dlg_event = {};
			dlg_event.current.y = g_mouse->y;
			dlg_event.number_order = 14;
			dlg_event.user = 14;
			dlg_event.field_4 = 0;
			dlg_event.current.x = g_mouse->x;
			if (((u32(__fastcall*)(BinDlg*, DialogEvent*, s32, s32))dialog->fxn_interact)(dialog, &dlg_event, 14, 0)) {
				dlg_event.current.x = g_mouse->x;
				dlg_event.number_order = 14;
				dlg_event.user = 6;
				dlg_event.field_4 = 0;
				dlg_event.current.y = g_mouse->y;
				((void(__fastcall*)(BinDlg*, DialogEvent*, s32, s32))dialog->fxn_interact)(dialog, &dlg_event, 6, 0);
				u32 dialog_flags = dialog->flags;
				if (!(dialog_flags & BinDlgFlags::Highlighted)) {
					dialog->flags = dialog_flags | BinDlgFlags::Highlighted;
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

		loc_41D16F: */
			MOVZX   EAX, AX // formerly AL
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

		loc_41D1C3:
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
			MOV     AX, [EBP + 0x8] // formerly AL
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

		loc_4EF64A:
			MOVSX   EAX, word ptr[ESI + 0x2]
			PUSH    EBX
			MOVZX   EBX, DI
			CMP     EBX, EAX
			MOV		[EBP - 0xE], BX
			JG      short loc_4EF65E
			MOV		[EBP - 0xE], AX

		loc_4EF65E:
			MOVZX   EAX, word ptr[EBP + 0xA]
			ADD     EAX, ECX
			MOVSX   ECX, word ptr[ESI + 0x4]
			CMP     EAX, ECX
			MOV     word ptr[EBP - 0xC], AX
			JL      short loc_4EF674
			MOV     word ptr[EBP - 0xC], CX

		loc_4EF674:
			MOVZX   ECX, word ptr[EBP - 0x4]
			LEA     EAX, [ECX + EBX]
			MOVSX   ECX, word ptr[ESI + 0x6]
			CMP     EAX, ECX
			POP     EBX
			MOV     word ptr[EBP - 0xC + 0x2], AX
			JL      short loc_4EF68C
			MOV     word ptr[EBP - 0xC + 0x2], CX

		loc_4EF68C:
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

		loc_4EF766:
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

		loc_4EF78F:
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

		loc_4EF7B2:
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

		loc_4EF7CB:
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

			LOC_4B4BA0:
				MOV		AL, [EDI + 0x1]
				INC		EDI
				TEST	AL, AL
				JNZ		short LOC_4B4BA0
				MOV		EDX, dword ptr ds: pIcons_grp
				MOV		EAX, dword ptr ds: pIcons_grp + 0x4
				MOV		CX, word ptr ds: pIcons_grp + 0x8
				MOV		[EDI], EDX
			 // 	MOV		DL, byte ptr ds: aIscore_grp + 0xA
				MOV		[EDI + 0x4], EAX
				MOV		[EDI + 0x8], CX
				PUSH	0x560
				MOV		[EDI + 0xA], 0x0	// formerly DL, would grab the string's null terminator from the above commented-out line.
				PUSH	0x005034D8			// PUSH offset 0x005034D8 // "Starcraft\SWAR\lang\gluScore.cpp"
				XOR		EDX, EDX
				LEA		ECX, [EBP - 0x10C]
				CALL	LoadGraphic
				MOV		ds: [0x0059B72C], EAX
				XOR		EAX, EAX

			LOC_4B4B80_iScore:
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
			CMP		AL, 0x1
			JNE		offset isNewColor
			CMP		dword ptr ds: [0x006CE0E8], offset tFontGame_ImageData + 0x130 // Use saved font pColor if savedFontColor is also pColor.
			JNE		offset skip_0
			ADD		dword ptr ds: [0x006CE0E8], 8
			skip_0:
			MOV		EAX, ds: [0x006CE0E8] // <Set savedFontColor to currentFontColor if <01>.
			MOV		ds: [0x006CE0C0], EAX
			RETN

		isNewColor:
			CMP		AL, 0x0B
			JE		offset isPlayerColor_0
			CMP		AL, 0x14
			JE		offset isPlayerColor_0
			MOV		ECX, ds: [0x006CE0C0] // Save currentFontColor to savedFontColor.
			MOV		ds: [0x006CE0E8], ECX
			MOVZX	EAX, AL
			DEC		AL
			CMP		AL, 0xA // No tfont offset adjustment for <02> through <0A> tags.
			JB		offset off_2
			CMP		AL, 0x1F // tfont offsets for <0B> through <1F> tags.
			JB		offset off_1
			SUB		AL, 0x77 // tfont offsets for <8F> through <9F> tags.
			CMP		AL, 0x22 // Decrement 1 more if above <99>.
			JB		offset off_2
			DEC		AL
			JMP		offset off_2

		off_1:
			SUB		AL, ds: [EAX + 0x004200AC]

		off_2:
			DEC		AL
			LEA		EAX, [EAX * 8 + tFontGame_ImageData] // Set new font color to currentFontColor.
			MOV		ds: [0x006CE0C0], EAX
			RETN

		isPlayerColor_0:
			TEST	AH, AH
			JNE		offset isPlayerMessage_0
			MOV		CL, [ESI + 1]
			CMP		CL, 0x08
			JBE		offset isPlayerColor_2
			SUB		CL, 0x0E
			CMP		CL, 0x03
			JBE		offset isPlayerColor_1
			ADD[ESP], 6
			RETN

		isPlayerMessage_0:
			SHR		EAX, 8
			JMP		offset isPlayerMessage_1

		isPlayerColor_1:
			ADD		CL, 0x09

		isPlayerColor_2:
			INC		ESI
			MOV		AL, CL

		isPlayerMessage_1:
			DEC		AL
			MOV		ECX, ds: [0x006CE0C0] // Save currentFontColor to savedFontColor.
			MOV		ds: [0x006CE0E8], ECX
			PUSH	EDX
			MOV		EDX, offset tFontGame_ImageData + 0x130 // Set player-colored font to currentFontColor.
			MOV		ds: [0x006CE0C0], EDX
			MOVZX	EAX, AL
			LEA		EAX, [EAX * 8 + 0x00581D74] // Unit color table
			PUSH	EBX
			MOV		EBX, [EDX + 1]
			MOV[EDX + 9], EBX

		loop_0:
			ADD		EAX, 2
			INC		EDX
			MOV		BL, [EAX]
			MOV[EDX], BL
			CMP		EDX, offset tFontGame_ImageData + 0x134
			JNE		offset loop_0
			POP		EBX
			POP		EDX
			RETN
		}
	};

	const u32 writeFontApplyColour_1FE01 = 0x0041FE01;
	const u32 writeFontApplyColour_2000A = 0x0042000A;
	void __declspec(naked) writeFontApplyColour_patch() {
		__asm {
			// CL is -1 from actual character ASCII code to naturally exclude <00>.
			CMP		CL, 0x1F // Include <01> to <1F>.
			JB		offset off_3
			CMP		CL, 0x9E // Exclude <A0> to <FF>.
			JA		offset off_1FE01
			CMP		CL, 0x8E // Exclude <20> to <8E>.
			JB		offset off_1FE01
			CMP		CL, 0x98 // Exclude <99>.
			JE		offset off_1FE01
			JMP		offset off_4

		off_3:
			CMP		byte ptr ds: [ECX + 0x004200AC], 0x1 // Exclude the non-color tags (newlines, aligns, etc).
			JE		offset off_1FE01
			MOV		CL, ds: [0x006D5DE0] // dword_6D5DE0
			TEST	CL, CL
			JNE		offset off_2000A
			CMP		dword ptr ds: [0x006CE0C0], offset tFontGame_ImageData[0x18] // Make grey color override subsequent tags.
			JE		off_2000A

		off_4:
			MOVZX	EAX, AL
			CALL	get_color_handle

		off_2000A:
			JMP		writeFontApplyColour_2000A

		off_1FE01:
			JMP		writeFontApplyColour_1FE01
		}
	};

	void __declspec(naked) getColourID_new() {
		__asm {
			MOV		ECX, 3
			CMP		byte ptr ds: [0x006D5BBE], 2 // g_minimap_state
			JNE		offset locret_49B124
			MOV		ECX, ds: [0x00512684] // g_local_nation_id
			MOVZX	EAX, AL
			CMP		ECX, EAX
			JNE		offset loc_49B0FA
			MOV		AL, 2
			JMP		offset loc_49B10E

		loc_49B0FA:
			LEA		ECX, [ECX + ECX * 2]
			MOV		DL, [EAX + ECX * 4 + 0x0058D634] // player_alliance
			TEST	DL, DL
			SETE	AL

		loc_49B10E:
			MOV		CL, AL
			XOR		EAX, EAX

		locret_49B124:
			INC		EAX
			RET
		}
	};

	void __declspec(naked) set_display_text_color() {
		__asm {
			MOV[EDX * 2 + display_text_color], AX
			RET
		}
	};

	void __declspec(naked) get_display_text_color() {
		__asm {
			MOV		AX, [EBX * 2 + display_text_color]
			RET
		}
	};

	const u32 StatHangarCount_25477 = 0x00425477;
	void __declspec(naked) StatHangarCount_patch_25471() {
		__asm {
			PUSH    EDX
			XOR     BL, BL
			MOV     EDX, ds: [0x00597248] // activePortraitUnit
			MOV     EAX, [EDX + 0x64] // unit->id
			CMP     AX, 0x48 // Carrier
			JE      offset loc_isCarrier
			CMP     AX, 0x52 // Gantrithor
			JNE     offset loc_notCarrier

		loc_isCarrier:
			ADD     BL, [EDX + 0xC9] // unit->inHangarCount

		loc_notCarrier:
			ADD     BL, [EDX + 0xC8] // unit->outHangarCount
			MOV     AX, 0x26
			CMP     BL, 0xA
			JAE     offset dont_dec
			DEC     AX
			dont_dec:
			MOV     word ptr[ESI + 0x24], AX
			POP     EDX
			JMP     StatHangarCount_25477
		}
	}
	
	const u32 load_dialog = 0x004F57A0;
	void __declspec(naked) RestartGameMenu() {
		__asm {
			MOV		dword ptr ds: [0x006D1234], 0x004C9530 // last_bindlg_menu_function = CMDACT_RestartGame
			MOV		EDX, 0x004CA240 // game_menu_dialog_interact
			MOV		ECX, 0x0050294C // "rez\restart.bin"
			JMP		load_dialog
		}
	}

	const u32 sub_490300_903D2 = 0x004903D2;
	void __declspec(naked) sub_490300_patch_903CA() {
		__asm {
			LEA		EDX, [ECX * 4 + 0x0057EEE9] // playerTable[playerId].race
			CMP		byte ptr[EDX], 7 // Check if race is Inactive.
			JE		offset off_0
			DEC		EDX // playerTable[playerId].type
			CMP		byte ptr[EDX], 3 // Check if type is neither Human or Computer (Players of type 0 (Unused) have already been filtered out).
			JBE		offset off_0 // Zero-flag is naturally set if the previous CMP is equal, so don't need to CMP again to set it.
			CMP		EDX, EDX // Set zero-flag if player type is 4+.

			off_0:
			JMP sub_490300_903D2
		}
	};

	void __declspec(naked) sub_4AABB0_new() {
		__asm {
			MOV		EAX, ds: [0x0057F0B4] // g_multiplayer_mode
			TEST	AL, AL
			JE		offset off_1
			MOV		CL, ds: [0x00596874] // game_data.bAlliesEnabled
			CMP		CL, AL
			JNE		offset off_0
			MOV		CL, ds: [0x00596877] // game_data.tournamentModeEnabled
			TEST	CL, CL
			JE		offset off_1

		off_0:
			XOR		EAX, EAX

		off_1:
			RETN
		}
	}

	const u32 sub_490E90_90EC8 = 0x00490EC8;
	void __declspec(naked) sub_490E90_patch_90EC3() {
		__asm {
			MOV		EAX, ds: [0x0057F0B4] // g_multiplayer_mode
			OR		EAX, ds: [0x006D0F14] // g_is_in_replay
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
			MOV		ECX, 0xFFFFFFFC // button id

		off_1:
			CMP[EAX + 0x20], CX
			JE		offset off_2
			MOV		EAX, [EAX]
			TEST	EAX, EAX
			JNE		offset off_1
			POP		EBP
			RETN	8

		off_2:
			MOVZX	ECX, word ptr[EBP + 8]
			MOV		DX, ds: [ECX * 2 + 0x00512768] // OKButtonGraphicIndex
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
			PUSH	[ESP] // Love the return address up the stack.
			MOV		AL, ds: [0x006D0F14] // g_is_in_replay
			TEST	AL, AL
			JNE		offset off_0
			MOV		AL, ds: [0x0057F0B4] // g_multiplayer_mode
			TEST	AL, AL
			JNE		offset off_0
			MOV		dword ptr[ESP + 4], offset minimap_sp // Store "minimap_sp.bin" address in the previous stack position.
			RET

		off_0:
			MOV		dword ptr[ESP + 4], 0x00504070 // Store "minimap.bin" address in the previous stack position.
			RET
		}
	}

	const u32 minimap_dlg_Create_A526E = 0x004A526E;
	const u32 GetActivePlayerCount = 0x004C40F0;
	void __declspec(naked) minimap_dlg_Create_patch_A5242() {
		__asm {
			MOV		ECX, ds: [0x006D0F14] // g_is_in_replay
			TEST	ECX, ECX
			MOV		CL, 6
			JNE		offset off_0
			MOV		AL, ds: [0x0057F0B4] // g_multiplayer_mode
			SHL		EAX, CL
			XOR		CL, CL

		off_0:
			MOV		DX, ds: [ECX * 2 + 0x0051276A] // OKButtonGraphicIndex
			ADD		EDX, EAX
			MOV[ESI + 0x24], DX
			CALL	GetActivePlayerCount
			MOV		ECX, ds: [0x0057F0B4] // g_multiplayer_mode
			XOR		EAX, ECX
			JMP		minimap_dlg_Create_A526E
		}
	}

	const u32 preview_minimap_patch_EF06B = 0x004EF06B;
	const u32 LoadMap = 0x004BF520;
	void __declspec(naked) preview_minimap_patch_EF066() {
		__asm {
			OR		byte ptr ds: [0x00596871], 0x80 // game_data.startingUnits
			CALL	LoadMap
			AND		byte ptr ds: [0x00596871], 0x7F // game_data.startingUnits
			JMP		preview_minimap_patch_EF06B
		}
	}

	void glu_expcampaign_custom_ctrl_id() {
		*g_ophelia_enabled = 0;
		*g_multiplayer_mode = 0;
		helper::load_campaign_progress();
		BinDlg* campaign_dialog = 0; // helper::load_full_menu_dialog("rez\\gluExpCmpgn.bin");

		switch (helper::glu_load_bin_dialog(campaign_dialog, helper::glu_expcampaign_custom_ctrl_id)) {
			case 5: {
				g_gl_glues_mode = 13; // GLUE_LOAD;
			} break;
			case 6: {
				g_gl_glues_mode = 9; // GLUE_READY_P;
			} break;
			case 7: {
				g_gl_glues_mode = 7; // GLUE_READY_T;
			} break;
			case 8: {
				g_gl_glues_mode = 8; // GLUE_READY_Z;
			} break;
			case 9: {
				g_gl_glues_mode = 5; // GLUE_LOGIN;
			} break;
			case 10: {
				g_gl_glues_mode = 11; // GLUE_CREATE;
			} break;
			case 11: {
				g_gl_glues_mode = 12; // GLUE_CREATE_MULTI;
			} break;
			default: {
				g_gl_glues_mode = 0; // GLUE_MAIN_MENU;
			} break;
		}
		helper::change_menu();
	}

	char __fastcall glu_main_dialog_interact(BinDlg* text_message, DialogEvent* dlg_event) { // 004DB7A0
		if (dlg_event->number_order == DialogEventType_Dialog) {
			switch (dlg_event->user) {
				case DialogUserType_Create: {
					helper::glu_main_create(text_message);
					helper::dialog_swish_in(text_message);
					helper::generic_dialog_interact(text_message, dlg_event);
					if (g_music_playing) {
						goto LABEL_17;
					}
					helper::dialog_music_fade(0x16); // music_title_wav
					return 1;
				}
				case DialogUserType_Destroy: {
					helper::glu_main_destroy(text_message);
					return helper::generic_dialog_interact(text_message, dlg_event);
				}
				case DialogUserType_Activate:
					switch (*last_control_index) {
						case 3: {
							if (*g_is_demo) {
								if (g_network_table_df->get_string_count() > 102) {
									helper::load_glu_pok_bindlg(g_network_table_df->get_string(103));
								} else {
									helper::load_glu_pok_bindlg("");
								}
								return 1;
							}
							if (*g_is_broodwar) {
								if (!helper::select_game_mode(0)) {
									goto LABEL_17;
								}
								return helper::dialog_swish_out(text_message);
							}
						} break;
						case 4: {
							if (*g_is_broodwar) {
								if (!helper::select_game_mode(1)) {
									return 1;
								}
								return helper::dialog_swish_out(text_message);
							}
							if (*g_is_demo) {
							LABEL_28:
								*g_is_expansion = 0;
								return helper::dialog_swish_out(text_message);
							}
						} break;
						case 5: {
							helper::load_staredit_process(text_message);
							return 1;
						}
						default: {
							return helper::dialog_swish_out(text_message);
						}
					}
					if (!helper::glu_main_display_cdrom_error_bindlg()) {
						goto LABEL_17;
					}
					goto LABEL_28;
				case DialogUserType_InitChildren: {
					helper::register_menu_functions(glu_main_functions, text_message, 0x2Cu, 0);
					return helper::generic_dialog_interact(text_message, dlg_event);
				}
				default: {
					goto LABEL_4;
				}
			}
		}

		if (dlg_event->number_order == DialogEventType_Spacebar && dlg_event->virtual_key == VK_SPACE) {
		LABEL_17:
			return 1;
		} else {
		LABEL_4:
			/*
			if (dlg_event->virtual_key == VK_F10) {
				*last_bindlg_menu_function = helper::game_menu_bindlg_bypass;
				helper::load_dialog("rez\\gamemenu.bin", helper::game_menu_dialog_interact);
			}
			*/
			return helper::generic_dialog_interact(text_message, dlg_event);
		}
	}

	// Hooked to change window name. -Pr0nogo
	void load_title() // 004D49A0
	{
		HANDLE* file_1 = 0;
		if (!helper::sfile_open_file_ex(0, "rez\\titledlg.bin", 0, &file_1)) {
			u32 last_error = helper::serr_get_last_error();
			helper::syswarn_file_not_found(last_error, "rez\\titledlg.bin");
		}

		HANDLE* file_2 = file_1;
		u32 file_size_1 = helper::SFileGetFileSize(file_1, 0);
		u32 file_size_2 = file_size_1;
		if (file_size_1 == -1) {
			u32 last_error_2 = GetLastError();
			helper::file_fatal(file_2, last_error_2);
		}
		if (!file_size_1) {
			helper::syswarn_file_not_found(0x18u, "rez\\titledlg.bin");
		}
		BinDlg* gamedata_dlg = (BinDlg*)helper::SMemAlloc(file_size_1, "Starcraft\\SWAR\\lang\\gamedata.cpp", 210, 0);
		HANDLE* file_3 = file_1;

		u32 read = 0;
		if (!helper::sfile_read_file((HANDLE*)file_1, gamedata_dlg, file_size_2, &read, 0)) {
			if (GetLastError() == ERROR_HANDLE_EOF) {
				helper::file_fatal(file_3, ERROR_BAD_LENGTH);
			}
			u32 last_error_3 = GetLastError();
			helper::file_fatal(file_3, last_error_3);
		}
		if (read != file_size_2) {
			helper::file_fatal(file_3, ERROR_BAD_LENGTH);
		}
		helper::SFileCloseFile(file_1);

		BinDlg* main_dlg = 0;
		if (gamedata_dlg) {
			gamedata_dlg->flags |= BinDlgFlags::Active;
			helper::alloc_init_dialog_data(gamedata_dlg, (void(__fastcall*)(u32, u32, u32, u32, u32))helper::alloc_background_image, "Starcraft\\SWAR\\lang\\title.cpp", 190);
			main_dlg = gamedata_dlg;
		}
		*g_main_dlg = main_dlg;
		helper::initialize_dialog((u32(__fastcall*)(BinDlg*, DialogEvent*))helper::title_dlg_proc, main_dlg);
	}

	// Hooked to change window name. -Pr0nogo
	// Currently doesn't support Xenotron wmode...
	void create_main_window() // 004E05B0
	{
	//	Sleep(3000); // For debugging.
		WNDCLASSEXA wclass;
		memset(&wclass, 0, sizeof(wclass));
		wclass.cbSize = sizeof(wclass);
		wclass.style = 8;
		wclass.lpfnWndProc = (WNDPROC)helper::main_window_proc;
		wclass.hInstance = *g_main_hinstance;
		wclass.hIcon = LoadIconA(*g_main_hinstance, (LPCSTR)0x66);
		wclass.hIconSm = (HICON)LoadImageA(*g_main_hinstance, (LPCSTR)0x66, 1, 16, 16, 0x8000);
		wclass.hCursor = LoadCursorA(0, (LPCSTR)IDC_ARROW);
		wclass.hbrBackground = (HBRUSH)GetStockObject(5);
		wclass.lpszClassName = "SWarClass";
		if (!RegisterClassExA(&wclass)) {
			helper::fatal_error((char*)"RegisterClass");
		}
		HINSTANCE instance = *g_main_hinstance;
		int height = GetSystemMetrics(SM_CYSCREEN);
		int width = GetSystemMetrics(SM_CXSCREEN);
		HWND hwnd = CreateWindowExA(0, "SWarClass", "Cosmonarchy", 0x90080000, 0, 0, width, height, NULL, NULL, instance, NULL);
		*g_main_window = hwnd;
		if (!hwnd) {
			helper::fatal_error((char*)"CreateWindowEx");
			hwnd = *g_main_window;
		}
		UpdateWindow(hwnd);
		SetFocus(*g_main_window);
		SetCursor(NULL);
		if (GetUserDefaultLangID() == 0x412) {
			if (!*g_app_context) {
				*g_app_context = helper::ImmGetContextBW(*g_main_window);
			}
			*g_app_context = helper::ImmAssociateContextBW(*g_main_window, 0);
		}
	}

	// Hooked to set Scenario as the default game type. -Pr0nogo
	// Currently defaults to Competitive... which is good enough for now, I guess!
	void initialize_game_type_list(BinDlg* list) { // 004AF400
		helper::compile_list((s32(__stdcall*)(u32, u32, u32, u32))helper::compile_list_callback);
		s32 last_game_type = helper::get_last_game_type_from_registry(list);
		u8 list_item_count = list->alternateVersion0x46.item_count;
		if (list_item_count) {
			u8 result = 0;
			if (!*g_multiplayer_mode && !last_game_type) {
				int* child = (int*)list->child_smk;
				while (1) {
					int got_template = child[--list_item_count];
					if (helper::is_melee_game(got_template >> 8, 1, got_template)) {
						break;
					}
					if (!list_item_count) {
						helper::unk_list_related(list);
						return;
					}
				}
				helper::dialog_set_selected_immediate(result, list);
			}
			helper::unk_list_related(list);
		} else {
			helper::disable_dialog(list);
		}
	}
}