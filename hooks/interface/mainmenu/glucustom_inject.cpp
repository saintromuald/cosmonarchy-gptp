#include "glucustom.h"
#include <hook_tools.h>

namespace
{
	void _declspec(naked) displayEstablishingShot_Wrapper() // 004D9200
	{
		_asm
		{
			PUSHAD
		}
		hooks::displayEstablishingShot();
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void injectGlucustomHooks()
	{
		jmpPatch(displayEstablishingShot_Wrapper, 0x004D9200, 1);
	}
}