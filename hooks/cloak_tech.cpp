#include "cloak_tech.h"
#include <SCBW/scbwdata.h>

namespace hooks
{

	// Returns the tech ID used by this unit for the cloaking spell.
	// For the cloaking energy consumption, see energy_regeneration.cpp
	u8 getCloakingTech(Unit* unit)
	{
		if (unit->id == UnitId::TerranEidolon)
		{
			return TechId::PersonnelCloaking;
		}

		if (unit->id == UnitId::TerranWraith)
		{
			return TechId::CloakingField;
		}

		return TechId::None;

	}

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

	b32 BTNSCOND_IsCloaked(Unit* unit, u32 playerId, u16 requirement_var)
	{

		b32 bReturnValue = TRUE;
		Unit* unit_from_selection;


		for (s32 i = 0; i < SELECTION_ARRAY_LENGTH && (bReturnValue == TRUE); i++)
		{

			unit_from_selection = clientSelectionGroup->unit[i];

			if (unit_from_selection != nullptr)
			{

				//original code is hardcoded, not using a function
				u8 techId = getCloakingTech(unit_from_selection);

				if (unit_from_selection->canUseTech(techId, playerId) == TRUE)
				{

					if (
						unit_from_selection->status & UnitStatus::CloakingForFree &&
						!(unit_from_selection->status & UnitStatus::Burrowed)
						)
						bReturnValue = FALSE;
					else
						if (!(unit_from_selection->status & UnitStatus::RequiresDetection))
							bReturnValue = FALSE;

				}
				else
					bReturnValue = FALSE;

			}

		}

		return bReturnValue;

	} //b32 BTNSCOND_IsCloaked(Unit* unit, u32 player, u16 requirement_var)

	b32 BTNSCOND_CanCloak(Unit* unit, u32 playerId, u16 requirement_var)
	{

		b32 bReturnValue = FALSE;
		Unit* unit_from_selection;

		for (s32 i = 0; i < SELECTION_ARRAY_LENGTH && (bReturnValue == FALSE); i++)
		{

			unit_from_selection = clientSelectionGroup->unit[i];

			if (unit_from_selection != nullptr)
			{

				//original code is hardcoded, not using a function
				u8 techId = getCloakingTech(unit_from_selection);

				if (
					unit_from_selection->canUseTech(techId, playerId) == TRUE &&				//must be able to use cloaking tech for more checks to be necessary
					(!(unit_from_selection->status & UnitStatus::CloakingForFree) ||	//to cloak, unit should not already be cloaked for free
						unit_from_selection->status & UnitStatus::Burrowed) &&				//if burrowed, cloaking for free, but still allowed to cloak
					!(unit_from_selection->status & UnitStatus::RequiresDetection)			//if unit doesn't already require detection
					)
					bReturnValue = TRUE;

			}

		}

		return bReturnValue;

	} //b32 BTNSCOND_CanCloak(Unit* unit, u32 player, u16 requirement_var)

} //hooks
