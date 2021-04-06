#include "glucustom.h"
#include <hook_tools.h>

namespace {
	/*
	void __declspec(naked) raceDropdown_Wrapper() { //004ADF20
		static BinDlg** dialog;
		__asm {
			MOV dialog, EAX
			PUSHAD
		}
		hooks::raceDropdown(dialog);
		__asm {
			POPAD
			RETN
		}
	};
	int __declspec(naked) initSingleplayerCustom_Wrapper() { //004AD850
		static BinDlg** dialog;
		static int result;
		__asm {
			MOV dialog, EAX
			PUSHAD
		}
		result = hooks::initSingleplayerCustom(dialog);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	};
	void __declspec(naked) initGameLoad_Wrapper() { //004EF100
		static uint32 a1;
		static char a2;
		static int result;
		__asm {
			MOV a1, ESI
			MOV a2, EDI
			PUSHAD
		}
		result = hooks::initGameLoad(a1, a2);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	};*/

	void _declspec(naked) displayEstablishingShot_Wrapper() { // 004D9200
		_asm {
			PUSHAD
		}
		hooks::displayEstablishingShot();
		__asm {
			POPAD
			RETN
		}
	}


}

namespace hooks {

	void injectGlucustomHooks() {
		//		jmpPatch(raceDropdown_Wrapper, 0x004ADF20, 1);
		//		jmpPatch(initSingleplayerCustom_Wrapper, 0x004AD850, 1);
		//		jmpPatch(initGameLoad_Wrapper, 0x004EF100, 1);
		jmpPatch(displayEstablishingShot_Wrapper, 0x004D9200, 1);
	}

}