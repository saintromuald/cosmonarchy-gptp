#pragma once
#include <BW/structures/Unit.h>
#include <BW/structures/Bullet.h>

struct WeaponOffsets {
	u32 value_id;
	u32 count;
	u32 fake_count;
	u32 frameset_offset;
	bool use_special_overlay;
};

struct FireMultipleOptions {
	bool ignore_main = false;
	bool use_weapon_factor = true;
	bool apply_cooldown = true;

	u32 range_overwrite = 0;
	u32 divisor_multiplier = 1;
	s32 stagger = -1; // -1 = no stagger, 0 = animation stagger, 3 = animation stagger + 3 extra frames

	bool center_on_target = false;
	u32 center_on_target_range_overwrite = 0;
};

struct FireBulletOptions {
	bool apply_cooldown = true;
	bool ignore_max_range = false;
	bool dont_follow = false;
	bool ignore_weapon_offsets = false;
};

Target make_target(Unit* unit, Point16 pos = {});
Point8 peek_bullet_offset(Unit* unit, WeaponOffsets* weapon_offsets);
Point8 next_bullet_offset(Unit* unit, WeaponOffsets* weapon_offsets);
Point16 make_launch_pos(Point16 unit_position, Point8 offset);
Bullet* fire_bullet(Unit* unit, WeaponId::Enum weapon_id, Target target, const FireBulletOptions& options = {});
void apply_weapon_cooldown(Unit* unit, WeaponId::Enum weapon_id);
Bullet* create_bullet_ex(WeaponId::Enum weapon_id, Unit* source, s16 x, s16 y, u8 attacking_player, u8 direction, bool apply_cooldown = true);
void fire_line(Unit* unit, WeaponId::Enum weapon_id, u32 count, s32 forward_spread, u32 lines, s32 side_spread);
void fire_sideways(Unit* unit, WeaponId::Enum weapon_id, s32 angle_spread, s32 vertical_scatter);
void fire_shotgun(Unit* unit, WeaponId::Enum weapon_id, u32 side_count, s32 angle_spread, s32 vertical_scatter);
bool fire_multiple(Unit* attacker, WeaponId::Enum weapon_id, u32 max_targets, u32 filters, const FireMultipleOptions& options);

namespace hooks {
	void fire_weapon(Unit* unit, WeaponId::Enum weapon_id);														// 00479C90
	void progress_bullet_flight(Bullet* a1);																	// 0048B3B0
	void bullet_behavior_bounce(Bullet* a1);																	// 0048B2D0
	bool find_next_bounce_target_proc(Unit* pIterator);															// 0048AFD0
	Unit* find_next_bounce_target(Bullet* a1);																	// 0048B1E0
	Bullet* delete_bullet(Bullet* bullet);																		// 0048A560
	Bullet* create_bullet(s32 weapon_id, Unit* parent, s32 x, s16 y, u8 player, s32 angle);						// 0048C260
	s32 initialize_bullet(u8 player, Unit* parent, s16 y, Bullet* bullet, s32 weapon_id, s32 x, s32 angle);		// 0048BEC0
	void progress_bullet_frames();																				// 0048C3D0
	void bullet_behavior_reassign(Bullet* bullet);																// 0048BD50
	void get_weapon_iscript_init(Unit* unit, char iscript_anim);												// 00476ED0
	s32 use_weapon_iscript(Unit* unit, u8 weapon_id, u32 attack_type, char iscript_anim);						// 00476C90
	b32 should_stay_at_range(Unit* unit);																		// 00475A10
	void move_for_melee_range(Unit* target, Unit* attacker);													// 00477F10
	Bullet* assign_weapon_data(Bullet* bullet, char behavior);													// 0048AA80
	void bullet_behavior_follow(Bullet* bullet);																// 0048B500

	void inject_weapon_fire_hooks();
}