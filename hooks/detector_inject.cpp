#include "detector.h"
#include "../hook_tools.h"

extern const u32 Func_CanDetect; // Defined in Unit.cpp

namespace
{
	void __declspec(naked) unit_can_detect_wrapper()
	{
		static Unit* unit;
		static u32 result;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_can_detect(unit) ? 1 : 0;
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) get_cloaked_target_visibility_wrapper()
	{
		static DetectorCheckParam* param;
		static Unit* detector;
		__asm {
			MOV detector, ECX
			MOV param, EDX
			PUSHAD
		}
		hooks::get_cloaked_target_visibility(detector, param);

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
	void inject_detector_hooks()
	{
		jump_patch(unit_can_detect_wrapper,					Func_CanDetect);
		jump_patch(get_cloaked_target_visibility_wrapper,	0x004408A0, 9);
	}
}