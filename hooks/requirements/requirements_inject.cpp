#include "requirements.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) parse_requirement_opcodes_wrapper() // 0046D610
	{
		static Unit* unit;
		static u32 dat_req_offset;
		static u32 tech_id;
		static u32 player_id;
		static u16* dat_req_base;
		static s32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV dat_req_offset, EAX
			MOV unit, ESI
			MOV EAX, [EBP + 0x08]
			MOV tech_id, EAX
			MOV EAX, [EBP + 0x0C]
			MOV player_id, EAX
			MOV EAX, [EBP + 0x10]
			MOV dat_req_base, EAX
			PUSHAD
		}
		result = hooks::parse_requirement_opcodes(unit, dat_req_offset, tech_id, player_id, dat_req_base);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) unit_create_allowed_wrapper() // 0046E1C0
	{
		static s32 result;
		static s16 unit_id;
		static Unit* unit;
		static s32 player;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			POP EAX
			MOV unit_id, AX
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::unit_create_allowed(unit_id, unit, player);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}
}

namespace hooks
{
	void inject_requirement_hooks()
	{
	//	jump_patch(parse_requirement_opcodes_wrapper,	0x0046D610, 1);
	//	jump_patch(unit_create_allowed_wrapper,			0x0046E1C0, 0);
	}
}