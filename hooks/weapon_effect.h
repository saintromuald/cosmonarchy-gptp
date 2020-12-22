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
  
	void friendlySplash(CBullet* bullet, u32 weaponId, u8 splash, CUnit* guaranteed_hit=NULL);
	void friendlyLurkerSplash(CBullet* bullet, u32 weaponId, u8 splash);

	void injectWeaponEffectHook();
	void normalDamage(CBullet* bullet, u32 weaponId);
	void yamatoDamage(CBullet* bullet, u32 weaponId);
	void airSplash(CBullet* bullet, u32 weaponId);

	// HYDRA-specific
	bool isChargedGorgolethBullet(CBullet* bullet, u32 weaponId);
	bool isUnsafeCyprianBullet(CBullet* bullet, u32 weaponId);
	void parasite(CBullet* bullet, u32 weaponId);
	void broodlings(CBullet* bullet, u32 weaponId);
	void lockdown(CBullet* bullet, u32 weaponId);

} //hooks
