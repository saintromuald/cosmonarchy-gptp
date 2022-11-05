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
	void splash(Bullet* bullet, u32 weapon_id, u8 splash_type, Unit* guaranteed_hit = nullptr);
	void lakizilisk_splash(Bullet* bullet, u32 weapon_id, u8 splash);

	void inject_weapon_effect_hooks();
	void normal_damage(Bullet* bullet, u32 weapon_id);
	void yamato_damage(Bullet* bullet, u32 weapon_id);
	void air_splash(Bullet* bullet, u32 weapon_id);
	void nuke(Bullet* bullet, u32 weapon_id);

	// Cosmonarchy BW specific
	bool is_charged_gorgrokor_bullet(Bullet* bullet, u32 weapon_id);
	void parasite(Bullet* bullet, u32 weapon_id);
	void broodlings(Bullet* bullet, u32 weapon_id);
	void lockdown(Bullet* bullet, u32 weapon_id);
}
