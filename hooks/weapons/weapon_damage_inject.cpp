#include "weapon_damage.h"
#include "../../hook_tools.h"

extern const u32 Func_DoWeaponDamage; // Defined in Unit.cpp
namespace
{
	void __declspec(naked) weaponDamageWrapper()
	{
		static Unit* target;
		static Unit* attacker;
		static s32 damage;
		static s32 direction;
		static u32 weaponId;
		static u32 damageDivisor;
		static u32 attackingPlayerId;

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

	void __declspec(naked) applyEnsnare_Wrapper()
	{
		static Unit* a1;
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

	void __declspec(naked) updateCorrosiveSpores_Wrapper() // 004F4480
	{
		static Unit* target;
		static u32 result;
		__asm {
			MOV target, EBX
			PUSHAD
		}
		result = hooks::ApplySpores_hook(target);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void injectWeaponDamageHook()
	{
		jump_patch(weaponDamageWrapper,				0x00479930, 1);
		jump_patch(applyEnsnare_Wrapper,			0x004F45E0, 2);
		jump_patch(updateCorrosiveSpores_Wrapper,	0x004F4480, 1);
	}
}