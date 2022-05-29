#include "file_operations.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) openMapFile_Wrapper()
	{
		static s32 a1; // needs documentation
		static bool result;
		__asm {
			MOV a1, EAX
			PUSHAD
		}
		result = hooks::openMapFile(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) loadMap_Wrapper()
	{
		static s32 result;
		static s32 a1;
		static char* a2;
		static s32 a3;
		__asm {
			MOV a3, ESI
			MOV a2, EDI
			MOV a1, ECX
			PUSHAD
		}
		result = hooks::loadMap(a1, a2, a3);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void injectFileOperationsHooks()
	{
		jump_patch(openMapFile_Wrapper, 0x0045A290, 4);
		jump_patch(loadMap_Wrapper,		0x004BF520, 4);
	}
}
