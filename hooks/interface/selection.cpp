#include "selection.h"
#include <SCBW/api.h>
#include "hooks/rally_point.h"
#include "trigger_system/trigger_system.h"

namespace hooks
{
	static s32 compare_selection_unit(const void* a, const void* b) {
		Unit* ua = *(Unit**)a;
		Unit* ub = *(Unit**)b;

		Point16 mouse_pos = {};
		mouse_pos.x = *g_move_to_x + g_selection_start->x;
		mouse_pos.y = *g_move_to_y + g_selection_start->y;

		s32 distance_a = scbw::get_distance_fast(ua->position, mouse_pos);
		s32 distance_b = scbw::get_distance_fast(ub->position, mouse_pos);
		return distance_a - distance_b;
	}

	static void sort_unit_selection(Unit** unit_array) {
		u32 size = 0;
		for (Unit** it = unit_array; *it; ++it) {
			size++;
		}
		qsort(unit_array, size, sizeof(Unit*), compare_selection_unit);
	}

	// Function used by Ctrl+Click (select all units of same type)
	u32 sort_all_units(Unit* unit, Unit** unit_list, Unit** units_in_bounds)
	{
		sort_unit_selection(units_in_bounds);
		Unit* current_unit;
		Unit* backup_current_unit = 0;
		u32 current_index_in_unit_list;
		bool stop_big_loop = false;
		bool keep_for_empty_list_case = false;
		bool dont_add_to_list = false;
		
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
				if (units_dat::base_property[current_unit->id] & UnitProperty::Subunit) {
					current_unit = current_unit->subunit;
				}
				if (units_dat::base_property[current_unit->id] & UnitProperty::Building) {
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
			stop_big_loop = true;
		}
		while (!stop_big_loop)
		{
			if (current_unit != unit)
			{
				if (has_units) {
					if (units_dat::base_property[current_unit->id] & UnitProperty::Building) {
						dont_add_to_list = true;
					}
				} else if (has_buildings) {
					if (!(units_dat::base_property[current_unit->id] & UnitProperty::Building)) {
						dont_add_to_list = true;
					}
				}

				if (!dont_add_to_list) {
					// if current_unit == unit, then it's already in
					// unit_list
					if (current_unit->subunit)
					{
						// if what got it is the turret or another subunit, swap
						// the unit and the subunit as current_unit
						if (units_dat::base_property[current_unit->id] & UnitProperty::Subunit)
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
									dont_add_to_list = true;
								}
							}
							while (index != 0 && !dont_add_to_list);
						}
					}
				}

				if (!dont_add_to_list)
				{
					// 0046F161
					// if the unit sprite is visible, perform additionnal visibility test, else skip the unit
					if (current_unit->sprite->visibility_status & (u8)*g_player_visions)
					{
						// if the unit is cloaked and visibility status test (detection by detectors units probably) fail, the unit is skipped
						if (current_unit->status & (UnitStatus::Cloaked + UnitStatus::RequiresDetection))
						{
							if (!(current_unit->detection_status & *g_player_visions))
							{
								dont_add_to_list = true;
							}
						}
					}
					else
					{
						dont_add_to_list = true;
					}

					if (!dont_add_to_list)
					{
						// 0046F18C
						if (helper::unit_is_unselectable(current_unit->id))
						{
							dont_add_to_list = true;
						}
						else if (!helper::unit_is_standard_and_movable(current_unit))
						{
							keep_for_empty_list_case = true; // skip the unit for the list, unless it's the only available
						}
						else if (*g_is_in_replay)
						{
							keep_for_empty_list_case = true; // skip the unit for the list, unless it's the only available
						}
						else if (current_unit->player != *g_local_nation_id)
						{
							keep_for_empty_list_case = true; // skip the unit for the list, unless it's the only available
						}
						else
						{
							// 0046F1C5
							if (unit)
							{
								// all units should belong to the same player (the one owning the clicked unit)
								if (unit->player != current_unit->player)
								{
									dont_add_to_list = true;
								}
								// select only units with same Id
								else if (unit->id != current_unit->id)
								{
									dont_add_to_list = true;
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
										dont_add_to_list = true;
									}

									// Same thing for hallucinations
									if (mixed_flags & UnitStatus::IsHallucination)
									{
										dont_add_to_list = true;
									}

									// Same thing for completed
									if (mixed_flags & UnitStatus::Completed)
									{
										dont_add_to_list = true;
									}

									// Same thing for lifted
									if (mixed_flags & UnitStatus::InAir)
									{
										dont_add_to_list = true;
									}
								}
							}

							if (!dont_add_to_list)
							{
								// 0046F203
								// 0x0046F208 (use of SelectionCap)
								if (current_index_in_unit_list >= SelectionCap) // action when unit_list is full
								{
									stop_big_loop = true; // This was added, so sorting by cursor is respected, not sure what was the role of the original call

									// helper::function_0046F040(current_unit, unit_list, unit, current_index_in_unit_list);
								}
								else
								{
									if (units_dat::base_property[current_unit->id] & UnitProperty::Building) {
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

			if (keep_for_empty_list_case)
			{
				backup_current_unit = current_unit;
			}

			// 0046F226
			// select next unit in bounds
			units_in_bounds += 1; // this actually advance by 4 bytes
			current_unit = *units_in_bounds;
			keep_for_empty_list_case = false;
			dont_add_to_list = false;

			// continue until current_unit is nullptr
			if (!current_unit)
			{
				stop_big_loop = true;
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
					current_index_in_unit_list = 1; // return 1 (an unit preserved by keep_for_empty_list_case)
				}
			}
		}	

		return current_index_in_unit_list;
	}

	/// Function used by Click on unit (possibly with key pressed)
	void unit_selection_click(Unit* clicked_unit)
	{
		Unit* local_temp_array_1[SelectionCap];	// used instead of an array starting from [EBP-3C]
		Unit* local_temp_array_2[SelectionCap];	// used instead of an array starting from [EBP-6C]
		Box16 local_temp_box16_structure;		// used instead of using from [EBP-0C] to [EBP-06]
		bool holding_ctrl_or_double_clicking = *g_is_holding_ctrl || (*g_is_double_clicking && (clicked_unit->sprite->flags & SpriteFlags::Selected));
		for (s32 i = 0; i < SelectionCap; i++)
		{
			local_temp_array_1[i] = 0;
		}

		if (holding_ctrl_or_double_clicking || (*g_is_holding_shift && active_player_selection->unit[0]))
		{
			if (!*g_is_holding_shift)
			{
				// 0046FE07
				if (holding_ctrl_or_double_clicking)
				{
					// Ctrl+click on unit or f64-click on already
					// it unit without other key
					// =>	select surrounding units with same type as clicked unit, making them the current unit_list

					Unit** units_in_bounds;
					u32 sorted_list_length;

					// locate surrounding units for unit_list
					local_temp_box16_structure.left = *g_move_to_x;
					local_temp_box16_structure.top = *g_move_to_y;
					local_temp_box16_structure.right = *g_move_to_x + 640;
					local_temp_box16_structure.bottom = *g_move_to_y + 400;
					units_in_bounds = helper::get_all_units_in_bounds(&local_temp_box16_structure);
					sorted_list_length = sort_all_units(clicked_unit, local_temp_array_1, units_in_bounds);

					// reload the previous temporary unit list from before the call to get_all_units_in_bounds
					*g_temp_units_list_arrays_counts_list_last_index = *g_temp_units_list_arrays_counts_list_last_index - 1;
					*g_temp_units_list_current_array_count = g_temp_units_list_arrays_counts_list[*g_temp_units_list_arrays_counts_list_last_index];

					if (sorted_list_length != 0)
					{
						// turn the local array as the newly applied unit_list
						helper::select_units_if_alt_not_held(local_temp_array_1, sorted_list_length, 1, 1);

						// This is what make the rally point briefly appear when selecting a building with one. Maybe can do other things?
						if (sorted_list_length == 1
							&& clicked_unit->player == *g_active_nation_id
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
				if (holding_ctrl_or_double_clicking)
				{
					// {Ctrl+Shift+click on unit} OR {Shift+f64-click on 
					// it unit WHILE having an active unit_list}
					// select units same type as clicked unit, adding them to the
					// active unit_list

					Unit** units_in_bounds;
					u32 sorted_list_length;

					// prepare to locate surrounding units for unit_list
					local_temp_box16_structure.left = *g_move_to_x;
					local_temp_box16_structure.top = *g_move_to_y;
					local_temp_box16_structure.right = *g_move_to_x + 640;
					local_temp_box16_structure.bottom = *g_move_to_y + 400;

					// save existing unit_list to a temporary array
					for (s32 i = 0; i < SelectionCap; i++)
					{
						local_temp_array_2[i] = active_player_selection->unit[i];
					}

					// locate surrounding units for unit_list
					units_in_bounds = helper::get_all_units_in_bounds(&local_temp_box16_structure);
					sorted_list_length = sort_all_units(clicked_unit, local_temp_array_1, units_in_bounds);

					// reload the previous temporary unit list from before the call to get_all_units_in_bounds
					*g_temp_units_list_arrays_counts_list_last_index = *g_temp_units_list_arrays_counts_list_last_index - 1;
					*g_temp_units_list_current_array_count = g_temp_units_list_arrays_counts_list[*g_temp_units_list_arrays_counts_list_last_index];

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
					s32 array_index;
					// copy the existing unit_list into the local array
					for (array_index = 0; array_index < SelectionCap && active_player_selection->unit[array_index]; array_index++)
					{
						local_temp_array_1[array_index] = active_player_selection->unit[array_index];
					}

					if (!(clicked_unit->sprite->flags & SpriteFlags::Selected))
					{
						// 0x0046FD13 (getting unit sprite) and 0x0046FD18 (middle of the test with the flag)
						// unit not it, so it's added to current unit_list if valid
						if (array_index < SelectionCap
							&& helper::unit_is_standard_and_movable(local_temp_array_1[0])
							&& helper::unit_is_owned_by_current_player(local_temp_array_1[0])
							&& helper::unit_is_standard_and_movable(clicked_unit)
							&& helper::unit_is_owned_by_current_player(clicked_unit))
						{
							// new unit is added at end of array, and the index is increased to be used as new array length
							local_temp_array_1[array_index] = clicked_unit;
							array_index++;

							// turn local_temp_array_1 as the newly applied unit_list
							helper::select_units_if_alt_not_held(local_temp_array_1, array_index, 1, 1);
						}
					}
					else
					{
						// 0046FD77
						// unit already it, remove it from unit_list
						u32 memcpy_size;
						bool bUpdateSelection = true;

						// decrease it so it's equal to the index of last element
						array_index--;
						// each element is 4 bytes, multiply it by the number of elements after the one to remove
						memcpy_size = (array_index - clicked_unit->sprite->selection_index) * 4;

						// Copy the elements on the right of the one to delete one step left to overwrite the one to delete
						helper::sc_memcpy_0((u32)&local_temp_array_1[clicked_unit->sprite->selection_index], (u32)&local_temp_array_1[clicked_unit->sprite->selection_index + 1], memcpy_size);

						// either erase the element to delete or one that was repeated after memcpy
						local_temp_array_1[array_index] = 0;

						if (array_index == 1)
						{
							if (*g_is_holding_alt)
							{
								// It seems you can unselect things with Shift+Alt like
								// with shift alone, UNTIL you have 2 units it.
								// It is the code below that is causing the
								// deselection to fail where simple shift+click would
								// continue to deselect.
								u32 unit_index = helper::unit_to_unit_id(local_temp_array_1[0]);
								if (helper::select_single_unit_from_id(unit_index) != 0)
								{
									bUpdateSelection = false;
								}
							}
						}

						if (bUpdateSelection)
						{
							// Create a new unit unit_list (since the clicked unit is unselected)
							// and choose it as current unit_list)
							helper::create_new_unit_selections_from_list(local_temp_array_1, array_index);
							helper::cmdact_select(local_temp_array_1, array_index);

							// tell the GUI to update stuff
							*g_control_under_mouse = 0;
							*g_control_under_mouse_value = 0;
							*g_client_selection_changed = 1;
							*g_force_button_refresh = 1;
							*g_portrait_redraw = 1;
							*g_status_screen_redraw = 1;

						}
					}
				}
			}
		}

		else
		{
			// 0046FBB7
			// Simple click...almost (there's still ALT key)
			bool recreated_selection_via_alt_click = false;
			if (*g_is_holding_alt)
			{
				// Recall the unit_list the unit belong to if there's one
				// In this case, skip the code for a single unit handling

				u32 temp_unit_index = helper::unit_to_unit_id(clicked_unit);
				if (helper::select_single_unit_from_id(temp_unit_index) != 0)
				{
					recreated_selection_via_alt_click = true;
				}
			}

			if (!recreated_selection_via_alt_click)
			{
				// 0046FBD1
				// Create a new unit_list containing only the clicked unit
				helper::create_new_unit_selections_from_list(&clicked_unit, 1);
				// play the new_sound of the unit (don't need to be a building)
				helper::select_building_sfx(clicked_unit);
				// select the unit
				helper::cmdact_select(&clicked_unit, 1);

				// tell the GUI to update stuff
				*g_client_selection_changed = 1;
				*g_force_button_refresh = 1;
				*g_portrait_redraw = 1;
				*g_status_screen_redraw = 1;
				*g_control_under_mouse = 0;
				*g_control_under_mouse_value = 0;
			}

			// 0046FC2C
			// this is what make the rally point briefly appear when
			// selecting a building with one.
			// maybe can do others things?
			if (clicked_unit->player == *g_active_nation_id
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
			if (units_dat::base_property[unit->id] & UnitProperty::Building) {
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
				if (units_dat::base_property[unit->id] & UnitProperty::Building) {
					list[count++] = unit;
				}
			} else if (!has_buildings) {
				if (!(units_dat::base_property[unit->id] & UnitProperty::Building)) {
					list[count++] = unit;
				}
			}
		}
		return count;
	}

	bool can_add_unit_to_active_selection(Unit* unit, u32 slot) {
		u32 player = *g_active_player_id;
		bool has_buildings = false;
		bool has_units = false;
		for (u32 i = 0; i < SelectionCap; ++i) {
			UnitIndex unit_index = control_group_array->player[player].control_group[slot].unit_index[i];
			if (!unit_index.index) {
				continue;
			}
			Unit* unit = &g_unit_table[unit_index.index - 1];
			if (!unit || !unit->sprite || unit->unique_id != unit_index.unique_id || !unit->main_order_id) {
				continue;
			}
			if (units_dat::base_property[unit->id] & UnitProperty::Building) {
				has_buildings = true;
			} else {
				has_units = true;
			}
		}

		if ((has_units && has_buildings) || !has_units) {
			if (units_dat::base_property[unit->id] & UnitProperty::Building) {
				return true;
			}
		} else if (!has_buildings) {
			if (!(units_dat::base_property[unit->id] & UnitProperty::Building)) {
				return true;
			}
		}
		return false;
	}
	
	// Hooked to forbid buildings. -Pr0nogo
	s32 select_multiple_units_from_unit_list(u32 count, Unit** unit_list, char play_sound, u32 send_command) { // 0049AEF0
		if (*g_is_holding_shift) {
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
		*g_client_selection_changed = 1;
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
			if (units_dat::base_property[unit->id] & UnitProperty::Subunit) {
				unit = unit->subunit;
			}
			helper::destroy_unit_selection(unit);
		}
		for (u32 i = 0; i < count; ++i) {
			Unit* unit = unit_list[i];
			active_player_selection->unit[i] = unit;
			if (units_dat::base_property[unit->id] & UnitProperty::Subunit) {
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
		*g_temp_units_list_current_array_count = g_temp_units_list_arrays_counts_list_last_index[g_temp_units_list_arrays_counts_list_last_index[0]--];
		if (sorted_units) {
			for (u32 i = 0; i < SelectionCap; ++i) {
				Unit* unit = new_units[i];
				if (unit && unit->player == *g_active_nation_id) {
					if (hooks::unit_can_rally(unit)) {
						helper::show_rally_target(unit);
						break; // Currently the game doesn't support showing more than 1 rally thingy XD
					}
				}
			}

			if (g_is_keycode_used[16] && active_player_selection[0].unit) {
				Unit* combined_units[SelectionCap] = {};
				memcpy(combined_units, active_player_selection, sizeof(UnitsSel));
				u32 added_units = helper::combine_lists(0, new_units, combined_units, sorted_units);
				helper::select_units_if_alt_not_held(combined_units, added_units, 1, 1);
				return;
			}

			if (sorted_units != 1
			|| !g_is_keycode_used[18]
			|| !helper::select_single_unit_from_id(helper::unit_to_unit_id(new_units[0]))) {
				helper::select_multiple_units_from_unit_list((u32)sorted_units, new_units, 1, 1);
				if (sorted_units != 1) {
					return;
				}
			}
		}
	}

	// Hooked to help debug hpbars.cpp hooks. -Pr0nogo
	void create_unit_selection(Unit* unit, char index) // 004E6180
	{
		Sprite* sprite = unit->sprite;
		if (units_dat::base_property[unit->id] & UnitStatus::Invincible)
		{
			if (!(sprite->flags & SpriteFlags::DrawSelCircle))
			{
				if (helper::create_selection_circle(sprite, g_selection_circle_color[(unsigned __int8)unit->player], ImageId::SelectionCircleDashed_22pixels))
				{
					sprite->flags |= SpriteFlags::DrawSelCircle;
				}
			}
			Sprite* sprite_copy = unit->sprite;
			sprite_copy->flags |= SpriteFlags::Selected;
			sprite_copy->selection_index = index;
		}
		else
		{
			u8 sprite_id = g_selection_circle_color[(unsigned __int8)unit->player];
			u8 sprite_flags = sprite->flags;
			if (!(sprite_flags & SpriteFlags::Selected))
			{
				sprite->selection_index = index;
				sprite->flags = sprite_flags | SpriteFlags::Selected;
				helper::create_healthbar(sprite);
				if (!(sprite->flags & SpriteFlags::DrawSelCircle))
				{
					if (helper::create_selection_circle(sprite, sprite_id, ImageId::SelectionCircleDashed_22pixels))
					{
						sprite->flags |= SpriteFlags::DrawSelCircle;
					}
				}
			}
			if (!*g_is_in_replay && unit->player == *g_local_nation_id)
			{
				if (unit->status & UnitStatus::Completed)
				{
					if (scbw::unit_id_is_power_generator(unit->id))
					{
						helper::create_pylon_aura();
					}
				}
			}
		}
	}

	// Hooked to help debug hpbars.cpp hooks. -Pr0nogo
	Image* create_selection_circle(Sprite* sprite, u8 color, u16 image_id) { // 004D7070
		Image* last_free_circle = *g_last_free_selection_circle;
		if (*g_last_free_selection_circle == 0) {
			return 0;
		}
		
		Image* next_free_circle = (*g_last_free_selection_circle)->link.next;
		*g_last_free_selection_circle = (*g_last_free_selection_circle)->link.next;
		if (*g_first_free_selection_circle == last_free_circle) {
			*g_first_free_selection_circle = last_free_circle->link.prev;
		}
		if (last_free_circle->link.prev) {
			last_free_circle->link.prev->link.next = next_free_circle;
		}
		if (next_free_circle) {
			next_free_circle->link.prev = last_free_circle->link.prev;
		}
		last_free_circle->link.prev = 0;
		next_free_circle = 0;

		Image* tail = sprite->images.tail;
		if (tail->palette_type == 10) {
			if (sprite->images.head == tail) {
				sprite->images.head = last_free_circle;
			}
			last_free_circle->link.prev = tail->link.prev;
			next_free_circle = tail;
			if (tail->link.prev)
			{
				tail->link.prev->link.next = last_free_circle;
			}
			tail->link.prev = last_free_circle;
		} else {
			if (sprite->images.tail == tail) {
				sprite->images.tail = last_free_circle;
			}
			last_free_circle->link.prev = tail;
			next_free_circle = tail->link.next;
			Image* next_tail = tail->link.next;
			if (next_tail) {
				next_tail->link.prev = last_free_circle;
			}
			tail->link.next = last_free_circle;
		}
		helper::create_selection_circle_image(last_free_circle, sprite, color, image_id);
		return last_free_circle;
	}

	// Hooked to investigate "archon mode".
	// Ended up being used for allied selection circles. -Pr0nogo
	b32 is_human_player_on_same_team(u32 player_id) { // 0049A110
		if (player_id != *g_local_human_id
		&&  player_id < PlayerLimit) { // Originally checked for singleplayer (turned off so sp replays load selections) and for a team game (turned off so it works at all).
			if (scbw::is_allied_to(player_id, *g_local_human_id) && scbw::is_allied_to(*g_local_human_id, player_id) && !globals.players[player_id].is_witness) {
				return true;
			}
		}
		if (*g_is_in_replay || globals.players[*g_local_human_id].is_witness) {
			return true;
		}
		return false;
	}

	// Hooked to not die to lard. -Pr0nogo
	u8 get_team_size(u32 selecting_player) { // 00485710
		if ((selecting_player < PlayerLimit && globals.players[selecting_player].is_witness)
		||	!scbw::is_allied_to(selecting_player, *g_local_human_id) 
		||	!scbw::is_allied_to(*g_local_human_id, selecting_player)) {
			return 2;
		}
		return 4;
	}

	// Hooked to investigate "archon mode".
	// Seems to only be called from lobbies, so it's probably not related to what I want. -Pr0nogo
	b32 game_is_team_game() { // 00484F50
		return *g_multiplayer_mode && game_data->teams_enabled;
	}

	// Hooked to forbid witness selections from showing. -Pr0nogo
	void create_dashed_selection_main(Unit* unit, u32 selecting_player) { // 004E65CD
		if (!*g_is_in_replay && *g_local_human_id == selecting_player) {
			return;
		}
		if (selecting_player < PlayerLimit && globals.players[selecting_player].is_witness) {
			return;
		}
		helper::create_dashed_selection(selecting_player, unit->sprite, g_selection_circle_color[unit->player]);
	}
}