#include "select_larva.h"
#include <SCBW/api.h>

namespace hooks
{
	/// Filter larvae out of a list of units
	/// unitList contains various units (possibly including larvae) outputList is an empty list of 12 (SELECTION_ARRAY_LENGTH) slots to fill with larvae found in unitList.
	u32 get_larvae_units_from_list(Unit** unit_list, Unit** output_list) // 00423190
	{
		Unit* current_unit = *unit_list;
		u32 current_index = 0;
		if (current_unit)
		{
			bool bStopLoop = false;
			while (!bStopLoop)
			{
				if  (current_unit->id == UnitId::ZergLarva
				&&	 current_unit->connected_unit == *activePortraitUnit
				&&	!(current_unit->sprite->flags & SpriteFlags::Hidden))
				{
					output_list[current_index] = current_unit;
					current_index++;
					bStopLoop = (current_index >= SELECTION_ARRAY_LENGTH);
				}

				if (!bStopLoop)
				{
					unit_list++;
					current_unit = *unit_list;
					bStopLoop = (current_unit == nullptr);
				}
			}
		}
		return current_index;
	}

	/// Action of Select Larva button from main Zerg building
	/// Like almost all button actions, a command is queued and broadcast to all players (happen in cmdact_select that happen in selectMultipleUnitsFromUnitList).
	void BTSNACT_select_larva() // 00423930
	{
		Box16 coords;
		Unit** units_in_bounds;
		Unit* new_selection[SELECTION_ARRAY_LENGTH];
		u32 larvaeCount;

		coords.right = (*activePortraitUnit)->sprite->position.x + 128;
		coords.bottom = (*activePortraitUnit)->sprite->position.y + 128;
		coords.left = (*activePortraitUnit)->sprite->position.x - 128;
		coords.top = (*activePortraitUnit)->sprite->position.y - 128;

		units_in_bounds = helper::get_all_units_in_bounds(&coords);

		larvaeCount = get_larvae_units_from_list(units_in_bounds, new_selection);

		*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsListLastIndex[*tempUnitsListArraysCountsListLastIndex];
		*tempUnitsListArraysCountsListLastIndex = *tempUnitsListArraysCountsListLastIndex - 1;

		if (larvaeCount != 0)
		{
			bool bJumpToEnd = false;

			if (larvaeCount == 1 && *IS_HOLDING_ALT != 0)
			{
				u32 unitID = helper::unit_to_unit_id(new_selection[0]);
				bJumpToEnd = (helper::select_single_unit_from_id(unitID) != 0);
			}

			if (!bJumpToEnd)
			{
				helper::selectMultipleUnitsFromUnitList(new_selection, larvaeCount, 1, 1);
			}
		}
	}
}