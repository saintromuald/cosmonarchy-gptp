//The header file for the Weapon/Spell hit hook module.
#pragma once
#include <SCBW/structures/CUnit.h>
#include <SCBW/structures/CBullet.h>
#include "SCBW\api.h"
namespace hooks {

	//	void IrradiateHit(CUnit* attacker, CUnit* target, uint8 attackingPlayerId);		// 00454E00
	void OpticalFlareHit(CUnit* target, uint32 attackingPlayerId, CBullet* bullet);		// 00455170
	void RestoreHit(CUnit* target, CBullet* bullet);									// 00455230
	void LockdownHit(CUnit* target, uint8 previousLockdownTimer, CBullet* bullet);		// 00455380
	void WeaponBulletHit(CBullet* bullet, CUnit* target, uint32 damageDivisor);			// 00479AE0
	void MeleeAttackHit(CUnit* attacker);												// 00479B40
	void EMPShockwaveHit(CUnit* attacker, int x, int y, CBullet* bullet);				// 00492BA0
	void CorrosiveAcidHit(uint32 attackingPlayerId, int x, int y, CBullet* bullet);		// 004F4770
	void ConsumeHit(CUnit* attacker, CUnit* target);									// 004F47B0
	void EnsnareHit(CUnit* attacker, int x, int y, CBullet* bullet);					// 004F4830
	void DarkSwarmHit(int x, int y, uint32 attackingPlayerId, CBullet* bullet);			// 004F48A0
	void BroodlingHit(CUnit* attacker, CUnit* target, CBullet* bullet);					// 004F4940
	void PlagueHit(CUnit* attacker, int x, int y, CBullet* bullet);						// 004F4B30
	void MaelstromHit(CUnit* attacker, int x, int y, CBullet* bullet);					// 004F6850
	void MindControlHit(CUnit* attacker, CUnit* target);								// 004F6910
	void StasisFieldHit(CUnit* attacker, int x, int y, CBullet* bullet);				// 004F6A80
	void DisruptionWebHit(uint32 attackingPlayerId, int x, int y, CBullet* bullet);		// 004F6AF0
	CUnit* createDisruptionWeb(uint32 playerId, int x, int y);
	int ensnareProc(CUnit* target, CUnit* attacker);									// 004F46C0
	void createScanner(CUnit* a1);														// 00464E40
	uint8 stasisFieldOverlay(CUnit* a1, uint8 a2);

	//The injector function for this hook module.
	void injectWpnSpellHitHooks();

} //hooks

//Hooks could be confirmed thanks to 0048B770 CBullet_Damage that call almost all of them
//Hit Features not supported by these hooks may be hardcoded in that function (like
//the Parasite Hit effect)
