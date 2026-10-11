#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{
	Unit* find_best_hallucination_target(Unit* caster, bool isUnderAttack)
	{
		if (isUnderAttack)
		{
			return nullptr;
		}
		s32 bounds = 32 * 32;
		return bw::UnitFinder::get_nearest_target(
			caster->position.x - bounds,
			caster->position.y - bounds,
			caster->position.x + bounds,
			caster->position.y + bounds,
			caster,
			[&](Unit* target)
		{
			if (target->player != caster->player)
			{
				return false;
			}
			if (target->status & UnitStatus::Invincible)
			{
				return false;
			}
			if (bw::get_tech_user_error_message((Unit*)target, caster->player, TechId::Hallucination) != 0)
			{
				return false;
			}
			if (target->id == UnitId::ProtossSolarion
				|| target->id == UnitId::ProtossPanoptus
				|| target->id == UnitId::ProtossDidact
				|| target->id == UnitId::ProtossArchon
				|| target->id == UnitId::ProtossAugur)
			{
				return true;
			}
			return false;
		});
	}
}
