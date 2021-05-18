#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* findBestRecallTarget(Unit* caster, bool isUnderAttack)
	{

		if (isUnitInUnsafeRegion(caster))
		{
			return nullptr;
		}

		return scbw::UnitFinder::getNearestTarget(caster,
			[&](Unit* target)
		{
			if (target->playerId != caster->playerId && !scbw::isAlliedTo(target->playerId, caster->playerId))
			{
				return false;
			}

			if (!target->orderTarget.unit)
			{
				return false;
			}

			if (target->id == UnitId::ProtossSolarion
				|| target->id == UnitId::ProtossAccantor)
			{
				if (target->getCurrentShieldsInGame() <= 10)
				{
					return true;
				}
			}

			return false;
		});
	}
}
