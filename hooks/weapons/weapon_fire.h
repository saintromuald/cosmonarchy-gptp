//Header file for the Weapon Fire hook module.
#include <SCBW/structures/Unit.h>
#include <SCBW/structures/Bullet.h>
namespace hooks {

	void fireWeaponHook(Unit* unit, u8 weaponId);
	void progressBulletFlight(Bullet* a1);
	void bulletBehaviour_Bounce(Bullet* a1); // 0048B2D0
	bool findNextBounceTargetProc(Unit* pIterator); // 0048AFD0
	Unit* findNextBounceTarget(Bullet* a1); // 0048B1E0
//	s32 writeBullets(FILE *a1); // 0048AEB0

	void injectWeaponFireHooks();

} //hooks
