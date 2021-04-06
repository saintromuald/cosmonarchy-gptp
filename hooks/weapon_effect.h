#pragma once
#include <SCBW/scbwdata.h>

namespace SplashType {
	enum {
		Radial = 0,
		Enemy = 1,
		FriendlyNew = 2,
	};
}

namespace hooks {

	void friendlySplash(CBullet* bullet, uint32 weaponId, uint8 splash, CUnit* guaranteed_hit = nullptr);
	void friendlyLurkerSplash(CBullet* bullet, uint32 weaponId, uint8 splash);

	void injectWeaponEffectHook();
	void normalDamage(CBullet* bullet, uint32 weaponId);
	void yamatoDamage(CBullet* bullet, uint32 weaponId);
	void airSplash(CBullet* bullet, uint32 weaponId);

	// HYDRA-specific
	bool isChargedGorgolethBullet(CBullet* bullet, uint32 weaponId);
	bool isUnsafeCyprianBullet(CBullet* bullet, uint32 weaponId);
	void parasite(CBullet* bullet, uint32 weaponId);
	void broodlings(CBullet* bullet, uint32 weaponId);
	void lockdown(CBullet* bullet, uint32 weaponId);

} //hooks
