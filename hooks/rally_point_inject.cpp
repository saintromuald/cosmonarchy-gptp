#include "rally_point.h"
#include "../hook_tools.h"

namespace
{
	void __declspec(naked) orderNewUnitToRallyWrapper()
	{
		static Unit* unit;
		static Unit* factory;
		__asm {
			MOV unit, EAX
			MOV factory, ECX
			PUSHAD
		}
		hooks::order_new_unit_to_rally(unit, factory);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) setRallyPositionWrapper()
	{
		static Unit* unit;
		static u16 x;
		static u16 y;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			MOV CX, [EBP + 0x08]
			MOV x, CX
			MOV DX, [EBP + 0x0C]
			MOV y, DX
			PUSHAD
		}
		hooks::set_rally_position(unit, x, y);
		__asm {
			POPAD
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) setRallyUnitWrapper()
	{
		static Unit* unit;
		static Unit* target;
		__asm {
			MOV unit, ECX
			MOV target, EAX
			PUSHAD
		}
		hooks::set_rally_unit(unit, target);
		__asm {
			POPAD
			RETN
		}
	}

	// Additional wrappers for uses of the logic that would be hardcoded instead of using the functions
	void __declspec(naked) set_rally_position_wrapper_jump1()
	{
		static const u32 Hook_SetRallyGenericJmpBack1 = 0x00456384;
		static Unit* unit;
		static u16 x;
		static u16 y;
		__asm {
			MOV unit, EDI
			MOV x, BX
			MOV y, SI
			PUSHAD
		}
		hooks::set_rally_position(unit, x, y);
		__asm {
			POPAD
			JMP Hook_SetRallyGenericJmpBack1
		}
	}

	void __declspec(naked) set_rally_position_wrapper_jump2()
	{
		static const u32 Hook_SetRallyGenericJmpBack2 = 0x0049AD77;
		static Unit* unit;
		static u16 x;
		static u16 y;
		__asm {
			MOV unit, ESI
			MOV x, CX
			MOV y, DX
			PUSHAD
		}
		hooks::set_rally_position(unit, x, y);
		__asm {
			POPAD
			JMP Hook_SetRallyGenericJmpBack2
		}
	}

	void __declspec(naked) set_rally_unit_wrapper_jump1()
	{
		static const u32 Hook_SetRallyGenericJmpBack1 = 0x00456384;
		static Unit* unit;
		static Unit* target;
		__asm {
			MOV unit, EDI
			MOV target, EAX
			PUSHAD
		}
		hooks::set_rally_unit(unit, target);
		__asm {
			POPAD
			JMP Hook_SetRallyGenericJmpBack1
		}
	}

	void __declspec(naked) set_rally_unit_wrapper_jump2()
	{
		static const u32 Hook_SetRallyGenericJmpBack2 = 0x0049AD77;
		static Unit* unit;
		static Unit* target;
		__asm {
			MOV unit, ESI
			MOV target, EBX
			PUSHAD
		}
		hooks::set_rally_unit(unit, target);

		__asm {
			POPAD
			JMP Hook_SetRallyGenericJmpBack2
		}
	}

	void __declspec(naked) remove_references_wrapper() { // 0049E590
		static Unit* unit;
		static Unit* dying;
		static Order* result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV dying, EAX
			POP EAX
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::remove_references(unit, dying);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) right_click_command_wrapper() { // 004560D0
		static s32 x;
		static s32 y;
		static Unit* unit;
		static s32 fow_unit_id;
		static s8 queued;
		static Unit* result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV x, EAX
			MOV EAX, [EBP + 0x0C]
			MOV y, EAX
			MOV EAX, [EBP + 0x10]
			MOV unit, EAX
			MOV EAX, [EBP + 0x14]
			MOV fow_unit_id, EAX
			MOV EAX, [EBP + 0x18]
			MOV queued, AL
			POP EAX
			PUSHAD
		}
		result = hooks::right_click_command(x, y, unit, fow_unit_id, queued);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x14
		}
	}

	void __declspec(naked) btnscond_rally_wrapper() // 00429520
	{
		static u32 unused;
		static u32 player;
		static Unit* unit;
		static u32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			MOV player, EDX
			MOV unused, ECX
			PUSHAD
		}
		result = hooks::btnscond_rally(unused, player, unit);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}
}

namespace hooks
{
	void inject_rally_hooks()
	{
		jump_patch(setRallyPositionWrapper,				0x00466910, 2);
		jump_patch(setRallyUnitWrapper,					0x00466B40, 1);
		jump_patch(orderNewUnitToRallyWrapper,			0x00466F50, 0);

		jump_patch(set_rally_position_wrapper_jump1,	0x00456256, 19);	// 24 - 5 of jump
		jump_patch(set_rally_position_wrapper_jump2,	0x0049ACEF, 19);	// 24 - 5 of jump

		jump_patch(set_rally_unit_wrapper_jump1,		0x0045620D, 35);	// 40 - 5 of jump
		jump_patch(set_rally_unit_wrapper_jump2,		0x0049ACB0, 37);	// 42 - 5 of jump

		jump_patch(remove_references_wrapper,			0x0049E590, 1);
		jump_patch(right_click_command_wrapper,			0x004560D0, 1);

	//	jump_patch(btnscond_rally_wrapper,				0x00429520, 7);
	}
}
