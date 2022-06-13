#include "neutral_related.h"
#include <SCBW/api.h>
#include <hook_tools.h>

namespace
{
	void __declspec(naked) unit_is_not_neutral_wrapper() // 004CBE20
	{
		static CHKUnit* unit;
		static bool result;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_is_not_neutral(unit);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_neutral_related_hooks()
	{
		jump_patch(unit_is_not_neutral_wrapper, 0x004CBE20, 1);
	}
}