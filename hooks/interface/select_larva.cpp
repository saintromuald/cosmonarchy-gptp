#include "select_larva.h"
#include <SCBW/api.h>

//Helper functions declaration

namespace {

	uint32 getLarvaeUnitsFromList(CUnit** unitList, CUnit** outputList);									//0x00423190
	CUnit** getAllUnitsInBounds(Box16* coords);															//0x0042FF80
	uint32 CUnitToUnitID(CUnit* unit);																		//0x0047B1D0
	bool32 selectSingleUnitFromID(uint32 unitIndex);														//0x00496D30
	void selectMultipleUnitsFromUnitList(CUnit** unitList, uint32 unitsCount, bool32 unk1, bool32 unk2);	//0x0049AEF0

} //unnamed namespace

namespace hooks {

	///
	/// Filter larvae out of a list of units
	/// unitList contains various units (possibly including larvae)
	/// outputList is an empty list of 12 (SELECTION_ARRAY_LENGTH) 
	///slots to fill with larvae found in unitList.
	///
	uint32 getLarvaeUnitsFromList(CUnit** unitList, CUnit** outputList) {

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

	;

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

		larvaeCount = getLarvaeUnitsFromList(units_in_bounds, new_selection);

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

	;

} //namespace hooks

//-------- Helper function definitions. Do NOT modify! --------//

namespace {

	const uint32 Func_getLarvaeUnitsFromList = 0x00423190;
	uint32 getLarvaeUnitsFromList(CUnit** unitList, CUnit** outputList) {

		static uint32 larvaeList_Count;

		__asm {
			PUSHAD
			MOV ESI, outputList
			MOV EDX, unitList
			CALL Func_getLarvaeUnitsFromList
			MOV larvaeList_Count, EAX
			POPAD
		}

		return larvaeList_Count;

	}

	;

	const uint32 Func_GetAllUnitsInBounds = 0x0042FF80;
	CUnit** getAllUnitsInBounds(Box16* coords) {

		static CUnit** units_in_bounds;

		__asm {
			PUSHAD
			MOV EAX, coords
			CALL Func_GetAllUnitsInBounds
			MOV units_in_bounds, EAX
			POPAD
		}

		return units_in_bounds;

	}

	;

	const uint32 Func_CUnitToUnitID = 0x0047B1D0;
	uint32 CUnitToUnitID(CUnit* unit) {

		static uint32 return_value;

		__asm {
			PUSHAD
			MOV ESI, unit
			CALL Func_CUnitToUnitID
			MOV return_value, EAX
			POPAD
		}

		return return_value;

	}

	;

	const uint32 Func_SelectSingleUnitFromID = 0x00496D30;
	bool32 selectSingleUnitFromID(uint32 unitIndex) {

		bool32 bReturnedValue;

		__asm {
			PUSHAD
			PUSH unitIndex
			CALL Func_SelectSingleUnitFromID
			MOV bReturnedValue, EAX
			POPAD
		}

		return bReturnedValue;

	}

	;

	const uint32 Func_selectMultipleUnitsFromUnitList = 0x0049AEF0;
	void selectMultipleUnitsFromUnitList(CUnit** unitList, uint32 unitsCount, bool32 unk1, bool32 unk2) {

		__asm {
			PUSHAD
			PUSH unk2
			PUSH unk1
			PUSH unitList
			PUSH unitsCount
			CALL Func_selectMultipleUnitsFromUnitList
			POPAD
		}

	}

	;

} //unnamed namespace

//End of helper functions
