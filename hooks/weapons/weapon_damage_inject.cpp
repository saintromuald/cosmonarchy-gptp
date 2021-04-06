#include "weapon_damage.h"
#include "../../hook_tools.h"

extern const uint32 Func_DoWeaponDamage; //Defined in CUnit.cpp

namespace {

	void __declspec(naked) weaponDamageWrapper() {
		static CUnit* target;
		static CUnit* attacker;
		static int32 damage;
		static int32 direction;
		static uint32 weaponId;
		static uint32 damageDivisor;
		static uint32 attackingPlayerId;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV damage, EAX
			MOV target, EDI
			MOV EAX, [EBP + 0x08]
			MOV weaponId, EAX
			MOV EAX, [EBP + 0x0C]
			MOV damageDivisor, EAX
			MOV EAX, [EBP + 0x10]
			MOV direction, EAX
			MOV EAX, [EBP + 0x14]
			MOV attacker, EAX
			MOV EAX, [EBP + 0x18]
			MOV attackingPlayerId, EAX
			PUSHAD
		}
		hooks::weaponDamageHook(damage, target, weaponId, attacker, attackingPlayerId,
			direction, damageDivisor);
		__asm {
			POPAD
			POP EBP
			RETN 0x14
		}
	}

	void __declspec(naked) applyEnsnare_Wrapper() {
		static CUnit* a1;
		__asm {
			MOV a1, EDI
			PUSHAD
		}
		hooks::applyEnsnare(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) updateCorrosiveSpores_Wrapper() { // 004F4480
		static CUnit* target;
		static uint32_t result;
		__asm {
			MOV target, EBX
			PUSHAD
		}
		result = hooks::updateCorrosiveSpores(target);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

} //unnamed namespace

namespace hooks {
	void injectWeaponDamageHook() {
		jmpPatch(weaponDamageWrapper, 0x00479930, 1);
		jmpPatch(applyEnsnare_Wrapper, 0x004F45E0, 2);
		jmpPatch(updateCorrosiveSpores_Wrapper, 0x004F4480, 1);
	}
} //hooks
