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
	void splash(Bullet* bullet, u32 weaponId, u8 splashType, Unit* guaranteedHit = nullptr);
	void lurkerSplash(Bullet* bullet, u32 weaponId, u8 splash);

	void inject_weapon_effect_hooks();
	void normalDamage(Bullet* bullet, u32 weaponId);
	void yamatoDamage(Bullet* bullet, u32 weaponId);
	void airSplash(Bullet* bullet, u32 weaponId);
	void nuke(Bullet* bullet, u32 weaponId);

	// Cosmonarchy-specific
	bool isChargedGorgolethBullet(Bullet* bullet, u32 weaponId);
	bool isUnsafeCyprianBullet(Bullet* bullet, u32 weaponId);
	void parasite(Bullet* bullet, u32 weaponId);
	void broodlings(Bullet* bullet, u32 weaponId);
	void lockdown(Bullet* bullet, u32 weaponId);
}
