#include "hooks/limits/extender.h"
#include "hooks/limits/ccmu.h"
#include "hooks/limits/sfx_extender.h"
#include "logger.h"
#include "globals.h"
namespace hooks {
	void loadAiseSamaseData() {
		hooks::sfxExtender();
	}
	const u32 jmp_004DAF36 = 0x004DAF36;
	void __declspec(naked) preInitDataWrapper() {
		__asm {
			PUSHAD
		}
		loadAiseSamaseData();
		__asm {
			POPAD
			PUSH 0x004D2700
			JMP jmp_004DAF36
		}
	}
	void injectLimitHooks() {

//		hooks::injectCCMUHooks();
//		hooks::sfxExtender();
//		jmpPatch(preInitDataWrapper, 0x004DAF31);
		/*
		auto str = scbw::readSamaseFile("samase\\sfx_ext.txt");
		GPTP::logger << "Read Test Len: " << str.size() << '\n';*/
	}
}
