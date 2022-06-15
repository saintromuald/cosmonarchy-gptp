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
	void inject_briefing_hooks()
	{
//		jump_patch(updateMissionObjectives_Wrapper,	0x0046CD30, 0);
	}
}