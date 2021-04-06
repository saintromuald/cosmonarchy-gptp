#include "file_operations.h"
#include <hook_tools.h>

namespace {

	void __declspec(naked) openMapFile_Wrapper() {
		static int32 a1; // needs documentation
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

	void __declspec(naked) loadMap_Wrapper() {
		static int32 result;
		static int32 a1;
		static char* a2;
		static int32 a3;
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

}//unnamed namespace

namespace hooks {

	void injectFileOperationsHooks() {
		jmpPatch(openMapFile_Wrapper, 0x0045A290, 4);
		jmpPatch(loadMap_Wrapper, 0x004BF520, 4);
	}

} //hooks
