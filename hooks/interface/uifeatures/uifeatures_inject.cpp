#include "uifeatures.h"
#include "hook_tools.h"

namespace {
	const u32 SetFontHandle = 0x0041F610;
	const u32 drawUIFeatures_jmp = 0x0048D0AE;
	void _declspec(naked) drawUIFeatures_wrapper() {
		_asm {
			PUSHAD
		}
		hooks::drawUIFeatures();
		_asm {
			POPAD
			CALL SetFontHandle
			JMP drawUIFeatures_jmp
		}
	}

	void __declspec(naked) targetedOrder_Wrapper() {
		static __int16 a1;
		static int a2;
		static CUnit* a3;
		static __int16 a4;
		static int a5;
		static int a6;
		static char a7;
		static int a8;
		static int a9;
		static int a10;

		__asm {
			PUSHAD
			MOVZX DX, a1
			MOV EBX, a2
			MOV ESI, a3
			MOVZX AX, a4
			PUSH a5
			PUSH a6
			PUSH a7
			PUSH a8
			PUSH a9
		}
		hooks::targetedOrder(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		__asm {
			POPAD
		}
	}

	void __declspec(naked) selectUnits_Wrapper() {
		static CUnit** a1;
		static int a2;
		static char a3;
		static int a4;
		static signed int result;

		__asm {
			PUSHAD
			PUSH a1
			PUSH a2
			PUSH a3
			PUSH a4
		}
		result = hooks::selectUnits(a1, a2, a3, a4);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks {
	void injectUIFeaturesHooks() {
		jmpPatch(drawUIFeatures_wrapper, 0x0048D0A9, 0);
		jmpPatch(targetedOrder_Wrapper, 0x0048D0A9, 0);
		jmpPatch(selectUnits_Wrapper, 0x0048D0A9, 0);
	}
}