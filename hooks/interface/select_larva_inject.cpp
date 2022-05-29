#include "select_larva.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) getLarvaeUnitsFromList_Wrapper()
	{
		static Unit** unitList;
		static Unit** outputList;
		static u32 larvaeCount;
		__asm {
			MOV unitList, EDX
			MOV outputList, ESI
			PUSHAD
		}
		larvaeCount = hooks::get_larvae_units_from_list(unitList, outputList);

		__asm {
			POPAD
			MOV EAX, larvaeCount
			RETN
		}
	}

	void __declspec(naked) BTNSACT_SelectLarva_Wrapper()
	{
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::BTSNACT_select_larva();
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}
}

namespace hooks
{
	void injectSelectLarvaHooks()
	{
		jump_patch(getLarvaeUnitsFromList_Wrapper,	0x00423190, 1);
		jump_patch(BTNSACT_SelectLarva_Wrapper,		0x00423930, 1);
	}
}