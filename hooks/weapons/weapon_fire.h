//Header file for the Weapon Fire hook module.
#include <SCBW/structures/Unit.h>
#include <SCBW/structures/Bullet.h>
namespace hooks {

	void fireWeaponHook(Unit* unit, u8 weaponId);
	void progressBulletFlight(Bullet* a1);
	void bulletBehaviour_Bounce(Bullet* a1);																	// 0048B2D0
	bool findNextBounceTargetProc(Unit* pIterator);																// 0048AFD0
	Unit* findNextBounceTarget(Bullet* a1);																		// 0048B1E0
	Bullet* deleteBullet(Bullet* bullet);																		// 0048A560
	Bullet* createBullet(s32 weaponId, Unit* parent, s32 x, s16 y, char playerId, s32 angle);					// 0048C260
	s32 initializeBullet(char playerId, Unit* parent, s16 y, Bullet* bullet, s32 weaponId, s32 x, s32 angle);	// 0048BEC0
//	s32 writeBullets(FILE *a1);																					// 0048AEB0

	void injectWeaponFireHooks();

} //hooks
