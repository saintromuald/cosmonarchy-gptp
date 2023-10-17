#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* find_best_disruption_field_target(Unit* caster, bool isUnderAttack)
	{
		s32 bounds;
		if (isUnderAttack)
			bounds = 32 * 9;
		else if (is_campaign_ai(caster->player))
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
			if (!is_target_worth_hitting(target, caster))
			{
				return false;
			}
			Unit* targetOfTarget;
			if (unit_id_is_garrison(target->id) && target->has_loaded_unit())
			{
				targetOfTarget = target->get_first_loaded_unit()->order_target.unit;
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
			if (caster->is_target_enemy(targetOfTarget))
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
			||  target->id == UnitId::TerranAnchor
		//	||  target->id == UnitId::TerranTencendur
			||  target->id == UnitId::ZergSpraithCesiant
			||  target->id == UnitId::ZergSkortrithCesiant
			||  target->id == UnitId::ZergSpirtithCesiant
			||  target->id == UnitId::ProtossWarden
			||  target->id == UnitId::ProtossMatrix)
			{
				return true;
			}
			return false;
		});
	}
}