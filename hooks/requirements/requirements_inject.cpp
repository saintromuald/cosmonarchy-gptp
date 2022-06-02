#include "requirements.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) getParseRequirementOpcodesWrapper() // 0046D610
	{
		static Unit* unit;
		static u32 datReqOffset;
		static u32 techID;
		static u32 playerId;
		static u16* datReqBase;
		static s32 result;

		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV datReqOffset, EAX
			MOV unit, ESI
			MOV EAX, [EBP + 0x08]
			MOV techID, EAX
			MOV EAX, [EBP + 0x0C]
			MOV playerId, EAX
			MOV EAX, [EBP + 0x10]
			MOV datReqBase, EAX
			PUSHAD
		}
		result = hooks::parse_requirement_opcodes(unit, datReqOffset, techID, playerId, datReqBase);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 12
		}
	}

	void __declspec(naked) getUnitCreateAllowedWrapper() // 0046E1C0
	{
		static s32 result;
		static s16 unitId;
		static Unit* unit;
		static s32 playerId;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV playerId, EAX
			POP EAX
			MOV unitId, AX
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::UnitCreateAllowed(unitId, unit, playerId);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void injectRequirementHooks()
	{
		jump_patch(getParseRequirementOpcodesWrapper,	0x0046D610, 0);
		jump_patch(getUnitCreateAllowedWrapper,			0x0046E1C0, 0);
	}
}