//Header file for the Weapon Fire hook module.
#include <SCBW/structures/CUnit.h>
#include <SCBW/structures/CBullet.h>
namespace hooks {

	void fireWeaponHook(CUnit* unit, u8 weaponId);
	void progressBulletFlight(CBullet* a1);
	void bulletBehaviour_Bounce(CBullet* a1); // 0048B2D0
	bool findNextBounceTargetProc(CUnit* pIterator); // 0048AFD0
	CUnit* findNextBounceTarget(CBullet* a1); // 0048B1E0
//	int writeBullets(FILE *a1); // 0048AEB0

	void injectWeaponFireHooks();

} //hooks
