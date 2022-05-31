#include "spells.h"
#include <AI/ai_common.h>
#include <globals.h>

//V241 for VS2008

namespace AI
{

	Unit* findBestRestorationTarget(Unit* caster, bool isUnderAttack)
	{

		s32 bounds = 32 * 32;

		return scbw::UnitFinder::getNearestTarget(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
			caster,
			[&](Unit* target)
		{

			if (target->player != caster->player)
				return false;

			if (!scbw::can_weapon_target_unit(WeaponId::Restoration, target, caster))
				return false;

			if (target->get_current_life_ingame() <= 60)
				return false;

			if (scbw::isDebuffed(target))
				return true;

			return false;

		});
	}


	Unit* findBestRestorationTargetSituational(Unit* caster, bool isUnderAttack)
	{

		if (isUnderAttack)
			return nullptr;

		return scbw::UnitFinder::getNearestTarget(caster, [&](Unit* target)
		{

			if (target->player != caster->player)
				return false;

			if (!(target->parasiteFlags || target->maelstromTimer))
				return false;

			if (!scbw::can_weapon_target_unit(WeaponId::Restoration, target, caster))
				return false;

			return true;

		});
	}

} //AI
