#include "weapon_damage.h"
#include "../../hook_tools.h"

extern const u32 Func_DoWeaponDamage; // Defined in Unit.cpp
namespace
{
	void __declspec(naked) weapon_damage_wrapper()
	{
		static Unit* target;
		static Unit* attacker;
		static s32 damage;
		static s32 direction;
		static u32 weapon_id;
		static u32 damage_divisor;
		static u32 attackingPlayerId;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV damage, EAX
			MOV target, EDI
			MOV EAX, [EBP + 0x08]
			MOV weapon_id, EAX
			MOV EAX, [EBP + 0x0C]
			MOV damage_divisor, EAX
			MOV EAX, [EBP + 0x10]
			MOV direction, EAX
			MOV EAX, [EBP + 0x14]
			MOV attacker, EAX
			MOV EAX, [EBP + 0x18]
			MOV attackingPlayerId, EAX
			PUSHAD
		}
		hooks::weapon_damage(damage, target, weapon_id, attacker, attackingPlayerId,
			direction, damage_divisor);
		__asm {
			POPAD
			POP EBP
			RETN 0x14
		}
	}

	void __declspec(naked) apply_ensnare_wrapper()
	{
		static Unit* a1;
		__asm {
			MOV a1, EDI
			PUSHAD
		}
		hooks::apply_ensnare(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) update_corrosive_spores_wrapper() // 004F4480
	{
		static Unit* target;
		static u32 result;
		__asm {
			MOV target, EBX
			PUSHAD
		}
		result = hooks::apply_spores_hook(target);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unit_attack_notify_wrapper() { // 0048F230
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			PUSHAD
		}
		hooks::unit_attack_notify(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}
}

namespace hooks
{
	void inject_weapon_damage_hooks()
	{
		jump_patch(weapon_damage_wrapper,				0x00479930, 1);
		jump_patch(apply_ensnare_wrapper,				0x004F45E0, 2);
		jump_patch(update_corrosive_spores_wrapper,		0x004F4480, 1);
	//	jump_patch(unit_attack_notify_wrapper,			0x0048F230, 3);
	}
}