#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* findBestMindControlTarget(Unit* caster, bool isUnderAttack)
	{
		s32 bounds = 32 * 32;
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

			if (units_dat::BaseProperty[target->id] & UnitProperty::Hero)
			{
				return false;
			}

			if (target->id == UnitId::ProtossEnvoy || target->id == UnitId::TerranTrojan)
			{
				if (target->hasLoadedUnit())
				{
					return true;
				}
			}

			switch (target->id)
			{
				case UnitId::TerranPhalanxTankMode:
				case UnitId::TerranPhalanxSiegeMode:
				case UnitId::TerranMadrigal:
				case UnitId::TerranSeraph:
				case UnitId::TerranMinotaur:
				case UnitId::TerranCleric:
				case UnitId::TerranShaman:
				case UnitId::ZergOvileth:
				case UnitId::ZergUltrokor:
				case UnitId::ZergMatraleth:
				case UnitId::ZergIsthrathaleth:
				case UnitId::ZergVithrilisk:
				case UnitId::ProtossMindTyrant:
				case UnitId::ProtossCantavis:
				case UnitId::ProtossDidact:
				case UnitId::ProtossSolarion:
				case UnitId::ProtossAccantor:
				case UnitId::ZergLakizilisk:
				{
					return true;
				}
				default:
				{
					if (units_dat::MineralCost[target->id] >= units_dat::MineralCost[UnitId::ProtossCabalist]
						&& units_dat::GasCost[target->id] >= units_dat::GasCost[UnitId::ProtossCabalist])
					{
						return true;
					}
					break;
				}
			}
			return false;
		});
	}
}
