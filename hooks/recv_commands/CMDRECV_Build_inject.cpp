#include "CMDRECV_Build.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) cmdrecv_place_building_allowed_wrapper()
	{
		static Unit* builder;
		static u8 order_id;
		static u16 built_unit_id;
		static b32 bReturnValue;
		__asm {
			MOV builder, ECX
			MOV order_id, DL
			MOV built_unit_id, AX
			PUSHAD
		}
		bReturnValue = (hooks::cmdrecv_place_building_allowed(builder, order_id, built_unit_id)) ? 1 : 0;
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
		static u32 order_id;
		static u32 built_unit_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV AX, [EBP + 0x08]
			MOV x, AX
			MOV AX, [EBP + 0x0A]
			MOV y, AX
			MOV EAX, [EBP + 0x0C]
			MOV order_id, EAX
			MOV EAX, [EBP + 0x10]
			MOV built_unit_id, EAX
			PUSHAD
		}
		hooks::cmdrecv_place_building_normal(x, y, order_id, built_unit_id);
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
		static u32 order_id;
		static u32 built_unit_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV AX, [EBP + 0x08]
			MOV x, AX
			MOV AX, [EBP + 0x0A]
			MOV y, AX
			MOV EAX, [EBP + 0x0C]
			MOV order_id, EAX
			MOV EAX, [EBP + 0x10]
			MOV built_unit_id, EAX
			PUSHAD
		}
		hooks::cmdrecv_place_addon(x, y, order_id, built_unit_id);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) cmdrecv_build_wrapper()
	{
		static u8 order_id;
		static s16 x;
		static s16 y;
		static u16 built_unit_id;
		__asm {
			MOV AL, [ESI + 0x01]
			MOV order_id, AL
			MOV AX, [ESI + 0x02]
			MOV x, AX
			MOV AX, [ESI + 0x04]
			MOV y, AX
			MOV AX, [ESI + 0x06]
			MOV built_unit_id, AX
			PUSHAD
		}
		hooks::cmdrecv_build(order_id, x, y, built_unit_id);
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
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) cmdrecv_place_building_wrapper() { // 0048E190
		static u32 order_id;
		static u16 built_unit_id;
		static Point16 position;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV position, EAX
			POP EAX
			MOV built_unit_id, AX
			MOV order_id, ECX
			PUSHAD
		}
		hooks::cmdrecv_place_building(order_id, built_unit_id, position);
		__asm {
			POPAD
			MOV EAX, 0 // always 0
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) check_resources_for_building_wrapper() { // 0042D0C0
		static b32 result;
		static u32 notify;
		static u8 player;
		static u32 unused;
		__asm {
			MOV notify, EAX
			MOV player, CL
			MOV unused, ESI // dunno if this is necessary to clear the register lol
			PUSHAD
		}
		result = hooks::check_resources_for_building(notify, player);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) display_error_message_id_with_sfx_wrapper() { // 0048EE30
		static u32 player_id;
		static u32 message_id;
		static u32 sound_id;
		__asm {
			MOV sound_id, ESI
			MOV message_id, EDI
			MOV player_id, EBX
			PUSHAD
		}
		hooks::display_error_message_id_with_sfx(player_id, message_id, sound_id);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_cmdrecv_build_hooks()
	{
	//	jump_patch(cmdrecv_place_building_allowed_wrapper,		0x0048DBD0, 1);
	//	jump_patch(cmdrecv_place_building_normal_wrapper,		0x0048E010, 0);
	//	jump_patch(cmdrecv_place_addon_wrapper,					0x0048E0A0, 0);
	//	jump_patch(cmdrecv_build_wrapper,						0x004C23C0, 3);
	//	jump_patch(process_building_placement_wrapper,			0x0048DE70, 0);
	//	jump_patch(cmdrecv_place_building_wrapper,				0x0048E190, 1);
	//	jump_patch(check_resources_for_building_wrapper,		0x0042D0C0, 6);
		jump_patch(display_error_message_id_with_sfx_wrapper,	0x0048EE30, 1);
	}
}