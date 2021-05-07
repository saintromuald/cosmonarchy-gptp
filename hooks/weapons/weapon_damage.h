#pragma once
#include "../../SCBW/structures/Unit.h"

void augurSacrifice(CUnit* target);
namespace hooks {

	void weaponDamageHook(
		s32 damage,
		CUnit* target,
		u8 weaponId,
		CUnit* attacker,
		u8 attackingPlayerId,
		s8 direction,
		u8 dmgDivisor
	);

	struct WeaponDamageData {
		const s32 damage;
		s32 finalDamage;
		s32 shieldReduceAmount;
		u8 damageType;
		u8 damageDivisor;
		u8 weaponId;
		s8 direction;

		CUnit* target;
		CUnit* attacker;
		u8 attackingPlayerId;
		bool dontProcPhaseLink;
		bool ignoreFlatArmor;
	};

	void weaponDamage_root(WeaponDamageData& data);

	void applyEnsnare(CUnit* a1);
	u16 GetSporesOverlayId(CUnit* target);
	s32 ApplySpores_hook(CUnit* target); // 004F4480

	bool GorgonCondition(CUnit* attacker, CUnit* target);

	void injectWeaponDamageHook();

} //hooks
