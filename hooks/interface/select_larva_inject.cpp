#include "select_larva.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) get_larvae_units_from_list_wrapper()
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

	void __declspec(naked) btnsact_select_larva_wrapper()
	{
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::btnsact_select_larva();
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
	void inject_select_larva_hooks()
	{
		jump_patch(get_larvae_units_from_list_wrapper,	0x00423190, 1);
		jump_patch(btnsact_select_larva_wrapper,		0x00423930, 1);
	}
}