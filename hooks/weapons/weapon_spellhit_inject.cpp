#include "weapon_spellhit.h"
#include <hook_tools.h>
#include <globals.h>

namespace
{
	void __declspec(naked) irradiate_hit_wrapper()
	{
		static Unit* attacker;
		static Unit* target;
		static u8 attacking_player_id;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV attacker, EAX
			MOV AL, [EBP + 0x0C]
			MOV attacking_player_id, AL
			MOV target, EDI
			PUSHAD
		}
		hooks::irradiate_hit(attacker, target->position, attacking_player_id);
		__asm
		{
			POPAD
			POP EBP
			RETN 8
		}
	}

	void __declspec(naked) optical_flare_hit_wrapper() {
		static Unit* target;
		static u32 attacking_player_id;
		static Bullet* bullet;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV attacking_player_id, EAX
			POP EAX
			MOV target, EAX
			MOV bullet, EBX
			PUSHAD
		}
		hooks::optical_flare_hit(target, attacking_player_id, bullet);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) restore_hit_wrapper()
	{
		static Unit* target;
		static Bullet* bullet;
		__asm
		{
			MOV bullet, EAX
			MOV target, EBX
			PUSHAD
		}
		hooks::restore_hit(target, bullet);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) lockdown_hit_wrapper()
	{
		static Unit* target;
		static u8 previousLockdownTimer;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV target, EDI
			MOV AL, [EBP + 0x08]
			MOV previousLockdownTimer, AL
			MOV bullet, EBX
			PUSHAD
		}
		hooks::lockdown_hit(target, previousLockdownTimer, bullet);
		__asm
		{
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) iscript_attack_melee_wrapper()
	{
		static Unit* attacker;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV attacker, EAX
			PUSHAD
		}
		hooks::iscript_attackmelee(attacker);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) weapon_bullet_hit_wrapper()
	{
		static Bullet* bullet;
		static Unit* target;
		static u32 hit_flags;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV bullet, EDX
			MOV target, EAX
			MOV EAX, [EBP + 0x08]
			MOV hit_flags, EAX
			PUSHAD
		}
		hooks::weapon_bullet_hit(bullet, target, hit_flags);
		__asm
		{
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) emp_showckwave_hit_wrapper()
	{
		static Unit* attacker;
		static Bullet* bullet;
		static s32 x;
		static s32 y;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV y, EAX
			MOV x, ECX
			MOV EAX, [EBP + 0x08]
			MOV attacker, EAX
			MOV bullet, EBX
			PUSHAD
		}
		hooks::emp_shockwave_hit(attacker, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) corrosive_acid_hit_wrapper()
	{
		static s32 x;
		static s32 y;
		static u32 player;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV x, ECX
			MOV y, EAX
			MOV bullet, EBX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			PUSHAD
		}
		hooks::corrosive_acid_hit(player, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) consume_hit_wrapper()
	{
		static Unit* attacker;
		static Unit* target;
		__asm
		{
			MOV target, EAX
			MOV attacker, ESI
			PUSHAD
		}
		hooks::consume_hit(attacker, target);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) ensnare_hit_wrapper()
	{
		static Unit* attacker;
		static s32 x;
		static s32 y;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV x, ECX
			MOV y, EAX
			MOV EAX, [EBP + 0x08]
			MOV attacker, EAX
			MOV bullet, EBX
			PUSHAD
		}
		hooks::ensnare_hit(attacker, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) dark_swarm_hit_wrapper()
	{
		static s32 x;
		static s32 y;
		static u32 attacking_player_id;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV x, EAX
			MOV EAX, [EBP + 0x0C]
			MOV y, EAX
			MOV EAX, [EBP + 0x10]
			MOV attacking_player_id, EAX
			MOV bullet, EBX
			PUSHAD
		}
		hooks::dark_swarm_hit(x, y, attacking_player_id, bullet);
		__asm
		{
			POPAD
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) broodling_hit_wrapper()
	{
		static Unit* attacker;
		static Unit* target;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV attacker, EAX
			MOV EAX, [EBP + 0x0C]
			MOV target, EAX
			MOV bullet, EBX
			PUSHAD
		}
		hooks::broodling_hit(attacker, target, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) plague_hit_wrapper()
	{
		static Unit* attacker;
		static s32 x;
		static s32 y;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV x, ECX
			MOV y, EAX
			MOV EAX, [EBP + 0x08]
			MOV attacker, EAX
			MOV bullet, EBX
			PUSHAD
		}
		hooks::plague_hit(attacker, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) maelstrom_hit_wrapper()
	{
		static Unit* attacker;
		static s32 x;
		static s32 y;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV y, EAX
			MOV EAX, [EBP + 0x08]
			MOV x, EAX
			MOV EAX, [EBP + 0x0C]
			MOV attacker, EAX
			MOV bullet, EBX
			PUSHAD
		}
		hooks::maelstrom_hit(attacker, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) mind_control_hit_wrapper()
	{
		static Unit* attacker;
		static Unit* target;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV target, ESI
			MOV EAX, [EBP + 0x08]
			MOV attacker, EAX
			PUSHAD
		}
		hooks::mind_control_hit(attacker, target);
		__asm
		{
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) stasis_field_hit_wrapper()
	{
		static Unit* attacker;
		static s32 x;
		static s32 y;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV y, EAX
			MOV x, ECX
			MOV EAX, [EBP + 0x08]
			MOV attacker, EAX
			MOV bullet, EBX
			PUSHAD
		}
		hooks::stasis_field_hit(attacker, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) disruption_web_hit_wrapper()
	{
		static s32 x;
		static s32 y;
		static u32 player;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV x, ECX
			MOV y, EAX
			MOV bullet, EBX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			PUSHAD
		}
		hooks::disruption_web_hit(player, x, y, bullet);
		__asm
		{
			POPAD
			POP EBP
			RETN 0x0C
		}
	}

	s32 __declspec(naked) ensnare_proc_wrapper()
	{
		static Unit* target;
		static Unit* attacker;
		static s32 result;
		__asm
		{
			MOV target, ECX
			MOV attacker, EDX
			PUSHAD
		}
		hooks::ensnare_proc(target, attacker);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) create_scanner_wrapper()
	{
		static Unit* a1;
		__asm
		{
			MOV a1, EAX
			PUSHAD
		}
		hooks::create_scanner(a1);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) stasis_field_overlay_wrapper()
	{
		static Unit* unit;
		static u32 a2;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EDI
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV a2, EAX
			POP EAX
			PUSHAD
		}
		a2 = hooks::stasis_field_overlay(unit, a2);
		__asm
		{
			POPAD
			MOV EAX, a2
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) disable_unit_wrapper() // 00492CC0
	{
		static char result;
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::disable_unit(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) maelstrom_proc_wrapper() // 004F6760
	{
		static bool result;
		static Unit* target;
		static Unit* source;
		__asm
		{
			MOV source, EDX
			MOV target, ECX
			PUSHAD
		}
		result = hooks::maelstrom_proc(target, source);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) iterate_units_at_location_target_proc_wrapper() // 004E8280
	{
		static Unit* result;
		static Box16* location;
		static Unit*(__fastcall *proc)(Unit*, Unit*);
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			MOV proc, EBX
			MOV location, EAX
			PUSHAD
		}
		result = hooks::iterate_units_at_location_target_proc(location, proc, unit);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) give_unit_to_player_wrapper()
	{
		static Unit* unit;
		static u32 player;
		static b32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			MOV EAX, [EBP + 0x0C]
			MOV unit, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::give_unit_to_player(unit, player);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}

	void __declspec(naked) set_maelstrom_timer_wrapper() // 004553F0
	{
		static Unit* unit;
		static u32 duration;
		static char result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV duration, EAX
			POP EAX
			MOV unit, EDI
			PUSHAD
		}
		result = hooks::set_maelstrom_timer(unit, duration);
		__asm
		{
			POPAD
			MOV AL, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}
}

namespace hooks
{
    void inject_weapon_spell_hit_hooks()
    {
        jump_patch(irradiate_hit_wrapper,							0x00454E00, 4);
        jump_patch(optical_flare_hit_wrapper,						0x00455170, 1);
        jump_patch(restore_hit_wrapper,								0x00455230, 6);
        jump_patch(lockdown_hit_wrapper,							0x00455380, 4);
        jump_patch(weapon_bullet_hit_wrapper,						0x00479AE0, 1);
        jump_patch(iscript_attack_melee_wrapper,					0x00479B40, 0);
        jump_patch(emp_showckwave_hit_wrapper,						0x00492BA0, 1);
        jump_patch(corrosive_acid_hit_wrapper,						0x004F4770, 1);
        jump_patch(consume_hit_wrapper,								0x004F47B0, 0);
        jump_patch(ensnare_hit_wrapper,								0x004F4830, 1);
        jump_patch(dark_swarm_hit_wrapper,							0x004F48A0, 2);
        jump_patch(broodling_hit_wrapper,							0x004F4940, 1);
        jump_patch(plague_hit_wrapper,								0x004F4B30, 1);
        jump_patch(maelstrom_hit_wrapper,							0x004F6850, 1);
        jump_patch(mind_control_hit_wrapper,						0x004F6910, 8);
        jump_patch(stasis_field_hit_wrapper,						0x004F6A80, 1);
        jump_patch(disruption_web_hit_wrapper,						0x004F6AF0, 2);
        jump_patch(ensnare_proc_wrapper,							0x004F46C0, 1);
        jump_patch(create_scanner_wrapper,							0x00464E40, 0);
        jump_patch(stasis_field_overlay_wrapper,					0x004F67B0, 4);
        jump_patch(disable_unit_wrapper,							0x00492CC0, 2);
        jump_patch(maelstrom_proc_wrapper,							0x004F6760, 3);
	//	jump_patch(iterate_units_at_location_target_proc_wrapper,	0x004F6760, 0);
	//	jump_patch(give_unit_to_player_wrapper,						0x004C8040, 2);
		jump_patch(set_maelstrom_timer_wrapper,						0x004553F0, 4);

		// Remove optical flare target check
		nops(0x0048BBC7, 2);
		nops(0x0048BBCB, 5);
		nops(0x0048BBD2, 2);
    }
}