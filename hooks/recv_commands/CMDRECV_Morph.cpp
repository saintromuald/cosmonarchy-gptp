#pragma once
#include "CMDRECV_Morph.h"
#include <SCBW/api.h>

namespace hooks
{
	void CMDRECV_BuildingMorph(u16 unit_id)
	{
		*unit_iterator_pos = 0;
		for (Unit* unit = get_active_player_next_selection(); unit; unit = get_active_player_next_selection()) {
			if (unit &&
				unit->id >= UnitId::ZergInfestedCommandCenter && unit->id < UnitId::None &&
				unit_id >= UnitId::ZergInfestedCommandCenter && unit_id < UnitId::None &&
				unit->can_make_unit(unit_id, *ACTIVE_NATION_ID) == 1 &&
				helper::has_money_can_make(unit, unit_id) && // start construction if successful
				unit->main_order_id != OrderId::Morph2) {
				unit->order_computer(OrderId::Morph2);
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
		for (Unit* unit = get_active_player_next_selection(); unit; unit = get_active_player_next_selection()) {
			if (helper::create_unit_allowed(*(buffer + 1), unit, *ACTIVE_NATION_ID)) {
				u16 unit_id = *(buffer + 1);
				if (unit_id <= UnitId::Spell_DisruptionWeb) {
					if (helper::has_money_can_make(unit, unit_id)) {
						helper::set_secondary_order(unit, OrderId::Train);
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
			if (!helper::unit_is_training_or_morphing(unit) && unit->building.tech_id == TechId::None && unit->building.upgrade_id == UpgradeId::None) {
				if (unit->status & UnitStatus::GroundedBuilding) {
					if (units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding) {
						*last_internal_error = 0;
						if (orders_dat::RequirementsOffset[OrderId::BuildingLiftoff] == 65535) {
							helper::parse_orders_dat_requirements();
						}
						if (unit->player != *ACTIVE_NATION_ID) {
							*last_internal_error = 1;
							continue;
						}
						if (!(unit->status & UnitStatus::Completed)) {
							*last_internal_error = 23;
							continue;
						}
						if (unit->status & UnitStatus::Frozen || unit->lockdown_timer || unit->stasis_timer || unit->maelstrom_timer) {
							*last_internal_error = 10;
							continue;
						}
						if (unit->status & UnitStatus::Burrowed && unit->id != UnitId::TerranCommandCenter) {
							*last_internal_error = 11;
							continue;
						}
						u16 unit_id = unit->id;
						if (unit_id == UnitId::TerranMason && unit->main_order_id == OrderId::ConstructingBuilding
							|| scbw::unit_can_nuke(unit) && unit->main_order_id == OrderId::NukeTrack
							|| unit_id == UnitId::ProtossAugur && unit->main_order_id == OrderId::CompletingArchonSummon)	{
							*last_internal_error = 8;
						} else if (orders_dat::RequirementsOffset[OrderId::BuildingLiftoff]) {
							if (helper::parse_requirement_opcodes(unit, OrderId::BuildingLiftoff, *ACTIVE_NATION_ID, *orders_requirement_tree == 1, orders_dat::RequirementsOffset[72])) {
								s32 width = *(buffer + 1);
								if (width < *map_width_pixels) {
									s32 height = *(buffer + 3);
									if (height < *map_height_pixels) {
										helper::issue_order_targeting_ground(unit, OrderId::BuildingLiftoff, width, height);
										helper::do_order(unit, units_dat::ReturnToIdleOrder[unit->id], *(buffer + 1), *(buffer + 3) - 42, 0);
									}
								}
							}
						} else {
							*last_internal_error = 20;
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