#include "utils.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) unit_IsStandardAndMovableWrapper()
	{
		static Unit* unit;
		static b32 result;
		__asm
		{
			MOV unit, ECX
			PUSHAD
		}
		if (hooks::unit_is_standard_and_movable(unit))
		{
			result = 1;
		}
		else
		{
			result = 0;
		}
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) getActivePlayerFirstSelectionWrapper()
	{
		static Unit* unit;
		__asm PUSHAD
		unit = hooks::get_active_player_first_selection();
		__asm
		{
			POPAD
			MOV EAX, unit
			RETN
		}
	}
}

namespace hooks
{
	void inject_utils_hooks()
	{
		jump_patch(unit_IsStandardAndMovableWrapper,		0x0047B770, 2);
		//jump_patch(getActivePlayerFirstSelectionWrapper,	0x0049A850, 2);
	}
}
