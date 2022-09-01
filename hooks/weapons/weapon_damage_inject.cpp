#include "weapon_damage.h"
#include "../../hook_tools.h"

extern const u32 Func_DoWeaponDamage; // Defined in Unit.cpp
namespace
{
	void __declspec(naked) weapon_damage_wrapper()
	{
		static u32 result;
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
		if (direction == 33333) {
			result = parse_aise_signal(attacker, target, (AiseSignal)damage_divisor, weapon_id, attackingPlayerId, damage);
		} else {
			hooks::weapon_damage(damage, target, weapon_id, attacker, attackingPlayerId,
				direction, damage_divisor);
			result = 0;
		}
		__asm {
			POPAD
			POP EBP
			MOV EAX, result
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

	void __declspec(naked) unit_was_hit_wrapper() { // 004795D0
		static Unit* victim;
		static Unit* attacker;
		static u32 reveal;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV attacker, EAX
			MOV EAX, [EBP + 0x0C]
			MOV reveal, EAX
			POP EAX
			MOV victim, EBX
			PUSHAD
		}
		hooks::unit_was_hit(victim, attacker, reveal);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) ask_for_help_wrapper() { // 00476E60
		static u32 result;
		static Unit* attacker;
		static Unit* helper;
		__asm {
			MOV helper, ESI
			MOV attacker, EAX
			PUSHAD
		}
		result = hooks::ask_for_help(attacker, helper);
		__asm {
			POPAD
			MOV EAX, result
			RETN
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
		jump_patch(unit_attack_notify_wrapper,			0x0048F230, 3);
		jump_patch(unit_was_hit_wrapper,				0x004795D0, 1);
		jump_patch(ask_for_help_wrapper,				0x00476E60, 2);
	}
}