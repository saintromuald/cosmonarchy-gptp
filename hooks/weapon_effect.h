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
	void normal_damage(Bullet* bullet, u32 weapon_id);
	void yamatoDamage(Bullet* bullet, u32 weapon_id);
	void air_splash(Bullet* bullet, u32 weapon_id);
	void nuke(Bullet* bullet, u32 weapon_id);

	// Cosmonarchy-specific
	bool is_charged_gorgrokor_bullet(Bullet* bullet, u32 weapon_id);
	bool is_unsafe_cyprian_bullet(Bullet* bullet, u32 weapon_id);
	void parasite(Bullet* bullet, u32 weapon_id);
	void broodlings(Bullet* bullet, u32 weapon_id);
	void lockdown(Bullet* bullet, u32 weapon_id);
}
