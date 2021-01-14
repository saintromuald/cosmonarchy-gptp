#include "file_operations.h"
#include <hook_tools.h>

namespace {
	
	void __declspec(naked) openMapFile_Wrapper() {
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

}//unnamed namespace

namespace hooks {

	void injectFileOperationsHooks() {
		jmpPatch(openMapFile_Wrapper, 0x0045A290, 4);
	}

} //hooks
