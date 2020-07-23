# include "glucustom.h"
#include <hook_tools.h>

namespace {

	void __declspec(naked) raceDropdown_Wrapper() { //004ADF20
		static BinDlg** dialog;
		static u32 a2;
		__asm {
			MOV ESI, dialog
			MOV EBX, a2
			PUSHAD
		}
		hooks::raceDropdown(dialog, a2);
		__asm {
			POPAD
			RETN
		}
	}
	;
	void __declspec(naked) initSingleplayerCustom_Wrapper() { //004AD850
		static BinDlg** dialog;
		__asm {

		}
		hooks::initSingleplayerCustom(dialog);
		__asm {

		}
	}
	;
	void __declspec(naked) initGameLoad_Wrapper() { //004EF100
		static u32 a1;
		static char a2;
		__asm {

		}
		hooks::initGameLoad(a1, a2);
		__asm {

		}
	}
	;
}

namespace hooks {

	void injectGlucustomHooks() {
		jmpPatch(raceDropdown_Wrapper, 0x004ADF20, 1);
		jmpPatch(initSingleplayerCustom_Wrapper, 0x004AD850, 1);
		jmpPatch(initGameLoad_Wrapper, 0x004EF100, 1);
	}
}