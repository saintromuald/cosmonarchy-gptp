//The header file for the Weapon/Spell hit hook module.
#pragma once
#include <SCBW/structures/CUnit.h>
#include <SCBW/structures/CBullet.h>
#include "SCBW\api.h"
namespace hooks {
	//	void IrradiateHit(CUnit* attacker, CUnit* target, u8 attackingPlayerId);		// 00454E00
	void OpticalFlareHit(CUnit* target, u32 attackingPlayerId, CBullet* bullet);		// 00455170
	void RestoreHit(CUnit* target, CBullet* bullet);									// 00455230
	void LockdownHit(CUnit* target, u8 previousLockdownTimer, CBullet* bullet);			// 00455380
	void WeaponBulletHit(CBullet* bullet, CUnit* target, u32 damageDivisor);			// 00479AE0
	void MeleeAttackHit(CUnit* attacker);												// 00479B40
	void EMPShockwaveHit(CUnit* attacker, s32 x, s32 y, CBullet* bullet);				// 00492BA0
	void CorrosiveAcidHit(u32 attackingPlayerId, s32 x, s32 y, CBullet* bullet);		// 004F4770
	void ConsumeHit(CUnit* attacker, CUnit* target);									// 004F47B0
	void EnsnareHit(CUnit* attacker, s32 x, s32 y, CBullet* bullet);					// 004F4830
	void DarkSwarmHit(s32 x, s32 y, u32 attackingPlayerId, CBullet* bullet);			// 004F48A0
	void BroodlingHit(CUnit* attacker, CUnit* target, CBullet* bullet);					// 004F4940
	void PlagueHit(CUnit* attacker, s32 x, s32 y, CBullet* bullet);						// 004F4B30
	void MaelstromHit(CUnit* attacker, s32 x, s32 y, CBullet* bullet);					// 004F6850
	void MindControlHitHook(CUnit* attacker, CUnit* target);							// 004F6910
	void StasisFieldHit(CUnit* attacker, s32 x, s32 y, CBullet* bullet);				// 004F6A80
	void DisruptionWebHit(u32 attackingPlayerId, s32 x, s32 y, CBullet* bullet);		// 004F6AF0
	CUnit* createDisruptionWeb(u32 playerId, s32 x, s32 y);
	s32 ensnareProc(CUnit* target, CUnit* attacker);									// 004F46C0
	void createScanner(CUnit* a1);														// 00464E40
	u8 stasisFieldOverlay(CUnit* a1, u8 a2);
	char disableUnit(CUnit* unit);														// 00492CC0
	bool maelstromProc(CUnit* target, CUnit* source);									// 004F6760
	CUnit* iterateUnitsAtLocationTargetProc(Box16* location,							// 004E8280
		s32(__fastcall* proc)(CUnit*, CUnit*),
		CUnit* unit); 

	//The injector function for this hook module.
	void injectWpnSpellHitHooks();
}

//Hooks could be confirmed thanks to 0048B770 CBullet_Damage that call almost all of them
//Hit Features not supported by these hooks may be hardcoded in that function (like
//the Parasite Hit effect)
