#include "placement.h"
#include "hook_tools.h"

namespace
{
	void __declspec(naked) InitPlacementBoxPos_Wrapper()
	{
		static s32 unused;
		static Point16* placement;
		static s32 result;

		__asm
		{
			MOV unused, ECX
			MOV placement, EDX
			PUSHAD
		}
		result = hooks::InitPlacementBoxPos(unused, placement);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) DrawPlacementBox_Wrapper()
	{
		static Point16 ecx0;
		static u8 result;

		__asm
		{
			MOV ecx0, ECX
			PUSHAD
		}
		result = hooks::DrawPlacementBox(ecx0);
		__asm
		{
			POPAD
			MOVZX EAX, result
			RETN
		}
	}


}

namespace hooks
{
	void injectPlacementHooks()
	{
		jmpPatch(InitPlacementBoxPos_Wrapper, 0x0048D660, 0);
		jmpPatch(DrawPlacementBox_Wrapper, 0x0048DDC0, 4);
	}
}