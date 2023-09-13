#pragma once
#include "../../SCBW/structures/Unit.h"

extern Bullet* weapon_damage_current_bullet;
static bool ignore_alert;

void apply_ensnare_internal(Unit* unit, u32 ensnare_factor);
bool carapace_swarm_blocks_damage(Unit* target, u32 weapon_id, u32 damage_divisor);

namespace hooks {
	void weapon_damage(
		s32 damage,
		Unit* target,
		u8 weapon_id,
		Unit* attacker,
		u8 attacking_player_id,
		s8 direction,
		u8 damage_divisor
	);

	void weapon_damage_custom(
		s32 damage,
		Unit* target,
		u32 armor_pen,
		Unit* attacker,
		u8 attacking_player_id,
		s8 direction,
		u8 damage_divisor,
		bool ignore_flat_armor
	);

	struct WeaponDamageData {
		s32 damage;
		s32 shield_reduce_amount;
		u32 bonus_armor_pen;
		u8 damage_type;
		u8 damage_divisor;
		u16 weapon_id;
		s8 direction;

		Unit* target;
		Unit* attacker;
		u8 attacking_player_id;
		bool dont_proc_link_damage;
		bool ignore_flat_armor;
	};

	void weapon_damage_root(WeaponDamageData& data);

	void apply_ensnare(Unit* unit);									// 004F45E0
	u16 get_spores_overlay_id(Unit* target);
	s32 apply_spores_hook(Unit* target);							// 004F4480
	void unit_attack_notify(Unit* unit);							// 0048F230
	void unit_was_hit(Unit* victim, Unit* attacker, u32 reveal);	// 004795D0
	u32 ask_for_help(Unit* attacker, Unit* helper);					// 00476E60
	void react_to_hit(Unit* attacker, Unit* victim);				// 004776C0

	void inject_weapon_damage_hooks();
}