//The header file for the Weapon/Spell hit hook module.
#pragma once
#include <SCBW/structures/Unit.h>
#include <SCBW/structures/Bullet.h>
#include "SCBW\api.h"

namespace hooks
{
	void IrradiateHit(Unit* attacker, Point16 target, u8 attackingPlayerId);		// 00454E00
	void OpticalFlareHit(Unit* target, u32 attackingPlayerId, Bullet* bullet);		// 00455170
	void RestoreHit(Unit* target, Bullet* bullet);									// 00455230
	void LockdownHit(Unit* target, u8 previousLockdownTimer, Bullet* bullet);		// 00455380
	void weapon_bullet_hit(Bullet* bullet, Unit* target, u32 damageDivisor);			// 00479AE0
	void MeleeAttackHit(Unit* attacker);											// 00479B40
	void EMPShockwaveHit(Unit* attacker, s32 x, s32 y, Bullet* bullet);				// 00492BA0
	void CorrosiveAcidHit(u32 attackingPlayerId, s32 x, s32 y, Bullet* bullet);		// 004F4770
	void ConsumeHit(Unit* attacker, Unit* target);									// 004F47B0
	void EnsnareHit(Unit* attacker, s32 x, s32 y, Bullet* bullet);					// 004F4830
	void DarkSwarmHit(s32 x, s32 y, u32 attackingPlayerId, Bullet* bullet);			// 004F48A0
	void BroodlingHit(Unit* attacker, Unit* target, Bullet* bullet);				// 004F4940
	void PlagueHit(Unit* attacker, s32 x, s32 y, Bullet* bullet);					// 004F4B30
	void MaelstromHit(Unit* attacker, s32 x, s32 y, Bullet* bullet);				// 004F6850
	void MindControlHitHook(Unit* attacker, Unit* target);							// 004F6910
	void StasisFieldHit(Unit* attacker, s32 x, s32 y, Bullet* bullet);				// 004F6A80
	void DisruptionWebHit(u32 attackingPlayerId, s32 x, s32 y, Bullet* bullet);		// 004F6AF0
	Unit* createDisruptionWeb(u32 player, s32 x, s32 y);
	s32 ensnareProc(Unit* target, Unit* attacker);									// 004F46C0
	void createScanner(Unit* unit);													// 00464E40
	u8 stasisFieldOverlay(Unit* a1, u8 a2);
	char disableUnit(Unit* unit);													// 00492CC0
	bool maelstromProc(Unit* target, Unit* source);									// 004F6760
	Unit* iterateUnitsAtLocationTargetProc(Box16* location,							// 004E8280
		Unit*(__fastcall* proc)(Unit*, Unit*),
		Unit* unit);
	b32 giveUnitToPlayer(Unit* unit, u32 player);									// 004C8040

	void inject_weapon_spell_hit_hooks();
}

// Hooks could be confirmed thanks to 0048B770 CBullet_Damage that call almost all of them
// Hit Features not supported by these hooks may be hardcoded in that function (like the Parasite Hit effect)