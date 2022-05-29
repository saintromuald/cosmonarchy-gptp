#include "build_time.h"
#include "../hook_tools.h"

namespace
{
	void __declspec(naked) decrementRemainingBuildTime_Wrapper()
	{ // 0x00466940
		static Unit* unit;
		s32 result;
		_asm {
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::decrementRemainingBuildTime(unit);
		_asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void injectBuildTimeHooks()
	{
		jump_patch(decrementRemainingBuildTime_Wrapper, 0x00466940, 1);
	}
}