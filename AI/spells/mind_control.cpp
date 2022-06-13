#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* findBestMindControlTarget(Unit* caster, bool isUnderAttack)
	{
		s32 bounds = 32 * 32;
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

			if (units_dat::BaseProperty[target->id] & UnitProperty::Hero)
			{
				return false;
			}

			if (target->id == UnitId::ProtossEnvoy || target->id == UnitId::TerranTrojan)
			{
				if (target->has_loaded_unit())
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
					if (units_dat::mineral_cost[target->id] >= units_dat::mineral_cost[UnitId::ProtossCabalist]
						&& units_dat::gas_cost[target->id] >= units_dat::gas_cost[UnitId::ProtossCabalist])
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
