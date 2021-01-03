//Injector source file for the Hallucination Spell Order hook module.
#include "hallucination_spell.h"
#include <hook_tools.h>

namespace {

	void __declspec(naked) orders_Hallucination1Wrapper() {
		static CUnit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EDI
			PUSHAD
		}
		hooks::orders_Hallucination1(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) createHallucination_Wrapper() {
		static CUnit* source;
		static int player;
		static CUnit* result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP+0x08]
			MOV player, EAX
			POP EAX
			MOV source, ECX
			PUSHAD
		}
		result = hooks::createHallucination(source, player);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

}//unnamed namespace

namespace hooks {

	void injectHallucinationSpellHook() {
		jmpPatch(orders_Hallucination1Wrapper,		0x004F6C40, 0);
		jmpPatch(createHallucination_Wrapper,		0x004F6B90, 2);
	}

} //hooks