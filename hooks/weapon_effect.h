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
void splash(Bullet* bullet, WeaponId::Enum weapon_id, u8 splash_type, Unit* guaranteed_hit = nullptr);
void lakizilisk_splash(Bullet* bullet, WeaponId::Enum weapon_id, u8 splash);

void inject_weapon_effect_hooks();
void normal_damage(Bullet* bullet, WeaponId::Enum weapon_id);
void yamato_damage(Bullet* bullet, WeaponId::Enum weapon_id);
void air_splash(Bullet* bullet, WeaponId::Enum weapon_id);
void nuke(Bullet* bullet, WeaponId::Enum weapon_id);
bool splash_target_check(Bullet* bullet, Unit* target);	// 0048B570
bool air_splash_proc(Unit* target, Bullet* bullet);     // 0048B150
bool lakiz_splash_proc(Unit* target, Bullet* bullet);   // 0048B680

// Cosmonarchy specific
void parasite(Bullet* bullet, WeaponId::Enum weapon_id);
void broodlings(Bullet* bullet, WeaponId::Enum weapon_id);
void lockdown(Bullet* bullet, WeaponId::Enum weapon_id);
} // namespace hooks