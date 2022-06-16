#pragma once
#include <SCBW/structures/Unit.h>
#include <SCBW/structures/Bullet.h>

namespace hooks
{
	// Custom functions
	bool fire_multiple(Unit* attacker, UnitExt* attackerExt, u32 weapon_id, u32 maxTargets, u32 flags, bool ignoreMain = false);

	// Hooked functions
	void fire_weapon(Unit* unit, u8 weapon_id);
	void progress_bullet_flight(Bullet* a1);
	void bullet_behavior_bounce(Bullet* a1);																	// 0048B2D0
	bool find_next_bounce_target_proc(Unit* pIterator);																// 0048AFD0
	Unit* find_next_bounce_target(Bullet* a1);																		// 0048B1E0
	Bullet* delete_bullet(Bullet* bullet);																		// 0048A560
	Bullet* create_bullet(s32 weapon_id, Unit* parent, s32 x, s16 y, char player, s32 angle);					// 0048C260
	s32 initialize_bullet(char player, Unit* parent, s16 y, Bullet* bullet, s32 weapon_id, s32 x, s32 angle);	// 0048BEC0
	void progress_bullet_frames();																				// 0048C3D0
	void bullet_behavior_reassign(Bullet* bullet);																// 0048BD50
	void get_weapon_iscript_init(Unit* unit, char iscript_anim);												// 00476ED0

	void inject_weapon_fire_hooks();
}