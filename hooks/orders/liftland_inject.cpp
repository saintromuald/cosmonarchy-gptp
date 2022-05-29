//Injector source file for the Lift/Land hook module.
#include "liftland.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) orders_BuildingLandWrapper() // 00464360
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x10
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_BuildingLand(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}

	void __declspec(naked) orders_BuildingLiftoffWrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x08
			MOV unit, ESI
			PUSHAD
		}
		hooks::orders_BuildingLiftoff(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RET
		}
	}
}

namespace hooks
{

	void injectLiftLandHooks()
	{
		jump_patch(orders_BuildingLandWrapper, 0x00464360, 1);
		jump_patch(orders_BuildingLiftoffWrapper, 0x004649B0, 1);
	}

}
