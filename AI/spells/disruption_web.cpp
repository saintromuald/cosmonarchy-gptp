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

		return scbw::UnitFinder::getNearestTarget(
			caster->getX() - bounds,
			caster->getY() - bounds,
			caster->getX() + bounds,
			caster->getY() + bounds,
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
				targetOfTarget = target->getFirstLoadedUnit()->orderTarget.unit;
			}
			else
			{
				targetOfTarget = target->orderTarget.unit;
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
				&& caster->getDistanceToTarget(target) <= 32 * 8)
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