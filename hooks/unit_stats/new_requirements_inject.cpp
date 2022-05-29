#include "new_requirements.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) createUnitAllowed_PartialWrapper() // 0046E1C0
	{
		bool runOriginal;
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

		result = hooks::createUnitAllowed_Partial(unitId, unit, playerId);

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
	void injectNewRequirementsHooks()
	{
		jump_patch(createUnitAllowed_PartialWrapper, 0x0046E1C0, 1);
	}
}