#include "CMDRECV_SiegeTank.h"
#include <SCBW/api.h>

namespace hooks
{
	void CMDRECV_Siege(u8 bCommandType)
	{
		Unit* current_selection;

		*unit_iterator_pos = 0;
		current_selection = getActivePlayerNextSelection();

		while (current_selection != nullptr)
		{

			if (
				(current_selection->id == UnitId::TerranPhalanxTankMode ||
					current_selection->id == UnitId::Hero_EdmundDukeTankMode) &&
				current_selection->main_order_id != OrderId::SiegeMode
				)
			{
				if (current_selection->canUseTech(TechId::TankSiegeMode, *ACTIVE_NATION_ID) == 1)
				{

					if (bCommandType != 0)
						bCommandType = 1;
					else
						bCommandType = 0;

					helper::TargetedOrderCopy(
						current_selection,
						UnitId::None,
						*(u32*)(0x006D5C24),
						*(u32*)(0x006D5C20),
						OrderId::SiegeMode,
						0,
						bCommandType,
						1
					);

				}
			}

			current_selection = getActivePlayerNextSelection();

		}

	}

	;

	void CMDRECV_Unsiege(u8 bCommandType)
	{

		Unit* current_selection;

		*unit_iterator_pos = 0;
		current_selection = getActivePlayerNextSelection();

		while (current_selection != nullptr)
		{

			if (
				(current_selection->id == UnitId::TerranPhalanxSiegeMode ||
					current_selection->id == UnitId::Hero_EdmundDukeSiegeMode) &&
				current_selection->main_order_id != OrderId::TankMode
				)
			{
				if (current_selection->canUseTech(TechId::TankSiegeMode, *ACTIVE_NATION_ID) == 1)
				{

					if (bCommandType != 0)
						bCommandType = 1;
					else
						bCommandType = 0;

					helper::TargetedOrderCopy(
						current_selection,
						UnitId::None,
						*(u32*)(0x006D5C24),
						*(u32*)(0x006D5C20),
						OrderId::TankMode,
						0,
						bCommandType,
						1
					);

				}
			}

			current_selection = getActivePlayerNextSelection();

		}

	}
}