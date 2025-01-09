#include "file_operations.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) open_map_file_wrapper() {
		static s32 a1; // needs documentation
		static bool result;
		__asm {
			MOV a1, EAX
			PUSHAD
		}
		result = hooks::open_map_file(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) load_map_wrapper() {
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
		result = hooks::load_map(a1, a2, a3);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks {
	void injectFileOperationsHooks() {
		jump_patch(open_map_file_wrapper,	0x0045A290, 4);
		jump_patch(load_map_wrapper,		0x004BF520, 4);
	}
}