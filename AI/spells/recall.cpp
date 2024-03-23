#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* find_best_recall_target(Unit* caster, bool isUnderAttack)
	{

		if (unit_is_in_unsafe_region(caster))
		{
			return nullptr;
		}

		return bw::UnitFinder::get_nearest_target(caster,
			[&](Unit* target)
		{
			if (target->player != caster->player && !bw::is_allied_to(target->player, caster->player))
			{
				return false;
			}

			if (!target->order_target.unit)
			{
				return false;
			}

			if (target->id == UnitId::ProtossSolarion
			||  target->id == UnitId::ProtossAccantorOld)
			{
				if (target->get_current_shields_ingame() <= 10)
				{
					return true;
				}
			}

			return false;
		});
	}
}