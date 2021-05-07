#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{

	Unit* findBestRecallTarget(Unit* caster, bool isUnderAttack)
	{

		if (isUnitInUnsafeRegion(caster))
			return nullptr;

		return scbw::UnitFinder::getNearestTarget(caster,
			[&](Unit* target)
		{

			if (target->playerId != caster->playerId && !scbw::isAlliedTo(target->playerId, caster->playerId))
				return false;

			if (!target->orderTarget.unit)
				return false;

			if (target->id == UnitId::carrier
				|| target->id == UnitId::gantrithor
				|| target->id == UnitId::reaver
				|| target->id == UnitId::warbringer)
			{
				if (target->getCurrentShieldsInGame() <= 10)
					return true;
			}

			return false;

		});
	}

} //AI
