#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{

	Unit* findBestParasiteTarget(Unit* caster, bool isUnderAttack)
	{

		s32 bounds;

		if (isUnderAttack)
			bounds = 32 * 9;
		else
			bounds = 32 * 64;

		return scbw::UnitFinder::get_nearest_target(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
			caster,
			[&](Unit* target)
		{

			if (!isTargetWorthHitting(target, caster))
				return false;

			if (isUmsMode(caster->player) && target->parasiteFlags)
				return false;

			if (target->parasiteFlags & (1 << caster->player))
				return false;

			if (!scbw::can_weapon_target_unit(WeaponId::Parasite, target, caster))
				return false;

			if (target->canDetect())
				return true;

			if (!(target->status & UnitStatus::IsHallucination)
				&& (target->id != UnitId::ZergOvileth || scbw::get_upgrade_level(target->player, UpgradeId::VentralSacs))
				&& units_dat::SpaceProvided[target->id] > 0)
				return true;

			if (target->is_valid_caster())
				return true;

			if (units_dat::BaseProperty[target->id] & UnitProperty::Worker)
				return true;

			if (target->get_current_life_ingame() >= 300)
				return true;

			return false;

		});
	}


} //AI
