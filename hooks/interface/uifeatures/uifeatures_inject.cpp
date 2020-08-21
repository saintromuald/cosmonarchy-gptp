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
}

namespace hooks {
	void injectUIFeaturesHooks() {
		jmpPatch(drawUIFeatures_wrapper, 0x0048D0A9, 0);
	}
}