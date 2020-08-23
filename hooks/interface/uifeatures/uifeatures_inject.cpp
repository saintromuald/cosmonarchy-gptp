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

	u8 _declspec(naked) QueueGameCommand_wrapper() {
		static const void* a1;
		static u32 a2;
		static u8 result;
		_asm {
			MOV a1, ECX
			MOV a2, EDX
			PUSHAD
		}
		hooks::QueueGameCommand(a1, a2);
		_asm {
			POPAD
			RETN
		}
		return result;
	}

	void __declspec(naked) targetedOrder_Wrapper() {
		static s16 a1;
		static s32 a2;
		static CUnit* a3;
		static s16 a4;
		static s32 a5;
		static s32 a6;
		static s32 a7;
		static s32 a8;
		static s32 a9;
		static s32 a10;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV a1, DX
			MOV a2, EBX
			MOV a3, ESI
			MOV a4, AX
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV a5, EAX
			MOV EAX, [EBP + 0xC]
			MOV a6, EAX
			MOV EAX, [EBP + 0x10]
			MOV a7, EAX
			MOV EAX, [EBP + 0x14]
			MOV a8, EAX
			MOV EAX, [EBP + 0x18]
			MOV a9, EAX
			MOV EAX, [EBP + 0x1C]
			MOV a10, EAX
			POP EAX
			PUSHAD
		}
		hooks::targetedOrder(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x18
		}
	}

	void __declspec(naked) selectUnits_Wrapper() {
		static CUnit** a1;
		static s32 a2;
		static s32 a3;
		static s32 a4;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV a1, EBX
			MOV a2, EDI
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV a3, EAX
			MOV EAX, [EBP + 0xC]
			MOV a4, EAX
			POP EAX
			PUSHAD
		}
		hooks::selectUnits(a1, a2, a3, a4);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}
}

namespace hooks {
	void injectUIFeaturesHooks() {
		jmpPatch(drawUIFeatures_wrapper, 0x0048D0A9, 0);
		//jmpPatch(targetedOrder_Wrapper, 0x004754F0, 0);
		//jmpPatch(selectUnits_Wrapper, 0x0046FA00, 0);
		jmpPatch(QueueGameCommand_wrapper, 0x00485BD0, 4);
	}
}