#pragma once
#include <SCBW/structures/Unit.h>
#include <SCBW/structures/Bullet.h>
namespace hooks
{
	// Custom functions
	bool fireMultiple(Unit* attacker, UnitExt* attackerExt, u32 weaponId, u32 maxTargets, u32 flags, bool ignoreMain = false);

	// Hooked functions
	void fireWeaponHook(Unit* unit, u8 weaponId);
	void progressBulletFlight(Bullet* a1);
	void bulletBehaviour_Bounce(Bullet* a1);																	// 0048B2D0
	bool findNextBounceTargetProc(Unit* pIterator);																// 0048AFD0
	Unit* findNextBounceTarget(Bullet* a1);																		// 0048B1E0
	Bullet* deleteBullet(Bullet* bullet);																		// 0048A560
	Bullet* createBullet(s32 weaponId, Unit* parent, s32 x, s16 y, char playerId, s32 angle);					// 0048C260
	s32 initializeBullet(char playerId, Unit* parent, s16 y, Bullet* bullet, s32 weaponId, s32 x, s32 angle);	// 0048BEC0
	void progressBulletFrames();																				// 0048C3D0
	void bulletBehaviorReassign(Bullet* bullet);																// 0048BD50
	void get_weapon_iscript_init(Unit* unit, char iscript_anim);												// 00476ED0

	void injectWeaponFireHooks();
}
