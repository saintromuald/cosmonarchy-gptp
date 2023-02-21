#include "cloak_tech.h"
#include <SCBW/scbwdata.h>

namespace hooks
{
	// Returns the tech ID used by this unit for the cloaking spell.
	// For the cloaking energy consumption, see energy_regeneration.cpp
	u8 get_cloaking_tech(Unit* unit) // 00491A50
	{
		//if (unit->id == UnitId::TerranEidolon)
		//{
		//	return TechId::PersonnelCloaking;
		//}
		//
		//if (unit->id == UnitId::TerranWraith)
		//{
		//	return TechId::CloakingField;
		//}
		return TechId::None;
	}

	b32 btnscond_is_cloaked(Unit* unit, u32 player, u16 requirement_var)
	{
		b32 result = true;
		Unit* unit_from_selection;
		for (s32 i = 0; i < SelectionCap && (result == true); i++)
		{
			unit_from_selection = g_client_selection_group->unit[i];
			if (unit_from_selection)
			{
				// Original code is hardcoded, not using a function
				u8 tech_id = get_cloaking_tech(unit_from_selection);
				if (unit_from_selection->can_use_tech(tech_id, player) == true)
				{
					if  (unit_from_selection->status & UnitStatus::CloakingForFree
					&& !(unit_from_selection->status & UnitStatus::Burrowed)) {
						result = false;
					}
					else if (!(unit_from_selection->status & UnitStatus::RequiresDetection))
					{
						result = false;
					}
				}
				else
				{
					result = false;
				}
			}
		}
		return result;
	}

	b32 btnscond_can_cloak(Unit* unit, u32 player, u16 requirement_var)
	{
		b32 result = false;
		Unit* unit_from_selection;
		for (s32 i = 0; i < SelectionCap && (result == false); i++)
		{
			unit_from_selection = g_client_selection_group->unit[i];
			if (unit_from_selection)
			{
				// Original code is hardcoded, not using a function
				u8 tech_id = get_cloaking_tech(unit_from_selection);

				if     (unit_from_selection->can_use_tech(tech_id, player) == true		// must be able to use cloaking tech for more checks to be necessary
				&&	 (!(unit_from_selection->status & UnitStatus::CloakingForFree)		// to cloak, unit should not already be cloaked for free
				||		unit_from_selection->status & UnitStatus::Burrowed)				// if burrowed, cloaking for free, but still allowed to cloak
				&&	  !(unit_from_selection->status & UnitStatus::RequiresDetection))	// if unit doesn't already require detection
				{
					result = true;
				}
			}
		}
		return result;
	}
}