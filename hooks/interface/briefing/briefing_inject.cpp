#include "briefing.h"
#include <hook_tools.h>

namespace
{
	void __declspec (naked) updateMissionObjectives_Wrapper() // 0046CD30
	{
		__asm
		{
			PUSHAD
		}
		hooks::updateMissionObjectives();
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void injectBriefingHooks()
	{
//		jmpPatch(updateMissionObjectives_Wrapper, 0x0046CD30, 0);
	}
}