#include "selection.h"
#include <SCBW/api.h>
#include "hooks/rally_point.h"

namespace hooks
{
	/// Function used by Ctrl+Click (select all units of same type)
	u32 sort_all_units(Unit* unit, Unit** unit_list, Unit** units_in_bounds)
	{
		const u32* VISIBILITY_CHECK_0057F0B0 = (u32*)0x0057F0B0;
		Unit* current_unit;
		Unit* backup_current_unit;
		u32 current_index_in_unit_list;

		bool bStopBigLoop = false;
		bool bKeepForEmptyListCase = false;
		bool dontAddToList = false;
		backup_current_unit = 0;

		// if unit not null, become first entry of unit_list
		if (unit) {
			unit_list[0] = unit;
			current_index_in_unit_list = 1;
		} else {
			current_index_in_unit_list = 0;
		}

		bool has_units = false;
		bool has_buildings = false;
		for (Unit** it = units_in_bounds; *it; ++it) {
			Unit* current_unit = *it;
			if (current_unit != unit && (!unit || current_unit->id == unit->id)) {
				if (units_dat::BaseProperty[current_unit->id] & UnitProperty::Subunit) {
					current_unit = current_unit->subunit;
				}
				if (units_dat::BaseProperty[current_unit->id] & UnitProperty::Building) {
					has_buildings = true;
				} else {
					has_units = true;
				}
			}
		}

		// work with the first unit from units_in_bounds
		current_unit = *units_in_bounds;
		if (!current_unit)
		{
			bStopBigLoop = true;
		}
		while (!bStopBigLoop)
		{
			if (current_unit != unit)
			{
				if (has_units) {
					if (units_dat::BaseProperty[current_unit->id] & UnitProperty::Building) {
						dontAddToList = true;
					}
				} else if (has_buildings) {
					if (!(units_dat::BaseProperty[current_unit->id] & UnitProperty::Building)) {
						dontAddToList = true;
					}
				}

				if (!dontAddToList) {
					// if current_unit==unit, then it's already in
					// unit_list
					if (current_unit->subunit)
					{
						// if what got it is the turret or another subunit, swap
						// the unit and the subunit as current_unit
						if (units_dat::BaseProperty[current_unit->id] & UnitProperty::Subunit)
						{
							current_unit = current_unit->subunit;
						}
						if (current_index_in_unit_list != 0)
						{
							// 0046F150
							u32 index = current_index_in_unit_list;
							do
							{
								index--;
								// if the unit is already in the list, skip it
								if (unit_list[index] == current_unit)
								{
									dontAddToList = true;
								}
							}
							while (index != 0 && !dontAddToList);
						}
					}
				}

				if (!dontAddToList)
				{
					// 0046F161
					// if the unit sprite is visible, perform additionnal visibility test, else skip the unit
					if (current_unit->sprite->visibilityFlags & (u8)*VISIBILITY_CHECK_0057F0B0)
					{
						// if the unit is cloaked and visibility status test (detection by detectors units probably) fail, the unit is skipped
						if (current_unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection))
						{
							if (!(current_unit->visibility_status & *VISIBILITY_CHECK_0057F0B0))
							{
								dontAddToList = true;
							}
						}
					}
					else
					{
						dontAddToList = true;
					}

					if (!dontAddToList)
					{
						// 0046F18C
						if (helper::unit_is_unselectable(current_unit->id))
						{
							dontAddToList = true;
						}
						else if (!helper::unit_is_standard_and_movable(current_unit))
						{
							bKeepForEmptyListCase = true; // skip the unit for the list, unless it's the only available
						}
						else if (*bw_is_in_replay)
						{
							bKeepForEmptyListCase = true; // skip the unit for the list, unless it's the only available
						}
						else if (current_unit->player != *LOCAL_NATION_ID)
						{
							bKeepForEmptyListCase = true; // skip the unit for the list, unless it's the only available
						}
						else
						{
							// 0046F1C5
							if (unit)
							{
								// all units should belong to the same player (the one owning the clicked unit)
								if (unit->player != current_unit->player)
								{
									dontAddToList = true;
								}
								// select only units with same Id
								else if (unit->id != current_unit->id)
								{
									dontAddToList = true;
								}
								else
								{
									// 0046F1D9
									// equivalent to "unit->status XOR current_unit->status"
									// keep the flags appearing in only 1 of the variables
									u32 mixed_flags = (~(unit->status) & current_unit->status) | (unit->status & ~(current_unit->status));

									// Only select all burrowed OR all unburrowed
									// units.The test exclude current_unit if the
									// burrowed state differ from the one of unit.
									if (mixed_flags & UnitStatus::Burrowed)
									{
										dontAddToList = true;
									}

									// Same thing for hallucinations
									if (mixed_flags & UnitStatus::IsHallucination)
									{
										dontAddToList = true;
									}

									// Same thing for completed
									if (mixed_flags & UnitStatus::Completed)
									{
										dontAddToList = true;
									}

									// Same thing for lifted
									if (mixed_flags & UnitStatus::InAir)
									{
										dontAddToList = true;
									}
								}
							}

							if (!dontAddToList)
							{
								// 0046F203
								// 0x0046F208 (use of SelectionCap)
								if (current_index_in_unit_list >= SelectionCap) // action when unit_list is full
								{
									helper::function_0046F040(current_unit, unit_list, unit, current_index_in_unit_list);
								}
								else
								{
									if (units_dat::BaseProperty[current_unit->id] & UnitProperty::Building) {
										has_buildings = true;
									} else {
										has_units = true;
									}

									// 0046F217
									// Add current_unit to unit_list
									unit_list[current_index_in_unit_list] = current_unit;
									current_index_in_unit_list++;
								}
							}
						}
					}
				}
			}

			if (bKeepForEmptyListCase)
			{
				backup_current_unit = current_unit;
			}

			// 0046F226
			// select next unit in bounds
			units_in_bounds += 1; // this actually advance by 4 bytes
			current_unit = *units_in_bounds;
			bKeepForEmptyListCase = false;
			dontAddToList = false;

			// continue until current_unit is nullptr
			if (!current_unit)
			{
				bStopBigLoop = true;
			}
		}

		// 0046F242
		if (current_index_in_unit_list == 0)
		{
			// if current_index_in_unit_list == 0, the list is empty and cannot be used, so another solution is found here
			if (!backup_current_unit)
			{
				// 0046F24E
				if (!unit)
				{
					current_index_in_unit_list = 0; // return 0 (no list)
				}
				else
				{
					// 0046F252
					unit_list[0] = unit;
					current_index_in_unit_list = 1; // return 1 (the clicked unit alone)
				}
			}
			else
			{
				// 0046F264
				if (unit)
				{
					unit_list[0] = unit;
					current_index_in_unit_list = 1; // return 1 (the clicked unit alone)
				}
				else
				{
					unit_list[0] = backup_current_unit;
					current_index_in_unit_list = 1; // return 1 (an unit preserved by bKeepForEmptyListCase)
				}
			}
		}	

		return current_index_in_unit_list;
	}

	/// Function used by Click on unit (possibly with key pressed)
	void unit_selection_click(Unit* clicked_unit)
	{
		b32* const IS_DOUBLE_CLICKING = (b32*)0x0066FF58;

		b8* const bCanUpdateSelectedUnitData = (b8*)0x0059723C;
		b8* const bCanUpdateSelectedUnitPortrait = (b8*)0x0068AC74;
		b32* const bCanUpdateCurrentButtonSet = (b32*)0x0068C1B0;
		BinDlg** const someDialogUnknown = (BinDlg**)0x0068C1E8; // related to MouseOver?
		b8* const bCanUpdateStatDataDialog = (b8*)0x0068C1F8;
		BinDlg** const someDialogUnknownUser = (BinDlg**)0x0068C1EC; // related to MouseOver? Usually someDialogUnknown->user if not 0 or -1

		Unit* local_temp_array_1[SelectionCap];	// used instead of an array starting from [EBP-3C]
		Unit* local_temp_array_2[SelectionCap];	// used instead of an array starting from [EBP-6C]

		Box16 local_temp_box16_structure;	// used instead of using from [EBP-0C] to [EBP-06]

		bool isHoldingCtrl_OR_isDoubleClickingSelectedClickedUnit =
			*IS_HOLDING_CTRL || (*IS_DOUBLE_CLICKING && (clicked_unit->sprite->flags & SpriteFlags::Selected));

		for (s32 i = 0; i < SelectionCap; i++)
		{
			local_temp_array_1[i] = 0;
		}

		if (isHoldingCtrl_OR_isDoubleClickingSelectedClickedUnit
			|| (*IS_HOLDING_SHIFT && active_player_selection->unit[0]))
		{
			if (!*IS_HOLDING_SHIFT)
			{
				// 0046FE07
				if (isHoldingCtrl_OR_isDoubleClickingSelectedClickedUnit)
				{
					// Ctrl+click on unit or f64-click on already
					// it unit without other key
					// =>	select surrounding units with same type as clicked unit, making them the current unit_list

					Unit** units_in_bounds;
					u32 sorted_list_length;

					// locate surrounding units for unit_list
					local_temp_box16_structure.left = *move_to_x;
					local_temp_box16_structure.top = *move_to_y;
					local_temp_box16_structure.right = *move_to_x + 640;
					local_temp_box16_structure.bottom = *move_to_y + 400;
					units_in_bounds = helper::get_all_units_in_bounds(&local_temp_box16_structure);
					sorted_list_length = sort_all_units(clicked_unit, local_temp_array_1, units_in_bounds);

					// reload the previous temporary unit list from before the call to get_all_units_in_bounds
					*tempUnitsListArraysCountsListLastIndex = *tempUnitsListArraysCountsListLastIndex - 1;
					*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsList[*tempUnitsListArraysCountsListLastIndex];

					if (sorted_list_length != 0)
					{
						// turn the local array as the newly applied unit_list
						helper::select_units_if_alt_not_held(local_temp_array_1, sorted_list_length, 1, 1);

						// This is what make the rally point briefly appear when selecting a building with one. Maybe can do other things?
						if (sorted_list_length == 1
							&& clicked_unit->player == *ACTIVE_NATION_ID
							&& clicked_unit->is_factory())
						{
							helper::show_rally_target(clicked_unit);
						}
					}
				}
			}
			else
			{
				// holding SHIFT (if not holding CTRL, then there's an active unit_list too)
				if (isHoldingCtrl_OR_isDoubleClickingSelectedClickedUnit)
				{
					// {Ctrl+Shift+click on unit} OR {Shift+f64-click on 
					// it unit WHILE having an active unit_list}
					// select units same type as clicked unit, adding them to the
					// active unit_list

					Unit** units_in_bounds;
					u32 sorted_list_length;

					// prepare to locate surrounding units for unit_list
					local_temp_box16_structure.left = *move_to_x;
					local_temp_box16_structure.top = *move_to_y;
					local_temp_box16_structure.right = *move_to_x + 640;
					local_temp_box16_structure.bottom = *move_to_y + 400;

					// save existing unit_list to a temporary array
					for (s32 i = 0; i < SelectionCap; i++)
					{
						local_temp_array_2[i] = active_player_selection->unit[i];
					}

					// locate surrounding units for unit_list
					units_in_bounds = helper::get_all_units_in_bounds(&local_temp_box16_structure);
					sorted_list_length = sort_all_units(clicked_unit, local_temp_array_1, units_in_bounds);

					// reload the previous temporary unit list from before the call to get_all_units_in_bounds
					*tempUnitsListArraysCountsListLastIndex = *tempUnitsListArraysCountsListLastIndex - 1;
					*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsList[*tempUnitsListArraysCountsListLastIndex];

					if (sorted_list_length != 0)
					{
						// 0046FCD4
						// recombine current unit_list and newly selectable items in local_temp_array_2
						u32 list_remixed_length = helper::combine_lists(clicked_unit, local_temp_array_1, local_temp_array_2, sorted_list_length);
						// turn local_temp_array_2 as the newly applied unit_list
						helper::select_units_if_alt_not_held(local_temp_array_2, list_remixed_length, 1, 1);
					}
				}
				else
				{
					// 0046FCF6

					// Shift + click while there's an existing unit_list (because hold on CTRL case was already dealt with so there's an existing unit_list at this point) add/remove unit from unit_list
					s32 arrayIndex;
					// copy the existing unit_list into the local array
					for (arrayIndex = 0; arrayIndex < SelectionCap && active_player_selection->unit[arrayIndex]; arrayIndex++)
					{
						local_temp_array_1[arrayIndex] = active_player_selection->unit[arrayIndex];
					}

					if (!(clicked_unit->sprite->flags & SpriteFlags::Selected))
					{
						// 0x0046FD13 (getting unit sprite) and 0x0046FD18 (middle of the test with the flag)
						// unit not it, so it's added to current unit_list if valid
						if (arrayIndex < SelectionCap
							&& helper::unit_is_standard_and_movable(local_temp_array_1[0])
							&& helper::unit_is_owned_by_current_player(local_temp_array_1[0])
							&& helper::unit_is_standard_and_movable(clicked_unit)
							&& helper::unit_is_owned_by_current_player(clicked_unit))
						{
							// new unit is added at end of array, and the index is increased to be used as new array length
							local_temp_array_1[arrayIndex] = clicked_unit;
							arrayIndex++;

							// turn local_temp_array_1 as the newly applied unit_list
							helper::select_units_if_alt_not_held(local_temp_array_1, arrayIndex, 1, 1);
						}
					}
					else
					{
						// 0046FD77
						// unit already it, remove it from unit_list
						u32 memcpy_size;
						bool bUpdateSelection = true;

						// decrease it so it's equal to the index of last element
						arrayIndex--;
						// each element is 4 bytes, multiply it by the number of elements after the one to remove
						memcpy_size = (arrayIndex - clicked_unit->sprite->selectionIndex) * 4;

						// Copy the elements on the right of the one to delete one step left to overwrite the one to delete
						helper::sc_memcpy_0((u32)&local_temp_array_1[clicked_unit->sprite->selectionIndex], (u32)&local_temp_array_1[clicked_unit->sprite->selectionIndex + 1], memcpy_size);

						// either erase the element to delete or one that was repeated after memcpy
						local_temp_array_1[arrayIndex] = 0;

						if (arrayIndex == 1)
						{
							if (*IS_HOLDING_ALT)
							{
								// It seems you can unselect things with Shift+Alt like
								// with shift alone, UNTIL you have 2 units it.
								// It is the code below that is causing the
								// deselection to fail where simple shift+click would
								// continue to deselect.
								u32 unitIndex = helper::unit_to_unit_id(local_temp_array_1[0]);
								if (helper::select_single_unit_from_id(unitIndex) != 0)
								{
									bUpdateSelection = false;
								}
							}
						}

						if (bUpdateSelection)
						{
							// Create a new unit unit_list (since the clicked unit is unselected)
							// and choose it as current unit_list)
							helper::create_new_unit_selections_from_list(local_temp_array_1, arrayIndex);
							helper::cmdact_select(local_temp_array_1, arrayIndex);

							// tell the GUI to update stuff
							*someDialogUnknown = 0;
							*someDialogUnknownUser = 0;
							*bCanUpdateSelectedUnitData = 1;
							*bCanUpdateCurrentButtonSet = 1;
							*bCanUpdateSelectedUnitPortrait = 1;
							*bCanUpdateStatDataDialog = 1;

						}
					}
				}
			}
		}

		else
		{
			// 0046FBB7
			// Simple click...almost (there's still ALT key)
			bool bRecreatedSelectionThroughAltClick = false;
			if (*IS_HOLDING_ALT)
			{
				// Recall the unit_list the unit belong to if there's one
				// In this case, skip the code for a single unit handling

				u32 temp_unit_index = helper::unit_to_unit_id(clicked_unit);
				if (helper::select_single_unit_from_id(temp_unit_index) != 0)
				{
					bRecreatedSelectionThroughAltClick = true;
				}
			}

			if (!bRecreatedSelectionThroughAltClick)
			{
				// 0046FBD1
				// Create a new unit_list containing only the clicked unit
				helper::create_new_unit_selections_from_list(&clicked_unit, 1);
				// play the sound of the unit (don't need to be a building)
				helper::select_building_sfx(clicked_unit);
				// select the unit
				helper::cmdact_select(&clicked_unit, 1);

				// tell the GUI to update stuff
				*bCanUpdateSelectedUnitData = 1;
				*bCanUpdateCurrentButtonSet = 1;
				*bCanUpdateSelectedUnitPortrait = 1;
				*bCanUpdateStatDataDialog = 1;
				*someDialogUnknown = 0;
				*someDialogUnknownUser = 0;
			}

			// 0046FC2C
			// this is what make the rally point briefly appear when
			// selecting a building with one.
			// maybe can do others things?
			if (clicked_unit->player == *ACTIVE_NATION_ID
				&& clicked_unit->is_factory())
			{
				helper::show_rally_target(clicked_unit);
			}
		}
	}

	static u32 filter_out_buildings(Unit** list, u32 unit_count, Unit** sample_list, u32 sample_count) {
		bool has_buildings = false;
		bool has_units = false;
		for (u32 i = 0; i < sample_count; ++i) {
			Unit* unit = sample_list[i];
			if (!unit) {
				break;
			}
			if (units_dat::BaseProperty[unit->id] & UnitProperty::Building) {
				has_buildings = true;
			} else {
				has_units = true;
			}
		}

		u32 count = 0;
		for (u32 i = 0; i < unit_count; ++i) {
			Unit* unit = list[i];
			if (!unit) {
				break;
			}
			if ((has_units && has_buildings) || !has_units) {
				if (units_dat::BaseProperty[unit->id] & UnitProperty::Building) {
					list[count++] = unit;
				}
			} else if (!has_buildings) {
				if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Building)) {
					list[count++] = unit;
				}
			}
		}
		return count;
	}
	
	// Hooked to forbid buildings. -Pr0nogo
	s32 select_multiple_units_from_unit_list(u32 count, Unit** unit_list, char play_sound, u32 send_command) { // 0049AEF0
		if (*IS_HOLDING_SHIFT) {
			count = filter_out_buildings(unit_list, count, active_player_selection->unit, SelectionCap);
		}
		helper::create_new_unit_selections_from_list(unit_list, count);
		if (play_sound) {
			Unit* sfx_unit = 0;
			if (count) {
				for (u32 i = 0; i < count; ++i) {
					Unit* unit = unit_list[i];
					if (helper::compare_unit_rank(unit, sfx_unit)) {
						sfx_unit = unit;
					}
				}
			}
			helper::select_building_sfx(sfx_unit);
		}
		if (send_command) {
			helper::cmdact_select(unit_list, count);
		}
		scbw::refresh_console();
		return 1;
	}

	// Hooked to forbid buildings. -Pr0nogo
	u32 create_new_unit_selections_from_list(Unit** unit_list, u32 count) { // 0049AE40
		count = filter_out_buildings(unit_list, count, unit_list, count);
		for (u32 i = 0; i < SelectionCap; ++i) {
			Unit* unit = active_player_selection->unit[i];
			if (!unit) {
				break;
			}
			active_player_selection->unit[i] = 0;
			if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit) {
				unit = unit->subunit;
			}
			helper::destroy_unit_selection(unit);
		}
		for (u32 i = 0; i < count; ++i) {
			Unit* unit = unit_list[i];
			active_player_selection->unit[i] = unit;
			if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit) {
				helper::create_unit_selection(unit->subunit, i);
				active_player_selection->unit[i] = unit->subunit;
				unit_list[i] = unit->subunit;
			} else {
				helper::create_unit_selection(unit, i);
			}
		}
		return count;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	void __stdcall get_selected_units_in_box(Box16* coords) { // 0046FA40
		Unit* new_units[SelectionCap] = {};
		Unit** found_units = helper::find_all_units(coords);
		u32 sorted_units = helper::sort_all_units(found_units, new_units, 0);
		*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsListLastIndex[tempUnitsListArraysCountsListLastIndex[0]--];
		if (sorted_units) {
			for (u32 i = 0; i < SelectionCap; ++i) {
				Unit* unit = new_units[i];
				if (unit && unit->player == *ACTIVE_NATION_ID) {
					if (hooks::unit_can_rally(unit)) {
						helper::show_rally_target(unit);
						break; // Currently the game doesn't support showing more than 1 rally thingy XD
					}
				}
			}

			if (IS_KEYCODE_USED[16] && active_player_selection[0].unit) {
				Unit* combined_units[SelectionCap] = {};
				memcpy(combined_units, active_player_selection, sizeof(UnitsSel));
				u32 added_units = helper::combine_lists(0, new_units, combined_units, sorted_units);
				helper::select_units_if_alt_not_held(combined_units, added_units, 1, 1);
				return;
			}

			if (sorted_units != 1
			|| !IS_KEYCODE_USED[18]
			|| !helper::select_single_unit_from_id(helper::unit_to_unit_id(new_units[0]))) {
				helper::select_multiple_units_from_unit_list((u32)sorted_units, new_units, 1, 1);
				if (sorted_units != 1) {
					return;
				}
			}
		}
	}
}