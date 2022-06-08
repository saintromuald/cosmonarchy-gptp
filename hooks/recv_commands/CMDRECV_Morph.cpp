#pragma once
#include "CMDRECV_Morph.h"
#include <SCBW/api.h>

namespace hooks
{
	void cmdrecv_building_morph(u16 unit_id)
	{
		*unit_iterator_pos = 0;
		while (Unit* unit = helper::get_active_player_next_selection()) {
			if (unit->id >= UnitId::ZergInfestedCommandCenter && unit->id < UnitId::None &&
				unit_id >= UnitId::ZergInfestedCommandCenter && unit_id < UnitId::None &&
				unit->can_make_unit(unit_id, *ACTIVE_NATION_ID) == 1 &&
				helper::has_money_can_make(unit, unit_id) && // start construction if successful
				unit->main_order_id != OrderId::Morph2) {
				unit->order_computer(OrderId::Morph2);
				scbw::refresh_console();
			}
		};
	}

	void cmdrecv_unit_morph(u16 wUnitType)
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
		while (Unit* unit = helper::get_active_player_next_selection()) {
			u8 unit_id = buffer[1];
			if (helper::create_unit_allowed(unit_id, unit, *ACTIVE_NATION_ID)) {
				if (helper::has_money_can_make(unit, unit_id)) {
					helper::set_secondary_order(unit, OrderId::Train);
				}
				scbw::refresh_console();
			}
		}
	}

	// Hooked for multi-building select. -Pr0nogo
	void cmdrecv_liftoff(char* buffer) { // 004C1620
		*unit_iterator_pos = 0;
		while (Unit* unit = helper::get_active_player_next_selection()) {
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
						if (unit->id == UnitId::TerranMason && unit->main_order_id == OrderId::ConstructingBuilding
							|| scbw::unit_can_nuke(unit) && unit->main_order_id == OrderId::NukeTrack
							|| unit->id == UnitId::ProtossAugur && unit->main_order_id == OrderId::CompletingArchonSummon)	{
							*last_internal_error = 8;
						} else if (orders_dat::RequirementsOffset[OrderId::BuildingLiftoff]) {
							if (helper::parse_requirement_opcodes(unit, OrderId::BuildingLiftoff, *ACTIVE_NATION_ID, (u32)orders_requirement_tree, orders_dat::RequirementsOffset[OrderId::BuildingLiftoff]) == 1) {
								u16* buffer_u16 = (u16*)(buffer + 1);
								u16 x = buffer_u16[0];
								u16 y = buffer_u16[1];
								if (x < *map_width_pixels && y < *map_height_pixels) {
									helper::issue_order_targeting_ground(unit, OrderId::BuildingLiftoff, x, y);
									helper::do_order(unit, units_dat::ReturnToIdleOrder[unit->id], x, y - 42, 0);
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

	// Hooked for contextual orders. -Pr0nogo
	u32 order_acquire_generic_right_click_unit_proc(Unit* unit, u32 x_pos, u16 y_pos, Unit** target, u16 fow_unit_id) { // 00455CC0 - guessed x_pos and y_pos
		Unit* target_unit = *target;
		if (!target_unit) {
			return helper::order_acquire_generic_idle_unit_proc(unit, x_pos, y_pos, target, fow_unit_id);
		}
		u32 result = helper::order_acquire_generic_right_click_harvest_proc(target_unit, unit);
		if (result == OrderId::None) {
			if (!(target_unit->status & UnitStatus::GroundedBuilding) || target_unit->status & UnitStatus::Completed) {
				if (scbw::is_allied_to(unit->player, target_unit->player)) {
					if (!(units_dat::BaseProperty[target_unit->id] & UnitProperty::Building)
					&&  helper::unit_is_active_transport(target_unit)
					&&  helper::can_enter_transport(target_unit, unit)) {
						return OrderId::EnterTransport;
					}
					if (helper::is_order_valid(OrderId::Repair1, unit, (s32)target_unit, target_unit, 0)) { // this helper is hilarious
						return OrderId::Repair1;
					}
					if (helper::unit_is_active_transport(target_unit) && helper::can_enter_transport(target_unit, unit)) {
						return OrderId::EnterTransport;
					}
				}
			} else if (target_unit->player == unit->player && target_unit->get_race() == RaceId::Terran
				&&   !(units_dat::BaseProperty[target_unit->id] & UnitProperty::Addon)) {
				if (!target_unit->connected_unit || target_unit->connected_unit->order_target.unit != target_unit) {
					return OrderId::ConstructingBuilding;
				}
			}
			return helper::order_acquire_generic_idle_unit_proc(unit, x_pos, y_pos, target, fow_unit_id);
		}
		return result;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	u32 order_acquire_generic_right_click_unit_proc_2(Unit* unit, u32 x_pos, u16 y_pos, Unit** target, u16 fow_unit_id) // 00455E00
	{
		u32 result = OrderId::Nothing2;
		if (unit->status & UnitStatus::GroundedBuilding || unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon)
		{
			if (helper::unit_is_factory(unit) || unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon)
			{
				result = OrderId::RallyPoint2 - (*target != 0);
			}
			return result;
		}

		Unit* target_unit = *target;
		if (!*target)
		{
			if ((unit->id == UnitId::ZergMatraleth) && fow_unit_id != UnitId::None)
			{
				return OrderId::Rightclickaction;
			}
			return OrderId::Move;
		}
		if (helper::unit_is_active_transport(unit) && helper::is_order_valid(OrderId::Pickup2, unit, 0, target_unit, 0))
		{
			return OrderId::Pickup2;
		}
		if (helper::unit_is_active_transport(target_unit) && helper::can_enter_transport(target_unit, unit))
		{
			return OrderId::EnterTransport;
		}
		if (target_unit->status & UnitStatus::Burrowed)
		{
			return OrderId::Move;
		}

		if (unit->id != UnitId::ZergMatraleth
		||  target_unit->status & UnitStatus::Invincible
		||  helper::get_target_unit_ability_error_stat_string(target_unit, unit->player, 12) != 0)
		{
			result = (unit->id != UnitId::TerranCleric ? 0 : 0x80) + OrderId::Follow;
		}
		else
		{
			result = OrderId::InfestMine2;
		}
		return result;
	}
}