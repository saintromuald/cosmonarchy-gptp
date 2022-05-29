#include "detector.h"
#include "../hook_tools.h"

extern const u32 Func_CanDetect; // Defined in Unit.cpp

namespace
{
	void __declspec(naked) unitCanDetectWrapper()
	{
		static Unit* unit;
		static u32 result;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unitCanDetectHook(unit) ? 1 : 0;
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) getCloakedTargetVisibilityWrapper()
	{
		static DetectorCheckParam* param;
		static Unit* detector;
		__asm {
			MOV detector, ECX
			MOV param, EDX
			PUSHAD
		}
		hooks::getCloakedTargetVisibility(detector, param);

		__asm {
			POPAD
			XOR EAX, EAX
			RETN
		}
		// Changes of param are not moved in something in this asm bloc because it was a pointer directly on Starcraft data from the start
	}
}

namespace hooks
{
	void injectDetectorHooks()
	{
		jump_patch(unitCanDetectWrapper,				Func_CanDetect);
		jump_patch(getCloakedTargetVisibilityWrapper,	0x004408A0, 9);
	}
}