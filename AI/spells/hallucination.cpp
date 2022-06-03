#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{
	Unit* findBestHallucinationTarget(Unit* caster, bool isUnderAttack)
	{
		if (isUnderAttack)
		{
			return nullptr;
		}
		s32 bounds = 32 * 32;
		return scbw::UnitFinder::get_nearest_target(
			caster->get_x() - bounds,
			caster->get_y() - bounds,
			caster->get_x() + bounds,
			caster->get_y() + bounds,
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
			if (scbw::get_tech_user_error_message((Unit*)target, caster->player, TechId::Hallucination) != 0)
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
