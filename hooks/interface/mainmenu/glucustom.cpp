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
		if (!*multiplayer_mode && !(*g_cheat_state & CheatFlags::NoGlues) && *campaign_index)
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
			MOV[EDI], SI
			MOVZX   SI, byte ptr[EAX + 0x1]
			MOV     EDI, [EBP + 0xC]
			MOV[EDI], SI
			MOV     SI, [ECX + 0x2]
			MOV     EDI, [EBP + 0x10]
			MOV[EDI], SI
			MOV     SI, [ECX + 0x4]
			MOV     EDI, [EBP + 0x14]
			MOV[EDI], SI
			MOVZX   SI, byte ptr[EAX + 0x3]
			MOV[EDX + 0x2], SI
			MOVZX   SI, byte ptr[EAX + 0x2]
			MOV     EAX, [EAX + 0x4]
			TEST    EAX, EAX
			POP     EDI
			JNS     short loc_41D1C3
			AND		EAX, 0x7FFFFFFF
			ADD     ESI, 0x100

			loc_41D1C3 :
			ADD     EAX, ECX
				MOV[EDX], SI
				MOV[EDX + 0x4], EAX
				POP     ESI
				POP     EBP
				RETN    0x10
		}
	}

#define off_6D5E30 __asm _emit 0x8B __asm _emit 0x0D __asm _emit 0x30 __asm _emit 0x5E __asm _emit 0x6D __asm _emit 0x00
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
			off_6D5E30//MOV ECX, dword ptr[0x006D5E30]
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
			MOV[EBP - 0x10], CX
			JG      short loc_4EF64A
			MOV[EBP - 0x10], AX

			loc_4EF64A :
			MOVSX   EAX, word ptr[ESI + 0x2]
				PUSH    EBX
				MOVZX   EBX, DI
				CMP     EBX, EAX
				MOV[EBP - 0xE], BX
				JG      short loc_4EF65E
				MOV[EBP - 0xE], AX

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
			MOV[EBP - 0xC], EAX
			MOV     EAX, [EBP + 0x14]
			ADD     EDX, ECX
			PUSH    EAX
			MOV[EBP - 0x10], ECX
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
			MOV[EBP - 0x8], EAX
			MOV     EAX, ESI
			SUB     EAX, EDI
			INC     BX//formerly BL
			DEC     EAX
			CDQ
			IDIV    EDI
			SUB     word ptr[EBP - 0xC], DI
			MOV[EBP - 0x4], BX//formerly BL
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
				MOV[EBP - 0x8], ECX
				JNZ     short loc_4EF766
				MOV     ECX, [EBP + 0x8]
				MOV     ESI, [EBP + 0x10]

			loc_4EF78F :
				MOV     DX, [EBP - 0x4]//formerly DL
				ADD     word ptr[EBP - 0xC], DI
				INC     DX//formerly DL
				CMP     ESI, EDI
				MOV[EBP - 0x4], DX//formerly DL
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
	#define aStarcraftSw_47 __asm _emit 0x68 __asm _emit 0xD8 __asm _emit 0x34 __asm _emit 0x50 __asm _emit 0x00
	const u32 LoadGraphic = 0x004D2B30;
	#define dword_59B72C __asm _emit 0xA3 __asm _emit 0x2C __asm _emit 0xB7 __asm _emit 0x59 __asm _emit 0x00
	const u32 loadMenu_gluScore = 0x004B4B92;
	void __declspec(naked) loadMenu_gluScore_pIcons() {
		__asm {
		LOC_4B4B80_pIcons:
			MOV		CL, [EAX + 0x0059B628]
				MOV[EBP + EAX - 0x10C], CL
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
				MOV[EDI], EDX
			//	MOV		DL, byte ptr ds : aIscore_grp + 0xA
				MOV[EDI + 0x4], EAX
				MOV[EDI + 0x8], CX
				PUSH	0x560
				MOV[EDI + 0xA], 0x0 // formerly DL, would grab the string's null terminator from the above commented-out line.
				aStarcraftSw_47		// PUSH offset 0x005034D8//"Starcraft\SWAR\lang\gluScore.cpp"
				XOR		EDX, EDX
				LEA		ECX, [EBP - 0x10C]
				CALL	LoadGraphic
				dword_59B72C		// MOV [0x0059B72C], EAX
				XOR		EAX, EAX

				LOC_4B4B80_iScore :
			MOV		CL, [EAX + 0x0059B628]
				MOV[EBP + EAX - 0x10C], CL
				INC		EAX
				TEST	CL, CL
				JNZ		short LOC_4B4B80_iScore
				JMP		loadMenu_gluScore
		}
	}

	void __declspec(naked) get_color_handle() {
		__asm {
			CMP		AL, 0x1
			JNE		offset off_0
			MOV		EAX, ds: [0x006CE0E8]//<Set savedFontColor to currentFontColor if <01>.
			MOV		ds : [0x006CE0C0] , EAX
			RETN

		off_0 :
			MOV		ECX, ds : [0x006CE0C0]//Save currentFontColor to savedFontColor.
			MOV		ds : [0x006CE0E8] , ECX
			MOVZX	EAX, AL
			DEC		AL
			PUSH	EAX
			CMP		AL, 0xA//Skip if <02> to <0A>.
			JB		offset off_2
			CMP		AL, 0x1F//Go to close gaps if <0B> to <1F>.
			JB		offset off_1
			SUB		AL, 0x75//Close the gap between <1F> and <8F> so we avoid having to insert unused space in tfont.pcx.
			CMP		AL, 0x24//Decrement 1 more if above <99>.
			JB		offset off_2
			DEC		AL
			JMP		offset off_2

		off_1 :
			PUSH	ECX
			MOV		CL, ds : [EAX + 0x004200AC]
			SUB		AL, CL
			POP		ECX

		off_2 :
			DEC		AL
			LEA		EAX, [EAX * 8 + tFontGame_ImageData]//Set new font color to currentFontColor.
			MOV		ds : [0x006CE0C0] , EAX
			POP		EAX
			RETN

		}
	};

	const u32 writeFontApplyColour_1FE01 = 0x0041FE01;
	const u32 writeFontApplyColour_2000A = 0x0042000A;
	void __declspec(naked) write_font_apply_color_patch() {
		__asm {
			// CL is -1 from actual character ASCII code to naturally exclude <00>.
			CMP		CL, 0x1F // Include <01> to <1F>.
			JB		offset off_2
			CMP		CL, 0x9E // Exclude <A0> to <FF>.
			JA		offset off_1FE01
			CMP		CL, 0x8E // Exclude <20> to <8E>.
			JB		offset off_1FE01
			CMP		CL, 0x98 // Exclude <99>.
			JE		offset off_1FE01
			JMP		offset off_3

		off_2 :
			MOVZX	ECX, byte ptr ds : [ECX + 0x004200AC] // Exclude the non-color tags (newlines, aligns, etc).
			TEST	CL, 0x1
			JNE		offset off_1FE01
			MOV		CL, ds : [0x006D5DE0]
			TEST	CL, CL
			JNE		offset off_2000A
			CMP		dword ptr ds : [0x006CE0C0] , offset tFontGame_ImageData + 0x18 // Make grey color override subsequent tags.
			JE		off_2000A

		off_3 :
			CALL	get_color_handle

		off_2000A :
			JMP		writeFontApplyColour_2000A

		off_1FE01 :
			JMP		writeFontApplyColour_1FE01
		}
	};
}