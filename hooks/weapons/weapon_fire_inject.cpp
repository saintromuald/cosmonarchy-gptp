#include "weapon_fire.h"
#include <hook_tools.h>
#include <BW/UnitFinder.h>
#include "BW/api.h"

namespace {
	void __declspec(naked) fire_weapon_wrapper() {
		static Unit* unit;
		static u8 weapon_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOVZX EAX, [EBP + 0x08]
			MOV weapon_id, AL
			MOV unit, ESI
			PUSHAD
		}
		hooks::fire_weapon(unit, (WeaponId::Enum)weapon_id);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) progress_bullet_flight_wrapper() {
		static Bullet* bullet;
		__asm {
			MOV bullet, EAX
			PUSHAD
		}
		hooks::progress_bullet_flight(bullet);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) bullet_behavior_bounce_wrapper() {
		static Bullet* bullet;
		__asm {
			MOV bullet, EDI
			PUSHAD
		}
		hooks::bullet_behavior_bounce(bullet);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) find_next_bounce_target_proc_wrapper() {
		static Unit* pIterator;
		static bool result;
		__asm {
			MOV pIterator, ECX
			PUSHAD
		}
		result = hooks::find_next_bounce_target_proc(pIterator);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) find_next_bounce_target_wrapper() {
		static Bullet* bullet;
		static Unit* result;
		__asm {
			MOV bullet, EAX
			PUSHAD
		}
		result = hooks::find_next_bounce_target(bullet);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) delete_bullet_wrapper() {
		static Bullet* result;
		static Bullet* bullet;
		__asm {
			MOV bullet, EAX
			PUSHAD
		}
		result = hooks::delete_bullet(bullet);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) create_bullet_wrapper() { // 0048C260
		static Bullet* result;
		static s32 weapon_id;
		static Unit* parent;
		static s32 x;
		static s16 y;
		static char player;
		static s32 angle;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV x, EAX
			MOV EAX, [EBP + 0xC]
			MOV y, AX
			MOV EAX, [EBP + 0x10]
			MOV player, AL
			MOV EAX, [EBP + 0x14]
			MOV angle, EAX
			POP EAX
			MOV parent, EAX
			MOV weapon_id, ECX
			PUSHAD
		}
		result = hooks::create_bullet(weapon_id, parent, x, y, player, angle);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) initialize_bullet_wrapper() {
		static u8 player;
		static Unit* parent;
		static s16 y;
		static Bullet* bullet;
		static s32 weapon_id;
		static s32 x;
		static s32 angle;
		static s32 result;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV bullet, EAX
			MOV EAX, [EBP + 0x0C]
			MOV weapon_id, EAX
			MOV EAX, [EBP + 0x10]
			MOV x, EAX
			MOV EAX, [EBP + 0x14]
			MOV angle, EAX
			POP EAX
			MOV player, CL
			MOV parent, EAX
			MOV y, DX
			PUSHAD
		}
		result = hooks::initialize_bullet(player, parent, y, bullet, weapon_id, x, angle);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) progress_bullet_frames_wrapper() {
		__asm {
			PUSHAD
		}
		hooks::progress_bullet_frames();
		__asm {
			POPAD
			MOV EAX, 0
			RETN
		}
	}

	void __declspec(naked) bullet_behavior_reassign_wrapper() { // 0048BD50
		static Bullet* bullet;
		__asm {
			MOV bullet, ECX
			PUSHAD
		}
		hooks::bullet_behavior_reassign(bullet);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) get_weapon_iscript_init_wrapper() { // 00476ED0
		static Unit* unit;
		static char iscript_anim;
		__asm {
			MOV iscript_anim, DL
			MOV unit, EAX
			PUSHAD
		}
		hooks::get_weapon_iscript_init(unit, iscript_anim);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) use_weapon_iscript_wrapper() { // 00476C90
		static s32 result;
		static Unit* unit;
		static u8 weapon_id;
		static u32 attack_type;
		static char iscript_anim;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV iscript_anim, AL
			POP EAX
			MOV attack_type, EDI
			MOV weapon_id, BL
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::use_weapon_iscript(unit, weapon_id, attack_type, iscript_anim);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) should_stay_at_range_wrapper() { // 00475A10
		static b32 result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::should_stay_at_range(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) move_for_melee_range_wrapper() { // 00477F10
		static Unit* target;
		static Unit* attacker;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV attacker, EAX
			POP EAX
			MOV target, EAX
			PUSHAD
		}
		hooks::move_for_melee_range(target, attacker);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) assign_weapon_data_wrapper() { // 0048AA80
		static Bullet* bullet;
		static char behavior;
		static Bullet* result;
		__asm {
			MOV bullet, EAX
			MOV behavior, DL
			PUSHAD
		}
		result = hooks::assign_weapon_data(bullet, behavior);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) bullet_behavior_follow_wrapper() { // 0048B500
		static Bullet* bullet;
		__asm {
			MOV bullet, EAX
			PUSHAD
		}
		hooks::bullet_behavior_follow(bullet);
		__asm {
			POPAD
			RETN
		}
	}

	// PATCHES \\

	const u32 bullet_damage_jumpback = 0x0048B77A;
	void __declspec(naked) bullet_damage_bullet_nullcheck() {
		static Bullet* bullet;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 8
			PUSH EBX
			MOV EBX, [EBP + 0x08]
			MOV bullet, EBX
			PUSHAD
		}
		if (bullet) {
			__asm {
				POPAD
				JMP bullet_damage_jumpback
			}
		}
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) unit_id_is_melee_air_unit_wrapper() {
		u32 unit_id;
		static bool is_air_unit;
		__asm {
			MOVZX	EAX, word ptr[ESI + 0x64]
			MOV		unit_id, EAX
		}
		is_air_unit = unit_id_is_melee_air_unit(unit_id);
		__asm {
			CMP		is_air_unit, 1
			RET
		}
	}
}

namespace hooks
{
	void inject_weapon_fire_hooks()
	{
		jump_patch(fire_weapon_wrapper,						0x00479C90, 0);
		jump_patch(progress_bullet_flight_wrapper,			0x0048B3B0, 3);
		jump_patch(bullet_behavior_bounce_wrapper,			0x0048B2D0, 0);
		jump_patch(find_next_bounce_target_proc_wrapper,	0x0048AFD0, 3);
		jump_patch(delete_bullet_wrapper,					0x0048A560, 0);
		jump_patch(create_bullet_wrapper,					0x0048C260, 5);
		jump_patch(initialize_bullet_wrapper,				0x0048BEC0, 0);
	//	jump_patch(progress_bullet_frames_wrapper,			0x0048C3D0, 2);
	//	jump_patch(find_next_bounce_target_wrapper,			0x0048B1E0, 1);
		jump_patch(bullet_behavior_reassign_wrapper,		0x0048BD50, 1); // Not fully functional
	//	jump_patch(get_weapon_iscript_init_wrapper,			0x00476ED0, 2);
		jump_patch(use_weapon_iscript_wrapper,				0x00476C90, 1);
		jump_patch(should_stay_at_range_wrapper,			0x00475A10, 3);
		jump_patch(move_for_melee_range_wrapper,			0x00477F10, 0);
		jump_patch(assign_weapon_data_wrapper,				0x0048AA80, 0);
		jump_patch(bullet_behavior_follow_wrapper,			0x0048B500, 0);
		
		jump_patch(bullet_damage_bullet_nullcheck,			0x0048B770, 1);
		call_patch(unit_id_is_melee_air_unit_wrapper,		0x00475A10, 3);
		call_patch(unit_id_is_melee_air_unit_wrapper,		0x004788AB);
	}
}