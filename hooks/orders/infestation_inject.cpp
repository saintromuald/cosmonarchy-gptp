//Injector source file for the Infestation hook module.
#include "infestation.h"
#include <hook_tools.h>

namespace
{

	//Inject with jump_patch()
	void __declspec(naked) unitCanInfestWrapper()
	{

		static Unit* unit;
		static u32 result;

		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EAX
		}

		result = hooks::unitCanInfestHook(unit) ? 1 : 0;

		__asm {
			POPAD
			MOV EAX, result
			RETN
		}

	}

	//Inject with jump_patch()
	void __declspec(naked) unitCanBeInfestedWrapper()
	{
		static Unit* unit;
		static u32 result;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EDX
		}
		result = hooks::isInfestableUnitHook(unit) ? 1 : 0;
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	//Inject with jump_patch()
	void __declspec(naked) unitMorphIntoInfestedOrder()
	{

		static Unit* unitInfested;

		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unitInfested, EAX
		}

		hooks::orderMorphIntoInfested(unitInfested);

		__asm {
			POPAD
			RETN
		}

	}

	;

	//Inject with jump_patch()
	void __declspec(naked) unitInfestOrder()
	{

		static Unit* unitInfesting;

		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unitInfesting, EAX
		}

		hooks::orderInfestTarget(unitInfesting);

		__asm {
			POPAD
			RETN
		}

	}

	;

} //unnamed namespace

namespace hooks
{

	void injectInfestationHooks()
	{
		jump_patch(unitCanBeInfestedWrapper, 0x00402210);
		jump_patch(unitCanInfestWrapper, 0x00402750);
		jump_patch(unitInfestOrder, 0x004EA290);
		jump_patch(unitMorphIntoInfestedOrder, 0x004EA4C0);
	}

}
