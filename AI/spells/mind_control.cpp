#include "spells.h"
#include <AI/ai_common.h>

namespace AI
{
	Unit* find_best_tyranny_target(Unit* caster, bool isUnderAttack)
	{
		s32 bounds = 32 * 32;
		return bw::UnitFinder::get_nearest_target(
			caster->position.x - bounds,
			caster->position.y - bounds,
			caster->position.x + bounds,
			caster->position.y + bounds,
			caster,
			[&](Unit* target)
		{
			if (!is_target_worth_hitting(target, caster))
			{
				return false;
			}

			if (units_dat::base_property[target->id] & UnitProperty::Hero)
			{
				return false;
			}

			if (unit_id_is_transport(target->id))
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
				case UnitId::TerranMadrigalImperiosoMode:
				case UnitId::TerranSeraph:
				case UnitId::TerranMinotaur:
				case UnitId::TerranCleric:
				case UnitId::TerranShamanSupportMode:
				case UnitId::TerranShamanVisionaryMode:
				case UnitId::ZergOvileth:
				case UnitId::ZergUltrokor:
				case UnitId::ZergMatraleth:
				case UnitId::ZergIsthrathaleth:
				case UnitId::ZergVithrilisk:
				case UnitId::ProtossMindTyrant:
				case UnitId::ProtossCantavis:
				case UnitId::ProtossDidact:
				case UnitId::ProtossSolarion:
				case UnitId::ProtossAccantorOld:
				case UnitId::ZergLakizilisk:
				{
					return true;
				}
				default:
				{
					if (units_dat::mineral_cost[target->id] >= units_dat::mineral_cost[UnitId::ProtossCabalist]
					&&  units_dat::vespene_cost[target->id] >= units_dat::vespene_cost[UnitId::ProtossCabalist])
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
