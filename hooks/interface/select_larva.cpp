#include "select_larva.h"
#include <SCBW/api.h>

namespace hooks
{
	/// Filter larvae out of a list of units
	/// unitList contains various units (possibly including larvae) outputList is an empty list of 12 (SELECTION_ARRAY_LENGTH) slots to fill with larvae found_larvae in unitList.
	u32 get_larvae_units_from_list(Unit** unit_list, Unit** output_list) { // 00423190
		u32 count = 0;
		for (Unit** it = unit_list; *it; ++it) {
			Unit* unit = *it;
			if (unit->id == UnitId::ZergLarva && !(unit->sprite->flags & SpriteFlags::Hidden)) {
				bool connected = false;
				*unit_iterator_pos = 0;
				for (Unit* other = helper::get_active_player_next_selection(); other; other = helper::get_active_player_next_selection()) {
					if (unit->connected_unit == other) {
						connected = true;
						break;
					}
				}

				if (connected) {
					output_list[count++] = unit;
					if (count >= SELECTION_ARRAY_LENGTH) {
						break;
					}
				}
			}
		}
		return count;
	}

	/// Action of Select Larva button from main Zerg building
	/// Like almost all button actions, a command is queued and broadcast to all players (happen in cmdact_select that happen in select_multiple_units_from_unit_list).
	void BTSNACT_select_larva() { // 00423930
		u32 count = 0;
		Unit* selection[SELECTION_ARRAY_LENGTH] = {};

		*unit_iterator_pos = 0;
		while (Unit* unit = helper::get_active_player_next_selection()) {
			Box16 coords = {};
			coords.right = unit->sprite->position.x + 128;
			coords.bottom = unit->sprite->position.y + 128;
			coords.left = unit->sprite->position.x - 128;
			coords.top = unit->sprite->position.y - 128;

			Unit** units_in_bounds = helper::get_all_units_in_bounds(&coords);
			Unit* found_larvae[SELECTION_ARRAY_LENGTH] = {};

			u32 temp = *unit_iterator_pos;
			u32 found_count = get_larvae_units_from_list(units_in_bounds, found_larvae);
			*unit_iterator_pos = temp;

			*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsList[--*tempUnitsListArraysCountsListLastIndex];

			bool exit = false;
			for (u32 i = 0; i < found_count; ++i) {
				bool add = true;
				for (u32 j = 0; j < count; ++j) {
					if (found_larvae[i] == selection[j]) {
						add = false;
						break;
					}
				}
				if (add && count < SELECTION_ARRAY_LENGTH) {
					selection[count++] = found_larvae[i];
				}
				if (count >= SELECTION_ARRAY_LENGTH) {
					exit = true;
					break;
				}
			}
			if (exit) {
				break;
			}
		}

		if (count) {
			bool select = true;
			if (count == 1 && *IS_HOLDING_ALT) {
				u32 unit_id = helper::unit_to_unit_id(selection[0]);
				if (helper::select_single_unit_from_id(unit_id)) {
					select = false;
				}
			}
			if (select) {
				helper::select_multiple_units_from_unit_list(selection, count, 1, 1);
			}
		}
	}
}