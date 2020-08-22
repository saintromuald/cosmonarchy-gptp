#include "stats_panel_display.h"
#include <hook_tools.h>

namespace {

	void __declspec(naked) stats_panel_display_Wrapper() {

		static BinDlg* dialog;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP+0x08]
			MOV dialog, EAX
			PUSHAD
		}

		hooks::stats_panel_display(dialog);

		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}

	};

	void __declspec(naked) getUnitRankString_Wrapper() { // 0042F370
		static CUnit* a1;
		static char* result;
		__asm {
			MOV a1, ESI
			PUSHAD
		}
		result = hooks::getUnitRankString(a1);
		__asm {
			POPAD
			MOV EAX,result
			RETN
		}
	}

}//unnamed namespace

namespace hooks {

	void injectStatsPanelDisplayHook() {
		jmpPatch(stats_panel_display_Wrapper, 0x00426C60, 1);
		//jmpPatch(getUnitRankString_Wrapper, 0x0042F370, 1);
	}

} //hooks
