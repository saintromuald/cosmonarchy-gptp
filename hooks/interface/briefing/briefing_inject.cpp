#include "briefing.h"
#include <hook_tools.h>

namespace
{
	void __declspec (naked) update_mission_objectives_wrapper() // 0046CD30
	{
		__asm
		{
			PUSHAD
		}
		hooks::update_mission_objectives();
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
	//	jump_patch(update_mission_objectives_wrapper,	0x0046CD30, 0);
	}
}