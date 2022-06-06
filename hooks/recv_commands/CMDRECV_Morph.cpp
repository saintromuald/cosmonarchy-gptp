#pragma once
#include "CMDRECV_Morph.h"
#include <SCBW/api.h>

namespace hooks
{
	void CMDRECV_BuildingMorph(u16 unit_id)
	{
		*unit_iterator_pos = 0;
		for (Unit* selection = get_active_player_next_selection(); selection; selection = get_active_player_next_selection()) {
			if (selection &&
				selection->id >= UnitId::ZergInfestedCommandCenter &&
				selection->id <= 227 &&
				unit_id >= UnitId::ZergInfestedCommandCenter &&
				unit_id <= 227 &&
				selection->can_make_unit(unit_id, *ACTIVE_NATION_ID) == 1 &&
				helper::has_money_can_make(selection, unit_id) && //start construction if successful
				selection->main_order_id != OrderId::Morph2) {
				selection->order_computer(OrderId::Morph2);
				scbw::refresh_console();
			}
		};
	}

	void CMDRECV_UnitMorph(u16 wUnitType)
	{
		Unit* current_unit;
		*unit_iterator_pos = 0;
		current_unit = get_active_player_next_selection();
		while (current_unit != nullptr)
		{

			bool bAllowMorph;
			/*
			if(
				current_unit->id == UnitId::ZergHydralisk &&
				unit_id == UnitId::ZergLakizilisk
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
			current_unit = get_active_player_next_selection();
		}
		scbw::refresh_console();
	}

	// Hooked for multi-unit select. -Pr0nogo
	void cmdrecv_train(char* buffer) { // 004C1C20
		*unit_iterator_pos = 0;
		for (Unit* selection = get_active_player_next_selection(); selection; selection = get_active_player_next_selection()) {
			if (helper::create_unit_allowed(*(buffer + 1), selection, *ACTIVE_NATION_ID)) {
				u16 unit_id = *(buffer + 1);
				if (unit_id <= UnitId::Spell_DisruptionWeb) {
					if (helper::has_money_can_make(selection, unit_id)) {
						helper::set_secondary_order(selection, OrderId::Train);
					}
					scbw::refresh_console();
				}
			}
		}
	}

	// Hooked for multi-building select. -Pr0nogo
	void cmdrecv_liftoff(char* buffer) { // 004C1620
		*unit_iterator_pos = 0;
		for (Unit* unit = helper::get_active_player_next_selection(); unit; unit = helper::get_active_player_next_selection()) {
			if (!helper::unit_is_training_or_morphing(unit)
				&& unit->building.tech_id == TechId::None
				&& unit->building.upgrade_id == UpgradeId::None) {
				if (unit->status & UnitStatus::GroundedBuilding) {
					if (units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding) {
						*last_internal_error = 0;
						if (orders_dat::RequirementsOffset[72] == 65535) {
							helper::parse_orders_dat_requirements();
						}
						if (*ACTIVE_NATION_ID == unit->player) {
							u32 status = unit->status;
							if (status & UnitStatus::Completed) {
								if (status & UnitStatus::Frozen || unit->lockdown_timer || unit->stasis_timer || unit->maelstrom_timer) {
									*last_internal_error = 10;
								} else if (status & UnitStatus::Burrowed && unit->id != UnitId::TerranCommandCenter) {
									*last_internal_error = 11;
								} else {
									u16 unit_id = unit->id;
									if (unit_id == UnitId::TerranMason && unit->main_order_id == OrderId::ConstructingBuilding
										|| scbw::unit_can_nuke(unit) && unit->main_order_id == OrderId::NukeTrack
										|| unit_id == UnitId::ProtossAugur && unit->main_order_id == OrderId::CompletingArchonSummon)	{
										*last_internal_error = 8;
									} else if (orders_dat::RequirementsOffset[72]) {
										if (helper::parse_requirement_opcodes(unit, OrderId::BuildingLiftoff, *ACTIVE_NATION_ID, *orders_requirement_tree == 1, orders_dat::RequirementsOffset[72])) {
											s32 buffer_width = *(buffer + 1);
											if (buffer_width < *map_width_pixels) {
												s32 buffer_height = *(buffer + 3);
												if (buffer_height < *map_height_pixels) {
													helper::issue_order_targeting_ground(unit, OrderId::BuildingLiftoff, buffer_width, buffer_height);
													helper::do_order(unit, units_dat::ReturnToIdleOrder[unit->id], *(buffer + 1), *(buffer + 3) - 42, 0);
												}
											}
										}
									} else {
										*last_internal_error = 23;
									}
								}
							} else {
								*last_internal_error = 20;
							}
						} else {
							*last_internal_error = 1;
						}
					}
				}
			}
		}
	}
}