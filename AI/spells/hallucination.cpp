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
		return scbw::UnitFinder::getNearestTarget(
			caster->getX() - bounds,
			caster->getY() - bounds,
			caster->getX() + bounds,
			caster->getY() + bounds,
			caster,
			[&](Unit* target)
		{
			if (target->playerId != caster->playerId)
			{
				return false;
			}
			if (target->status & UnitStatus::Invincible)
			{
				return false;
			}
			if (scbw::getTechUseErrorMessage((Unit*)target, caster->playerId, TechId::Hallucination) != 0)
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
