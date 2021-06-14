#include "weapon_spellhit.h"
#include <hook_tools.h>
#include <globals.h>

namespace
{
	void __declspec(naked) IrradiateHitWrapper()
	{
		static Unit* attacker;
		static Unit* target;
		static u8 attackingPlayerId;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV attacker, EAX
			MOV AL, [EBP + 0x0C]
			MOV attackingPlayerId, AL
			MOV target, EDI
			PUSHAD
		}
		hooks::IrradiateHit(attacker, target->position, attackingPlayerId);
		__asm
		{
			POPAD
			POP EBP
			RETN 8
		}
	}

	void __declspec(naked) OpticalFlareHitWrapper()
	{
		static Unit* target;
		static u32 attackingPlayerId;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV bullet, EBX
			MOV EBX, [EBP + 0x08]
			MOV attackingPlayerId, EBX
			MOV target, EAX
			PUSHAD
		}
		hooks::OpticalFlareHit(target, attackingPlayerId, bullet);
		__asm
		{
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) RestoreHitWrapper()
	{
		static Unit* target;
		static Bullet* bullet;
		__asm
		{
			MOV bullet, EAX
			MOV target, EBX
			PUSHAD
		}
		hooks::RestoreHit(target, bullet);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) LockdownHitWrapper()
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
		hooks::LockdownHit(target, previousLockdownTimer, bullet);
		__asm
		{
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) MeleeAttackHitWrapper()
	{
		static Unit* attacker;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV attacker, EAX
			PUSHAD
		}
		hooks::MeleeAttackHit(attacker);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) WeaponBulletHitWrapper()
	{
		static Bullet* bullet;
		static Unit* target;
		static u32 hitFlags;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV bullet, EDX
			MOV target, EAX
			MOV EAX, [EBP + 0x08]
			MOV hitFlags, EAX
			PUSHAD
		}
		hooks::WeaponBulletHit(bullet, target, hitFlags);
		__asm
		{
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) EMPShockwaveHitWrapper()
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
		hooks::EMPShockwaveHit(attacker, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) CorrosiveAcidHitWrapper()
	{
		static s32 x;
		static s32 y;
		static u32 playerId;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV x, ECX
			MOV y, EAX
			MOV bullet, EBX
			MOV EAX, [EBP + 0x08]
			MOV playerId, EAX
			PUSHAD
		}
		hooks::CorrosiveAcidHit(playerId, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) ConsumeHitWrapper()
	{
		static Unit* attacker;
		static Unit* target;
		__asm
		{
			MOV target, EAX
			MOV attacker, ESI
			PUSHAD
		}
		hooks::ConsumeHit(attacker, target);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) EnsnareHitWrapper()
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
		hooks::EnsnareHit(attacker, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) DarkSwarmHitWrapper()
	{
		static s32 x;
		static s32 y;
		static u32 attackingPlayerId;
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
			MOV attackingPlayerId, EAX
			MOV bullet, EBX
			PUSHAD
		}
		hooks::DarkSwarmHit(x, y, attackingPlayerId, bullet);
		__asm
		{
			POPAD
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) BroodlingHitWrapper()
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
		hooks::BroodlingHit(attacker, target, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) PlagueHitWrapper()
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
		hooks::PlagueHit(attacker, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) MaelstromHitWrapper()
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
		hooks::MaelstromHit(attacker, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) MindControlHitWrapper()
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
		hooks::MindControlHitHook(attacker, target);
		__asm
		{
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) StasisFieldHitWrapper()
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
		hooks::StasisFieldHit(attacker, x, y, bullet);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) DisruptionWebHitWrapper()
	{
		static s32 x;
		static s32 y;
		static u32 playerId;
		static Bullet* bullet;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV x, ECX
			MOV y, EAX
			MOV bullet, EBX
			MOV EAX, [EBP + 0x08]
			MOV playerId, EAX
			PUSHAD
		}
		hooks::DisruptionWebHit(playerId, x, y, bullet);
		__asm
		{
			POPAD
			POP EBP
			RETN 0x0C
		}
	}

	s32 __declspec(naked) ensnareProcWrapper()
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
		hooks::ensnareProc(target, attacker);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) createScannerWrapper()
	{
		static Unit* a1;
		__asm
		{
			MOV a1, EAX
			PUSHAD
		}
		hooks::createScanner(a1);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) stasisFieldOverlayWrapper()
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
		a2 = hooks::stasisFieldOverlay(unit, a2);
		__asm
		{
			POPAD
			MOV EAX, a2
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) disableUnit_Wrapper() // 00492CC0
	{
		static char result;
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::disableUnit(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) maelstromProc_Wrapper() // 004F6760
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
		result = hooks::maelstromProc(target, source);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) iterateUnitsAtLocationTargetProc_Wrapper() // 004E8280
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
		result = hooks::iterateUnitsAtLocationTargetProc(location, proc, unit);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) giveUnitToPlayer_Wrapper()
	{
		static Unit* unit;
		static u32 playerId;
		static b32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV playerId, EAX
			MOV EAX, [EBP + 0x0C]
			MOV unit, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::giveUnitToPlayer(unit, playerId);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}
}

namespace hooks
{
    void injectWpnSpellHitHooks()
    {
        jmpPatch(IrradiateHitWrapper, 0x00454E00, 4);
        jmpPatch(OpticalFlareHitWrapper, 0x00455170, 1);
        jmpPatch(RestoreHitWrapper, 0x00455230, 6);
        jmpPatch(LockdownHitWrapper, 0x00455380, 4);
        jmpPatch(WeaponBulletHitWrapper, 0x00479AE0, 1);
        jmpPatch(MeleeAttackHitWrapper, 0x00479B40, 0);
        jmpPatch(EMPShockwaveHitWrapper, 0x00492BA0, 1);
        jmpPatch(CorrosiveAcidHitWrapper, 0x004F4770, 1);
        jmpPatch(ConsumeHitWrapper, 0x004F47B0, 0);
        jmpPatch(EnsnareHitWrapper, 0x004F4830, 1);
        jmpPatch(DarkSwarmHitWrapper, 0x004F48A0, 2);
        jmpPatch(BroodlingHitWrapper, 0x004F4940, 1);
        jmpPatch(PlagueHitWrapper, 0x004F4B30, 1);
        jmpPatch(MaelstromHitWrapper, 0x004F6850, 1);
        jmpPatch(MindControlHitWrapper, 0x004F6910, 8);
        jmpPatch(StasisFieldHitWrapper, 0x004F6A80, 1);
        jmpPatch(DisruptionWebHitWrapper, 0x004F6AF0, 2);
        jmpPatch(ensnareProcWrapper, 0x004F46C0, 1);
        jmpPatch(createScannerWrapper, 0x00464E40, 0);
        jmpPatch(stasisFieldOverlayWrapper, 0x004F67B0, 4);
        jmpPatch(disableUnit_Wrapper, 0x00492CC0, 2);
        jmpPatch(maelstromProc_Wrapper, 0x004F6760, 3);
//      jmpPatch(iterateUnitsAtLocationTargetProc_Wrapper, 0x004F6760, 0);
//		jmpPatch(giveUnitToPlayer_Wrapper, 0x004C8040, 2);
    }
}