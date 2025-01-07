#pragma once
#include <BW/bwdata.h>

namespace SplashType {
	enum {
		Radial		= 0,
		Enemy		= 1,
		FriendlyNew = 2,
	};
}

namespace hooks {
	void splash(Bullet* bullet, u32 weapon_id, u8 splash_type, Unit* guaranteed_hit = nullptr);
	void lakizilisk_splash(Bullet* bullet, u32 weapon_id, u8 splash);

	void inject_weapon_effect_hooks();
	void normal_damage(Bullet* bullet, u32 weapon_id);
	void yamato_damage(Bullet* bullet, u32 weapon_id);
	void air_splash(Bullet* bullet, u32 weapon_id);
	void nuke(Bullet* bullet, u32 weapon_id);
	b32 splash_target_check(Bullet* bullet, Unit* target);	// 0048B570

	// Cosmonarchy specific
	void parasite(Bullet* bullet, u32 weapon_id);
	void broodlings(Bullet* bullet, u32 weapon_id);
	void lockdown(Bullet* bullet, u32 weapon_id);
}