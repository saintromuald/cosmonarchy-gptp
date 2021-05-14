#include "placement.h"
#include "hook_tools.h"

namespace
{
	void _declspec(naked) sub_48D660_Wrapper()
	{
		static s32 a1;
		static Point16* pPt;
		static s32 result;

		__asm
		{
			MOV a1, ECX
			MOV pPt, EDX
			PUSHAD
		}
		result = hooks::sub_48D660(a1, pPt);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void injectPlacementHooks()
	{
		jmpPatch(sub_48D660_Wrapper, 0x0048D660, 0);
	}
}