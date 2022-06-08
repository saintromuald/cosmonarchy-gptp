#pragma once
#include "CMDRECV_Morph.h"
#include <SCBW/api.h>
#include "hooks/rally_point.h"

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
			if (hooks::unit_can_rally(unit))
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

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	void process_right_click_action(Unit* unit, u32 order_unit_id, u16 y_pos, u32 x_pos, Unit* target) // 00455F20
	{
		Box16 location = {};
		location.x = x_pos - 96;
		location.w = x_pos + 96;
		location.h = y_pos + 96;
		location.y = y_pos - 96;
		unk_order_array[0] = 23;
		*is_unit_in_point_x_pos = x_pos;
		*is_unit_in_point_y_pos = y_pos;
		u8 idle_order = helper::get_generic_unit_idle_order(order_unit_id, unit);
		Unit* unit_in_point = helper::iterate_units_at_location_target_proc_in_point(&location, target);
		u8 right_click_order = helper::get_right_click_order(idle_order, unit_in_point, x_pos, y_pos, &unit_in_point, UnitId::None);
		if (right_click_order == OrderId::Nothing2)
		{
			helper::set_resource_target(unit);
			u32 x_move_target = unit->move_target.pt.x;
			u32 y_move_target = unit->move_target.pt.y;
			if (x_move_target != unit->next_target_waypoint.x || y_move_target != unit->next_target_waypoint.y)
			{
				unit->next_target_waypoint.x = x_move_target;
				unit->next_target_waypoint.y = y_move_target;
			}
			if (unit->order_queue_head)
			{
				unit->user_action_flags |= 1u;
				helper::orders_nothing_2(unit);
			}
			else if (unit->unit_ai)
			{
				helper::issue_order_targeting_nothing(unit, OrderId::ComputerAI);
			}
			else
			{
				helper::issue_order_targeting_nothing(unit, units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
		else
		{
			if (right_click_order == 8)
			{
				right_click_order = units_dat::AttackUnitOrder[unit->id];
			}
			if (helper::order_allowed(unit, right_click_order, unit->player) == 1)
			{
				if (right_click_order == OrderId::RallyPoint1)
				{
					if (hooks::unit_can_rally(unit))
					{
						helper::set_rally_unit(unit_in_point, unit);
					}
				}
				else if (right_click_order == OrderId::RallyPoint2)
				{
					if (hooks::unit_can_rally(unit))
					{
						helper::set_rally_position(unit, x_pos, y_pos);
					}
				}
				else
				{
					if (unit_in_point)
					{
						helper::prepend_order_targeting_unit(unit_in_point, unit, right_click_order);
					}
					else
					{
						helper::prepend_order_targeting_ground(right_click_order, x_pos, y_pos, unit);
					}
					helper::to_idle(unit);
				}
			}
		}
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	u8 get_generic_unit_idle_order(u32 order_unit_id, Unit* unit) // 00455760
	{
		if (!unit)
		{
			return OrderId::Die;
		}

		u8 result = 0;
		if (unit->id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed)
		{
			result = OrderId::PlayerGuard;
		}
		else
		{
			u8 right_click_order = units_dat::RightClickAction[unit->id];
			if (!right_click_order)
			{
				if (unit->status & UnitStatus::GroundedBuilding)
				{
					if (helper::unit_can_rally(unit))
					{
						right_click_order = OrderId::Guard;
					}
				}
			}
			result = right_click_order;
		}
		if (unit->status & UnitStatus::IsHallucination)
		{
			if (result == OrderId::TurretGuard || result == OrderId::BunkerGuard)
			{
				result = OrderId::Stop;
			}
		}
		return result;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	void input_game_right_mouse_click(DialogEvent* dlg_event) // 004564E0
	{
		if (!helper::s_trans_442(dlg_event->current.x, dlg_event->current.y))
		{
			u8 local_group_owner = local_selection_group[0] ? local_selection_group[0]->player : -1;
			if (local_group_owner == *LOCAL_NATION_ID)
			{
				if (active_portrait_unit)
				{
					if (helper::unit_is_owned_by_current_player(*active_portrait_unit))
					{
						s32 selection_iterator = 0;
						while (1)
						{
							Unit* selected_unit = *client_selection_group[selection_iterator].unit;
							if (selected_unit)
							{
								if (helper::accepts_right_click_commands(selected_unit))
								{
									break;
								}
							}
							if (++selection_iterator >= 12)
							{
								return;
							}
						}
						if (selection_iterator < 12)
						{
							u32 find_x = *move_to_x + dlg_event->current.x;
							u32 find_y = *move_to_y + dlg_event->current.y;
							u32 find_x_var = find_x;
							Unit* unit_at_point = helper::find_unit_at_point(find_x, find_y);
							Unit* unit_at_point_var = unit_at_point;
							if (*client_selection_count != 1
								|| *active_portrait_unit != unit_at_point
								|| unit_at_point && unit_at_point->status & 2 && helper::unit_can_rally(unit_at_point))
							{
								if (unit_at_point_var)
								{
									goto LABEL_33;
								}
								s32 selection_iterator_2 = 0;
								do
								{
									Unit* selected_unit_2 = *client_selection_group[selection_iterator_2].unit;
									if (selected_unit_2)
									{
										if (helper::can_move(selected_unit_2))
										{
											break;
										}
									}
									++selection_iterator_2;
								} while (selection_iterator_2 < 12);
								if (selection_iterator_2 < 12)
								{
								LABEL_33:
									Thingy* command_thingy = helper::show_command_response(find_x_var, find_y, unit_at_point_var);
									u16 unit_id_xd = UnitId::None;
									if (command_thingy)
									{
										unit_id_xd = command_thingy->unit_id;
									}
									helper::cmdact_right_click_order(unit_at_point_var, find_x_var, find_y, unit_id_xd, IS_KEYCODE_USED[16]);
									if (helper::show_right_click_error_if_needed(unit_at_point_var))
									{
										helper::play_worker_action_sound(*active_portrait_unit);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	u32 can_do_targeted_order(Unit* unit, u8 order_id) // 0049A410
	{
		if (unit->status & UnitStatus::GroundedBuilding
		&&    (order_id == OrderId::RallyPoint1
			|| order_id == OrderId::RallyPoint2
			|| order_id == OrderId::Rechargeshields2
			|| order_id == OrderId::Pickup3))
		{
			return 1;
		}
		if (unit->id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed)
		{
			return 1;
		}
		u8 right_click_order = units_dat::RightClickAction[unit->id];
		if (!right_click_order)
		{
			if (unit->status & UnitStatus::GroundedBuilding)
			{
				if (helper::unit_can_rally(unit))
				{
					right_click_order = 2;
				}
			}
		}
		return right_click_order != 0;
	}

	// Hooked for right-click rally on eggs and cocoons. -Pr0nogo
	char get_right_click_action_order(Unit* unit) // 004E5EA0
	{
		if (unit->id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed)
		{
			return 3;
		}

		char right_click_order = units_dat::RightClickAction[unit->id];
		if (!right_click_order)
		{
			if (hooks::unit_can_rally(unit))
			{
				right_click_order = 2;
			}
		}
		return right_click_order;
	}
}