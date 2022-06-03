#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* findBestRecallTarget(Unit* caster, bool isUnderAttack)
	{

		if (unit_is_in_unsafe_region(caster))
		{
			return nullptr;
		}

		return scbw::UnitFinder::get_nearest_target(caster,
			[&](Unit* target)
		{
			if (target->player != caster->player && !scbw::is_allied_to(target->player, caster->player))
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
