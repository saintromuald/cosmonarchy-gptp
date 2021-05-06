#pragma once
#include <SCBW/scbwdata.h>

namespace SplashType
{
	enum
	{
		Radial = 0,
		Enemy = 1,
		FriendlyNew = 2,
	};
}

namespace hooks
{
	void splash(CBullet* bullet, u32 weaponId, u8 splashType, CUnit* guaranteedHit = nullptr);
	void lurkerSplash(CBullet* bullet, u32 weaponId, u8 splash);

	void injectWeaponEffectHook();
	void normalDamage(CBullet* bullet, u32 weaponId);
	void yamatoDamage(CBullet* bullet, u32 weaponId);
	void airSplash(CBullet* bullet, u32 weaponId);
	void nuke(CBullet* bullet, u32 weaponId);
	s32 irradiateUnitDamageProc(CUnit* target, CUnit* irradiated);

	// HYDRA-specific
	bool isChargedGorgolethBullet(CBullet* bullet, u32 weaponId);
	bool isUnsafeCyprianBullet(CBullet* bullet, u32 weaponId);
	void parasite(CBullet* bullet, u32 weaponId);
	void broodlings(CBullet* bullet, u32 weaponId);
	void lockdown(CBullet* bullet, u32 weaponId);
	CUnit* irradiateDamageLoop(CUnit* unit);						// 004555C0

} //hooks
