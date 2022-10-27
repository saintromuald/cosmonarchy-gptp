#pragma once
#include <SCBW/structures/Unit.h>
#include <SCBW/structures/Bullet.h>

void apply_cooldown(Unit* unit, u8 weapon_id);
Bullet* create_bullet_and_apply_cooldown(u8 weapon_id, Unit* source, s16 x, s16 y, u8 attackingPlayer, u8 direction);

namespace hooks
{
	// Custom functions
	void launch_missile(Unit* attacker, Unit* target, u8 weapon_id);
	bool fire_multiple(Unit* attacker, u32 weapon_id, u32 max_targets, u32 flags, bool ignore_main = false, bool use_weapon_factor = true);

	// Hooked functions
	void fire_weapon(Unit* unit, u8 weapon_id);																	// 00479C90
	void progress_bullet_flight(Bullet* a1);																	// 0048B3B0
	void bullet_behavior_bounce(Bullet* a1);																	// 0048B2D0
	bool find_next_bounce_target_proc(Unit* pIterator);															// 0048AFD0
	Unit* find_next_bounce_target(Bullet* a1);																	// 0048B1E0
	Bullet* delete_bullet(Bullet* bullet);																		// 0048A560
	Bullet* create_bullet(s32 weapon_id, Unit* parent, s32 x, s16 y, char player, s32 angle);					// 0048C260
	s32 initialize_bullet(char player, Unit* parent, s16 y, Bullet* bullet, s32 weapon_id, s32 x, s32 angle);	// 0048BEC0
	void progress_bullet_frames();																				// 0048C3D0
	void bullet_behavior_reassign(Bullet* bullet);																// 0048BD50
	void get_weapon_iscript_init(Unit* unit, char iscript_anim);												// 00476ED0
	s32 use_weapon_iscript(Unit* unit, u8 weapon_id, u32 attack_type, char iscript_anim);						// 00476C90

	void inject_weapon_fire_hooks();
}