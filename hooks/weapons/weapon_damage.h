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

bool darkswarm_blocks_damage(Unit* target, u32 weaponId, u32 damageDivisor);

namespace hooks {

	void weapon_damage(
		s32 damage,
		Unit* target,
		u8 weaponId,
		Unit* attacker,
		u8 attackingPlayerId,
		s8 direction,
		u8 damageDivisor
	);

	void weapon_damage_custom(
		s32 damage,
		Unit* target,
		u8 damageType,
		Unit* attacker,
		u8 attackingPlayerId,
		s8 direction,
		u8 damageDivisor,
		bool ignoreFlatArmor
	);

	struct WeaponDamageData {
		const s32 damage;
		s32 finalDamage;
		s32 shieldReduceAmount;
		u8 damageType;
		u8 damageDivisor;
		u16 weaponId;
		s8 direction;

		Unit* target;
		Unit* attacker;
		u8 attackingPlayerId;
		bool dontProcLinkDamage;
		bool ignoreFlatArmor;
	};

	void weapon_damage_root(WeaponDamageData& data);

	void apply_ensnare(Unit* a1);
	u16 get_spores_overlay_id(Unit* target);
	s32 apply_spores_hook(Unit* target);		// 004F4480
	void unit_attack_notify(Unit* unit);		// 0048F230

	void inject_weapon_damage_hooks();
}
