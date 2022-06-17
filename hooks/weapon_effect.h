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
	void splash(Bullet* bullet, u32 weapon_id, u8 splashType, Unit* guaranteedHit = nullptr);
	void lurkerSplash(Bullet* bullet, u32 weapon_id, u8 splash);

	void inject_weapon_effect_hooks();
	void normalDamage(Bullet* bullet, u32 weapon_id);
	void yamatoDamage(Bullet* bullet, u32 weapon_id);
	void airSplash(Bullet* bullet, u32 weapon_id);
	void nuke(Bullet* bullet, u32 weapon_id);

	// Cosmonarchy-specific
	bool isChargedGorgolethBullet(Bullet* bullet, u32 weapon_id);
	bool isUnsafeCyprianBullet(Bullet* bullet, u32 weapon_id);
	void parasite(Bullet* bullet, u32 weapon_id);
	void broodlings(Bullet* bullet, u32 weapon_id);
	void lockdown(Bullet* bullet, u32 weapon_id);
}
