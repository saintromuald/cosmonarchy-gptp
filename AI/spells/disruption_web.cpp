#include "spells.h"
#include <AI/ai_common.h>

// V241 for VS2008

namespace AI
{

	Unit* findBestDisruptionWebTarget(Unit* caster, bool isUnderAttack)
	{
		s32 bounds;
		if (isUnderAttack)
			bounds = 32 * 9;
		else if (isUmsMode(caster->player))
			bounds = 32 * 128;
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
			{
				return false;
			}
			Unit* targetOfTarget;
			if (target->id == UnitId::TerranBunker && target->hasLoadedUnit())
			{
				targetOfTarget = target->getFirstLoadedUnit()->order_target.unit;
			}
			else
			{
				targetOfTarget = target->order_target.unit;
			}
			if (!targetOfTarget || targetOfTarget->player >= 8)
			{
				return false;
			}
			if (!(targetOfTarget->status & UnitStatus::InAir))
			{
				return false;
			}
			if (caster->isTargetEnemy(targetOfTarget))
			{
				return false;
			}
			if (target->status & UnitStatus::CanNotAttack)
			{
				return false;
			}
			if (!(target->status & UnitStatus::InAir)
				&& caster->get_distance_to_target(target) <= 32 * 8)
			{
				return true;
			}
			if (target->id == UnitId::TerranWatchdog
				|| target->id == UnitId::TerranBunker
				|| target->id == UnitId::ZergSporeColony
				|| target->id == UnitId::ProtossPhotonCannon)
			{
				return true;
			}
			return false;
		});
	}

} //AI