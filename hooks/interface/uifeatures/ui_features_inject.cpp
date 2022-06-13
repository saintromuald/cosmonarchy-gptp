#include "ui_features.h"
#include "hook_tools.h"

namespace
{
	const u32 SetFontHandle = 0x0041F610;
	const u32 drawUIFeatures_jmp = 0x0048D0AE;
	void _declspec(naked) draw_ui_features_wrapper()
	{
		_asm {
			PUSHAD
		}
		hooks::draw_ui_features();
		_asm {
			POPAD
			CALL SetFontHandle
			JMP drawUIFeatures_jmp
		}
	}

	void _declspec(naked) queue_game_command_wrapper()
	{
		static const void* a1;
		static u32 a2;
		static u8 result;
		_asm {
			MOV a1, ECX
			MOV a2, EDX
			PUSHAD
		}
		result = hooks::queue_game_command(a1, a2);
		_asm {
			MOVZX EAX, result
			POPAD
			RETN
		}
	}

	void __declspec(naked) targeted_order_wrapper()
	{
		static s16 a1;
		static s32 a2;
		static Unit* a3;
		static s16 a4;
		static s32 a5;
		static s32 a6;
		static s32 a7;
		static s32 a8;
		static s32 a9;
		static s32 a10;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV a1, DX
			MOV a2, EBX
			MOV a3, ESI
			MOV a4, AX
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV a5, EAX
			MOV EAX, [EBP + 0xC]
			MOV a6, EAX
			MOV EAX, [EBP + 0x10]
			MOV a7, EAX
			MOV EAX, [EBP + 0x14]
			MOV a8, EAX
			MOV EAX, [EBP + 0x18]
			MOV a9, EAX
			MOV EAX, [EBP + 0x1C]
			MOV a10, EAX
			POP EAX
			PUSHAD
		}
		hooks::targeted_order(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x18
		}
	}

	void __declspec(naked) select_units_wrapper()
	{
		static Unit** a1;
		static s32 a2;
		static s32 a3;
		static s32 a4;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV a1, EBX
			MOV a2, EDI
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV a3, EAX
			MOV EAX, [EBP + 0xC]
			MOV a4, EAX
			POP EAX
			PUSHAD
		}
		hooks::select_units(a1, a2, a3, a4);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}

	void __declspec(naked) check_hotkey_char_wrapper()
	{
		static b32 result;
		static BinDlg* dialog;
		static char* hotkey;
		__asm
		{
			MOV dialog, ECX
			MOV hotkey, EDX
			PUSHAD
		}
		result = hooks::check_hotkey_char(dialog, *hotkey);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) create_upgrade_level_tooltip_wrapper()
	{
		static char result;
		static char* text;
		__asm
		{
			MOV text, ECX
			PUSHAD
		}
		result = hooks::create_upgrade_level_tooltip(text);
		__asm
		{
			POPAD
			MOVZX EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_ui_features_hooks()
	{
		jump_patch(draw_ui_features_wrapper,				0x0048D0A9, 0);
	//	jump_patch(targeted_order_wrapper,					0x004754F0, 1);
	//	jump_patch(select_units_wrapper,					0x0046FA00, 1);
		jump_patch(queue_game_command_wrapper,				0x00485BD0, 4);
		jump_patch(check_hotkey_char_wrapper,				0x004588C0, 3);
	//	jump_patch(create_upgrade_level_tooltip_wrapper,	0x00459150, 3);
	}
}