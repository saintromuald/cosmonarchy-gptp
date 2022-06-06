#pragma once
#include "CMDRECV_Morph.h"
#include <SCBW/api.h>

namespace hooks
{
	void CMDRECV_BuildingMorph(u16 wUnitType)
	{

		Unit* building;
		*selectionIndexStart = 0;
		building = getActivePlayerNextSelection();
		if (
			building != nullptr &&
			getActivePlayerNextSelection() == nullptr &&
			building->id >= UnitId::ZergInfestedCommandCenter &&
			building->id <= 227 &&
			wUnitType >= UnitId::ZergInfestedCommandCenter &&
			wUnitType <= 227 &&
			building->can_make_unit(wUnitType, *ACTIVE_NATION_ID) == 1 &&
			helper::has_money_can_make(building, wUnitType) && //start construction if successful
			building->main_order_id != OrderId::Morph2
			)
		{
			building->order_computer(OrderId::Morph2);
			scbw::refresh_console();
		}
	}

	void CMDRECV_UnitMorph(u16 wUnitType)
	{
		Unit* current_unit;
		*selectionIndexStart = 0;
		current_unit = getActivePlayerNextSelection();
		while (current_unit != nullptr)
		{

			bool bAllowMorph;
			/*
			if(
				current_unit->id == UnitId::ZergHydralisk &&
				wUnitType == UnitId::ZergLakizilisk
			)
				bAllowMorph = current_unit->canUseTech(TechId::LurkerAspect,*ACTIVE_NATION_ID) == 1;
			else*/
			if ((current_unit->id, true)
				)
			{

				bAllowMorph = current_unit->can_make_unit(wUnitType, *ACTIVE_NATION_ID) == 1;
			}
			else
			{
				bAllowMorph = false;
			}
			if (
				bAllowMorph &&
				current_unit->main_order_id != OrderId::Morph1 &&
				helper::has_money_can_make(current_unit, wUnitType) // start construction if successful
				)
			{
				current_unit->user_action_flags |= 1;
				if (current_unit->main_order_id != OrderId::Die)
				{
					bool bKeepLooping = true;
					while (bKeepLooping)
					{
						if (current_unit->order_queue_tail == nullptr)
							bKeepLooping = false;
						else
							if (orders_dat::CanBeInterrupted[current_unit->order_queue_tail->orderId] ||
								current_unit->order_queue_tail->orderId == OrderId::Morph1
								)
								helper::remove_order_from_unit_queue(current_unit->order_queue_tail, current_unit);
							else
								bKeepLooping = false;
					}
					current_unit->perform_another_order(OrderId::Morph1, 0, 0, nullptr, UnitId::None, 0);
				}
				prepare_for_next_order(current_unit);
			}
			current_unit = getActivePlayerNextSelection();
		}
		scbw::refresh_console();
	}

	// Hooked for multi-building select. -Pr0nogo
	void cmdrecv_train(char* buffer) // 004C1C20
	{
		*unit_iterator_pos = 0; // XD
		Unit* selection = helper::get_active_player_first_selection();
		if (selection)
		{
			if (helper::create_unit_allowed(*(buffer + 1), selection, *ACTIVE_NATION_ID))
			{
				u16 unit_id = *(buffer + 1);
				if (unit_id <= UnitId::Spell_DisruptionWeb)
				{
					if (helper::has_money_can_make(selection, unit_id))
					{
						helper::set_secondary_order(selection, OrderId::Train);
					}
					*force_button_refresh = 1;
					portrait_redraw[0] = 1;
					*status_screen_redraw = 1;
					*control_under_mouse = 0;
					*control_under_mouse_value = 0;
				}
			}
		}
	}

	// Hooked for multi-building select. -Pr0nogo
	void cmdrecv_liftoff(char* buffer) // 004C1620
	{
		*unit_iterator_pos = 0;
		Unit* selection = helper::get_active_player_first_selection();
		if (selection)
		{
			if (!helper::get_active_player_first_selection()
				&& !helper::unit_is_training_or_morphing(selection)
				&& selection->building.tech_id == 44
				&& selection->building.upgrade_id == 61)
			{
				if (selection->status & UnitStatus::GroundedBuilding)
				{
					if (units_dat::BaseProperty[selection->id] & UnitProperty::FlyingBuilding)
					{
						*last_internal_error = 0;
						if (orders_dat::RequirementsOffset[72] == -1)
						{
							helper::parse_orders_dat_requirements();
						}
						if (*ACTIVE_NATION_ID == selection->player)
						{
							u32 status = selection->status;
							if (status & UnitStatus::Completed)
							{
								if (status & UnitStatus::Frozen || selection->lockdown_timer || selection->stasis_timer || selection->maelstrom_timer)
								{
									*last_internal_error = 10;
								}
								else if (status & UnitStatus::Burrowed && selection->id != UnitId::TerranCommandCenter)
								{
									*last_internal_error = 11;
								}
								else
								{
									u16 unit_id = selection->id;
									if (unit_id == UnitId::TerranMason && selection->main_order_id == OrderId::ConstructingBuilding
										|| scbw::unit_can_nuke(selection) && selection->main_order_id == OrderId::NukeTrack
										|| unit_id == UnitId::ProtossAugur && selection->main_order_id == OrderId::CompletingArchonSummon)
									{
										*last_internal_error = 8;
									}
									else if (orders_dat::RequirementsOffset[72])
									{
										if (helper::parse_requirement_opcodes(selection, OrderId::BuildingLiftoff, *ACTIVE_NATION_ID, *orders_requirement_tree == 1, orders_dat::RequirementsOffset[72]))
										{
											s32 buffer_width = *(buffer + 1);
											if (buffer_width < *map_width_pixels)
											{
												s32 buffer_height = *(buffer + 3);
												if (buffer_height < *map_height_pixels)
												{
													helper::issue_order_targeting_ground(selection, OrderId::BuildingLiftoff, buffer_width, buffer_height);
													helper::do_order(selection, units_dat::ReturnToIdleOrder[selection->id], *(buffer + 1), *(buffer + 3) - 42, 0);
												}
											}
										}
									}
									else
									{
										*last_internal_error = 23;
									}
								}
							}
							else
							{
								*last_internal_error = 20;
							}
						}
						else
						{
							*last_internal_error = 1;
						}
					}
				}
			}
		}
	}

	// Hooked for multi-building select. -Pr0nogo
	void btnsact_rally_point(u32 xd) // 004244A0
	{
		*order_id_ground = OrderId::RallyPoint2;
		*order_id_obstructed = OrderId::RallyPoint2;
		bool network_table_1 = *network_table < 0x3Eu;
		bool network_table_2 = *network_table == 62;
		*is_targeting = 1;
		*order_id_unit = OrderId::RallyPoint1;

		const char* string = (const char*)(*network_table + network_table[63]);
		if (network_table_1 || network_table_2)
		{
			string = "";
		}

		helper::send_message_something(xd, string);
		helper::set_ingame_input_procs();
		*unk_rally_related = 2;
		*orders_buttonset = 229;
		helper::update_current_buttonset();
	}
}