#include "weapon_fire.h"
#include <hook_tools.h>
#include <SCBW/UnitFinder.h>

namespace
{
	void __declspec(naked) fireWeaponWrapper()
	{
		static Unit* unit;
		static u8 weaponId;

		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOVZX EAX, [EBP + 0x08]
			MOV weaponId, AL
			MOV unit, ESI
			PUSHAD
		}

		hooks::fireWeaponHook(unit, weaponId);

		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) progressBulletFlight_Wrapper()
	{
		static Bullet* bullet;
		__asm {
			MOV bullet, EAX
			PUSHAD
		}
		hooks::progressBulletFlight(bullet);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) bulletBehaviour_Bounce_Wrapper()
	{
		static Bullet* a1;
		__asm {
			MOV a1, EDI
			PUSHAD
		}
		hooks::bulletBehaviour_Bounce(a1);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) findNextBounceTargetProc_Wrapper()
	{
		static Unit* pIterator;
		static bool result;
		__asm {
			MOV pIterator, ECX
			PUSHAD
		}
		result = hooks::findNextBounceTargetProc(pIterator);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) findNextBounceTarget_Wrapper()
	{
		static Bullet* a1;
		static Unit* result;
		__asm {
			MOV a1, EAX
			PUSHAD
		}
		result = hooks::findNextBounceTarget(a1);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) deleteBullet_Wrapper()
	{
		static Bullet* result;
		static Bullet* bullet;
		__asm
		{
			MOV bullet, EAX
			PUSHAD
		}
		result = hooks::deleteBullet(bullet);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) createBullet_Wrapper() // 0048C260
	{
		static Bullet* result;
		static s32 weaponId;
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
			MOV weaponId, ECX
			PUSHAD
		}
		result = hooks::create_bullet(weaponId, parent, x, y, player, angle);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) initializeBullet_Wrapper()
	{
		static char player;
		static Unit* parent;
		static s16 y;
		static Bullet* bullet;
		static s32 weaponId;
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
			MOV weaponId, EAX
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
		result = hooks::initializeBullet(player, parent, y, bullet, weaponId, x, angle);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) progressBulletFrames_Wrapper()
	{
		__asm
		{
			PUSHAD
		}
		hooks::progressBulletFrames();
		__asm
		{
			POPAD
			MOV EAX, 0
			RETN
		}
	}

	void __declspec(naked) bulletBehaviorReassign_Wrapper()
	{
		static Bullet* bullet;
		__asm
		{
			MOV bullet, ECX
			PUSHAD
		}
		hooks::bulletBehaviorReassign(bullet);
		__asm
		{
			POPAD
			RETN
		}
	}

	const u32 BulletDamageJmpBack = 0x0048B77A;
	void __declspec(naked) BulletDamage_bulletNullcheck()
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
		jump_patch(fireWeaponWrapper,					Func_FireUnitWeapon, 0);
		jump_patch(progressBulletFlight_Wrapper,		0x0048B3B0, 3);
		jump_patch(bulletBehaviour_Bounce_Wrapper,		0x0048B2D0, 0);
		jump_patch(findNextBounceTargetProc_Wrapper,	0x0048AFD0, 3);
		jump_patch(deleteBullet_Wrapper,				0x0048A560, 0);
		jump_patch(createBullet_Wrapper,				0x0048C260, 5);
		jump_patch(initializeBullet_Wrapper,			0x0048BEC0, 0);
	//	jump_patch(progressBulletFrames_Wrapper,		0x0048C3D0, 2);
	//	jump_patch(findNextBounceTarget_Wrapper,		0x0048B1E0, 1);
	//	jump_patch(bulletBehaviorReassign_Wrapper,		0x00488D50, 1);
		jump_patch(BulletDamage_bulletNullcheck,		0x0048B770, 1);
	//	jump_patch(get_weapon_iscript_init_wrapper,		0x00476ED0, 2);
	}
}
