#include <SCBW/api.h>
#include "sfxhooks.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) getResponseSfx_Wrapper() // 0048EA20
	{
		static s32 a1;
		static s32 a2;
		static s32 result;
		__asm {
			MOV a1, EAX
			MOV a2, EDX
			PUSHAD
		}
		result = hooks::getResponseSfx(a1, a2);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) getRandYesSFX_Wrapper() // 0048EAB0
	{
		static s32 a1;
		static s32 result;
		__asm {
			MOV a1, ECX
			PUSHAD
		}
		result = hooks::getRandYesSFX(a1);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void injectSfxHooks()
	{
		jump_patch(getResponseSfx_Wrapper,	0x0048EA20, 1);
		jump_patch(getRandYesSFX_Wrapper,	0x0048EAB0, 0);
	}
}