#include "wireframe.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) statdata_UnitWireframeUpdate_FragmentWrapper()
	{
		static const u32 JUMPBACK_POINT_WIREFRAME = 0x00456955;
		static Unit* unit;
		static u16 unitId;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		unitId = hooks::statdata_UnitWireframeUpdate_Fragment(unit);
		__asm {
			POPAD
			MOVZX ECX, unitId
			JMP JUMPBACK_POINT_WIREFRAME
		}
	}
	void __declspec(naked) wireframeShields_Wrapper()
	{
		static Unit* unit;
		static u8 result;
		__asm {
			MOV unit, EDX
			PUSHAD
		}
		result = hooks::wireframeShields(unit);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}
}

namespace hooks
{
	void injectWireframeHook()
	{
		// unused right now, must be updated for new unit ids if used
	//	jump_patch(statdata_UnitWireframeUpdate_FragmentWrapper,	0x00456911, 2);
		jump_patch(wireframeShields_Wrapper,						0x004566B0, 5);
	}
}