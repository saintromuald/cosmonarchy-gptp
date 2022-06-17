#pragma once
#include "../../SCBW/structures/Unit.h"

struct {
	s32 damage_type;
	s32 unit_size_factor[5];	 // Order: {independent, small, medium, large, titanic}
} const damage_factors[] = {
	{0, 0, 0,	0,	 0,   0},			// Independent
	{1, 0, 128, 192, 256, 192},			// Explosive
	{2, 0, 256, 192, 128, 64},			// Concussive
	{3, 0, 256, 256, 192, 128},			// Normal
	{4, 0, 256, 256, 256, 256},			// IgnoreArmor
	{5, 0, 256, 256, 256, 256},			// Chronal
};

bool darkswarm_blocks_damage(Unit* target, u32 weapon_id, u32 damage_divisor);

namespace hooks {

	void weapon_damage(
		s32 damage,
		Unit* target,
		u8 weapon_id,
		Unit* attacker,
		u8 attackingPlayerId,
		s8 direction,
		u8 damage_divisor
	);

	void weapon_damage_custom(
		s32 damage,
		Unit* target,
		u8 damage_type,
		Unit* attacker,
		u8 attackingPlayerId,
		s8 direction,
		u8 damage_divisor,
		bool ignore_flat_armor
	);

	struct WeaponDamageData {
		const s32 damage;
		s32 final_damage;
		s32 shield_reduce_amount;
		u8 damage_type;
		u8 damage_divisor;
		u16 weapon_id;
		s8 direction;

		Unit* target;
		Unit* attacker;
		u8 attackingPlayerId;
		bool dont_proc_link_damage;
		bool ignore_flat_armor;
	};

	void weapon_damage_root(WeaponDamageData& data);

	void apply_ensnare(Unit* a1);
	u16 get_spores_overlay_id(Unit* target);
	s32 apply_spores_hook(Unit* target);		// 004F4480
	void unit_attack_notify(Unit* unit);		// 0048F230

	void inject_weapon_damage_hooks();
}
