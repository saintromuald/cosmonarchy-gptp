#include "spells.h"
#include <AI/ai_common.h>

// V241 for VS2008

namespace AI
{

	Unit* findBestEmpShockwaveTarget(Unit* caster, bool isUnderAttack)
	{

		s32 bounds;

		if (isUnderAttack)
			bounds = 32 * 9;
		else
			bounds = 32 * 64;

		Unit* result = scbw::UnitFinder::getNearestTarget(
			caster->getX() - bounds,
			caster->getY() - bounds,
			caster->getX() + bounds,
			caster->getY() + bounds,
			caster,
			[&](Unit* target)
		{
			if (!isTargetWorthHitting(target, caster))
				return false;

			if (!units_dat::ShieldsEnabled[target->id])
				return false;

			if (!scbw::canWeaponTargetUnit(WeaponId::EMP_Shockwave, target, caster))
				return false;

			const s32 totalEnemyShields = getTotalEnemyShieldsInArea(target->getX(), target->getY(), 160, caster);
			if (totalEnemyShields >= 200)
				return true;

			return false;
		});

		if (result || isUnderAttack)
			return result;

		return scbw::UnitFinder::getNearestTarget(
			caster->getX() - bounds,
			caster->getY() - bounds,
			caster->getX() + bounds,
			caster->getY() + bounds,
			caster,
			[&](Unit* target)
		{
			if (!isTargetWorthHitting(target, caster))
				return false;

			if (!target->is_valid_caster())
				return false;

			if (!scbw::canWeaponTargetUnit(WeaponId::EMP_Shockwave, target, caster))
				return false;

			const s32 totalEnemyEnergy = getTotalEnemyEnergyInArea(target->getX(), target->getY(), 160, caster);
			if (totalEnemyEnergy >= 200)
				return true;

			return false;
		});
	}

} //AI