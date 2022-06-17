#include "weapon_fire.h"
#include <hook_tools.h>
#include <SCBW/UnitFinder.h>

namespace
{
	void __declspec(naked) fire_weapon_wrapper()
	{
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

		hooks::fire_weapon(unit, weapon_id);

		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) progress_bullet_flight_wrapper()
	{
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

	void __declspec(naked) bullet_behavior_bounce_wrapper()
	{
		static Bullet* a1;
		__asm {
			MOV a1, EDI
			PUSHAD
		}
		hooks::bullet_behavior_bounce(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) find_next_bounce_target_proc_wrapper()
	{
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

	void __declspec(naked) find_next_bounce_target_wrapper()
	{
		static Bullet* a1;
		static Unit* result;
		__asm {
			MOV a1, EAX
			PUSHAD
		}
		result = hooks::find_next_bounce_target(a1);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) delete_bullet_wrapper()
	{
		static Bullet* result;
		static Bullet* bullet;
		__asm
		{
			MOV bullet, EAX
			PUSHAD
		}
		result = hooks::delete_bullet(bullet);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) create_bullet_wrapper() // 0048C260
	{
		static Bullet* result;
		static s32 weapon_id;
		static Unit* parent;
		static s32 x;
		static s16 y;
		static char player;
		static s32 angle;
		__asm
		{
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
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) initialize_bullet_wrapper()
	{
		static char player;
		static Unit* parent;
		static s16 y;
		static Bullet* bullet;
		static s32 weapon_id;
		static s32 x;
		static s32 angle;
		static s32 result;

		__asm
		{
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
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) progress_bullet_frames_wrapper()
	{
		__asm
		{
			PUSHAD
		}
		hooks::progress_bullet_frames();
		__asm
		{
			POPAD
			MOV EAX, 0
			RETN
		}
	}

	void __declspec(naked) bullet_behavior_reassign_wrapper()
	{
		static Bullet* bullet;
		__asm
		{
			MOV bullet, ECX
			PUSHAD
		}
		hooks::bullet_behavior_reassign(bullet);
		__asm
		{
			POPAD
			RETN
		}
	}

	const u32 BulletDamageJmpBack = 0x0048B77A;
	void __declspec(naked) bullet_damage_bullet_nullcheck()
	{
		static Bullet* bullet; 
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			SUB ESP, 8
			PUSH EBX
			MOV EBX, [EBP + 0x08]
			MOV bullet, EBX
			PUSHAD
		}
		if (bullet != nullptr)
		{
			__asm
			{
				POPAD
				JMP BulletDamageJmpBack
			}
		}
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
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
}

extern const u32 Func_FireUnitWeapon;
namespace hooks
{
	void inject_weapon_fire_hooks()
	{
		jump_patch(fire_weapon_wrapper,						Func_FireUnitWeapon, 0);
		jump_patch(progress_bullet_flight_wrapper,			0x0048B3B0, 3);
		jump_patch(bullet_behavior_bounce_wrapper,			0x0048B2D0, 0);
		jump_patch(find_next_bounce_target_proc_wrapper,	0x0048AFD0, 3);
		jump_patch(delete_bullet_wrapper,					0x0048A560, 0);
		jump_patch(create_bullet_wrapper,					0x0048C260, 5);
		jump_patch(initialize_bullet_wrapper,				0x0048BEC0, 0);
	//	jump_patch(progress_bullet_frames_wrapper,			0x0048C3D0, 2);
	//	jump_patch(find_next_bounce_target_wrapper,			0x0048B1E0, 1);
	//	jump_patch(bullet_behavior_reassign_wrapper,		0x00488D50, 1);
		jump_patch(bullet_damage_bullet_nullcheck,			0x0048B770, 1);
	//	jump_patch(get_weapon_iscript_init_wrapper,			0x00476ED0, 2);
	}
}
