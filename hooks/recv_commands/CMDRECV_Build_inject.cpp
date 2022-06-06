#include "CMDRECV_Build.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) cmdrecv_place_building_allowed_wrapper()
	{
		static Unit* builder;
		static u8 orderId;
		static u16 builtUnitId;
		static b32 bReturnValue;
		__asm {
			MOV builder, ECX
			MOV orderId, DL
			MOV builtUnitId, AX
			PUSHAD
		}
		bReturnValue = (hooks::cmdrecv_place_building_allowed(builder, orderId, builtUnitId)) ? 1 : 0;
		__asm {
			POPAD
			MOV EAX, bReturnValue
			RETN
		}
	}	

	void __declspec(naked) cmdrecv_place_building_normal_wrapper()
	{
		static s16 x;
		static s16 y;
		static u32 orderId;
		static u32 builtUnitId;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV AX, [EBP + 0x08]
			MOV x, AX
			MOV AX, [EBP + 0x0A]
			MOV y, AX
			MOV EAX, [EBP + 0x0C]
			MOV orderId, EAX
			MOV EAX, [EBP + 0x10]
			MOV builtUnitId, EAX
			PUSHAD
		}
		hooks::cmdrecv_place_building_normal(x, y, orderId, builtUnitId);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) cmdrecv_place_addon_wrapper()
	{
		static s16 x;
		static s16 y;
		static u32 orderId;
		static u32 builtUnitId;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV AX, [EBP + 0x08]
			MOV x, AX
			MOV AX, [EBP + 0x0A]
			MOV y, AX
			MOV EAX, [EBP + 0x0C]
			MOV orderId, EAX
			MOV EAX, [EBP + 0x10]
			MOV builtUnitId, EAX
			PUSHAD
		}
		hooks::cmdrecv_place_addon(x, y, orderId, builtUnitId);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) cmdrecv_build_wrapper()
	{
		static u8 orderId;
		static s16 x;
		static s16 y;
		static u16 builtUnitId;
		__asm {
			MOV AL, [ESI + 0x01]
			MOV orderId, AL
			MOV AX, [ESI + 0x02]
			MOV x, AX
			MOV AX, [ESI + 0x04]
			MOV y, AX
			MOV AX, [ESI + 0x06]
			MOV builtUnitId, AX
			PUSHAD
		}
		hooks::cmdrecv_build(orderId, x, y, builtUnitId);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) process_building_placement_wrapper() { // 0048DE70
		static Unit* unit;
		static u8 order_id;
		static Point16 pos;
		static u16 target_unit_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV order_id, AL
			MOV EAX, [EBP + 0x0C]
			MOV pos, EAX
			MOV EAX, [EBP + 0x10]
			MOV target_unit_id, AX
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		hooks::process_building_placement(unit, order_id, pos, target_unit_id);
		__asm {
			POPAD
			RETN 0x0C
		}
	}
}

namespace hooks
{
	void inject_cmdrecv_build_hooks()
	{
		jump_patch(cmdrecv_place_building_allowed_wrapper,	0x0048DBD0, 1);
		jump_patch(cmdrecv_place_building_normal_wrapper,	0x0048E010, 0);
	//	jump_patch(cmdrecv_place_addon_wrapper,				0x0048E0A0, 0);
		jump_patch(cmdrecv_build_wrapper,					0x004C23C0, 3);
	//	jump_patch(process_building_placement_wrapper,		0x0048DE70, 0);
	}
}