#include "select_larva.h"
#include <SCBW/api.h>

namespace hooks
{
	/// Filter larvae out of a list of units
	/// unitList contains various units (possibly including larvae) outputList is an empty list of 12 (SelectionCap) slots to fill with larvae found_larvae in unitList.
	u32 get_larvae_units_from_list(Unit** unit_list, Unit** output_list) { // 00423190
		u32 count = 0;
		for (Unit** it = unit_list; *it; ++it) {
			Unit* unit = *it;
			if (unit->id == UnitId::ZergLarva && !(unit->sprite->flags & SpriteFlags::Hidden)) {
				bool connected = false;
				for (u32 i = 0; i < SelectionCap; ++i) {
					Unit* other = client_selection_group->unit[i];
					if (!other) {
						break;
					}
					if (unit->connected_unit == other) {
						connected = true;
						break;
					}
				}

				if (connected) {
					output_list[count++] = unit;
					if (count >= SelectionCap) {
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
		Unit* selection[SelectionCap] = {};

		for (u32 i = 0; i < SelectionCap; ++i) {
			Unit* unit = client_selection_group->unit[i];
			if (!unit) {
				break;
			}
			Box16 coords = {};
			coords.right = unit->sprite->position.x + 128;
			coords.bottom = unit->sprite->position.y + 128;
			coords.left = unit->sprite->position.x - 128;
			coords.top = unit->sprite->position.y - 128;

			Unit** units_in_bounds = helper::get_all_units_in_bounds(&coords);
			Unit* found_larvae[SelectionCap] = {};

			u32 found_count = get_larvae_units_from_list(units_in_bounds, found_larvae);

			*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsList[--*tempUnitsListArraysCountsListLastIndex];

			for (u32 i = 0; i < found_count; ++i) {
				for (u32 j = 0; j < count; ++j) {
					if (found_larvae[i] == selection[j]) {
						goto skip;
					}
				}
				if (count < SelectionCap) {
					selection[count++] = found_larvae[i];
				}
			skip:
				if (count >= SelectionCap) {
					goto exit;
				}
			}
		}

	exit:
		if (count) {
			bool select = true;
			if (count == 1 && *IS_HOLDING_ALT) {
				u32 unit_id = helper::unit_to_unit_id(selection[0]);
				if (helper::select_single_unit_from_id(unit_id)) {
					select = false;
				}
			}
			if (select) {
				helper::select_multiple_units_from_unit_list(count, selection, 1, 1);
			}
		}
	}
}