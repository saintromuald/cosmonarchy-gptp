#include "select_larva.h"
#include <SCBW/api.h>

namespace hooks {
	///
	/// Filter larvae out of a list of units
	/// unitList contains various units (possibly including larvae)
	/// outputList is an empty list of 12 (SELECTION_ARRAY_LENGTH) 
	///slots to fill with larvae found in unitList.
	///
	uint32 getLarvaeUnitsFromListHook(CUnit** unitList, CUnit** outputList) {
		CUnit* current_unit = *unitList;
		uint32 current_index = 0;
		if (current_unit != nullptr) {
			bool bStopLoop = false;
			while (!bStopLoop) {
				if (
					current_unit->id == UnitId::ZergLarva &&
					current_unit->connectedUnit == *activePortraitUnit &&
					!(current_unit->sprite->flags & CSprite_Flags::Hidden)
					) {
					outputList[current_index] = current_unit;
					current_index++;
					bStopLoop = (current_index >= SELECTION_ARRAY_LENGTH);
				}

				if (!bStopLoop) {
					unitList++;
					current_unit = *unitList;
					bStopLoop = (current_unit == nullptr);
				}
			}
		}
		return current_index;
	}

	///
	/// Action of Select Larva button from main Zerg building
	/// Like almost all button actions, a command is queued and
	/// broadcast to all players (happen in CMDACT_Select that
	/// happen in selectMultipleUnitsFromUnitList).
	///
	void BTNSACT_SelectLarva() {

		Box16 coords;
		CUnit** units_in_bounds;
		CUnit* new_selection[SELECTION_ARRAY_LENGTH];
		uint32 larvaeCount;

		coords.right = (*activePortraitUnit)->sprite->position.x + 128;
		coords.bottom = (*activePortraitUnit)->sprite->position.y + 128;
		coords.left = (*activePortraitUnit)->sprite->position.x - 128;
		coords.top = (*activePortraitUnit)->sprite->position.y - 128;

		units_in_bounds = getAllUnitsInBounds(&coords);

		larvaeCount = getLarvaeUnitsFromListHook(units_in_bounds, new_selection);

		*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsListLastIndex[*tempUnitsListArraysCountsListLastIndex];
		*tempUnitsListArraysCountsListLastIndex = *tempUnitsListArraysCountsListLastIndex - 1;

		if (larvaeCount != 0) {

			bool bJumpToEnd = false;

			if (larvaeCount == 1 && *IS_HOLDING_ALT != 0) {
				uint32 unitID = CUnitToUnitID(new_selection[0]);
				bJumpToEnd = (selectSingleUnitFromID(unitID) != 0);
			}

			if (!bJumpToEnd)
				selectMultipleUnitsFromUnitList(new_selection, larvaeCount, 1, 1);
		}
	}
}