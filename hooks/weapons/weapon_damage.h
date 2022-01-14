#pragma once
#include "../../SCBW/structures/Unit.h"

bool darkswarm_blocks_damage(Unit* target, u32 weaponId, u32 damageDivisor);

namespace hooks {

	void weaponDamageHook(
		s32 damage,
		Unit* target,
		u8 weaponId,
		Unit* attacker,
		u8 attackingPlayerId,
		s8 direction,
		u8 damageDivisor
	);

	void weaponDamageCustom(
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
		bool dontProcPhaseLink;
		bool ignoreFlatArmor;
	};

	void weaponDamage_root(WeaponDamageData& data);

	void applyEnsnare(Unit* a1);
	u16 GetSporesOverlayId(Unit* target);
	s32 ApplySpores_hook(Unit* target); // 004F4480

	void injectWeaponDamageHook();

} //hooks
