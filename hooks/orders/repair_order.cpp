#include "repair_order.h"
#include <SCBW/api.h>
#include <hooks/rally_point.h>
#include "globals.h"

namespace hooks
{
	void orders_repair_1(Unit * unit) { // 004673D0
		Unit* target = unit->order_target.unit;
		if  (target
	//	&&   target->hitpoints < units_dat::max_hitpoints[target->id] // Disabled to allow tasking repair on full HP targets
		&&   target->status & UnitStatus::Completed
		&& !(unit->unit_ai && target->status & UnitStatus::Reacts && target->order_target.unit != unit)
		&& !(target->status & UnitStatus::InTransport)
		&&  !target->stasis_timer)
		{
			switch (unit->main_order_state)
			{
				case 0:
				{
					s16 repair_info;
					u32 repair_cost_minerals, repair_cost_vespene;
					helper::get_repair_info(target, &repair_cost_minerals, &repair_cost_vespene, &repair_info);
					helper::verify_resources(repair_cost_minerals, repair_cost_vespene, unit->player);
					unit->worker.repair_resource_loss_timer = 0; // Now used as a toggle for the AlmostBuilt animation to avoid some visual bug, but can probably just use a local bool if necessary.
					unit->main_order_state++;
				} // Fallthrough intended
				case 1:
				{
					if (!(target->status & UnitStatus::Completed)) {
						unit->order_to(OrderId::ConstructingBuilding, target);
						if (*g_is_placing_building && !helper::can_still_place_building()) {
							helper::refresh_layers_3_and_4();
							helper::end_building_placement();
						}
					} else {
						if (helper::order_to_move_to_target(unit, target)) {
							helper::unit_order_move_to_target_unit(unit, target);
							unit->main_order_state = 6;
						}
					}
					return;
				} break;
				case 6:
				{
					if (!unit->get_movable_state()) { // not reached destination
						helper::move_towards(unit, target);
						return;
					}
					unit->main_order_state++;
				} break;
				case 7:
				{
					if (!unit->is_target_within_min_range(target, 5)) {
						unit->main_order_state = 1;
						return;
					}
					helper::set_secondary_waypoint(unit, target->sprite->position.x, target->sprite->position.y);
					helper::unit_order_move_to_target_unit(unit, target);
					if (!helper::is_unit_positions_2_equal(unit)) {
						u8 current_angle = unit->current_direction_1 - scbw::get_angle(unit->sprite->position.x, unit->sprite->position.y, unit->next_target_waypoint.x, unit->next_target_waypoint.y);
						if (current_angle > 128) {
							current_angle = 256 - current_angle;
						}
						if (current_angle > weapons_dat::attack_angle[unit->get_ground_weapon()]) {
							break; // goto LABEL_35
						}
					}
					unit->worker.repair_resource_loss_timer++;
					unit->main_order_state++;
				} break; // goto LABEL_35
				case 8:
				{
					helper::set_secondary_waypoint(unit, target->sprite->position.x, target->sprite->position.y);
					helper::unit_order_move_to_target_unit(unit, target);

					if (!unit->is_target_within_min_range(target, 5) || target->main_order_id == OrderId::Liftoff) {
						unit->sprite->play_iscript_animation(IscriptAnimation::GndAttkToIdle, true);
						unit->main_order_state = 1;
					}
					if (unit->worker.repair_resource_loss_timer) {
						unit->sprite->play_iscript_animation(IscriptAnimation::AlmostBuilt, true);
						unit->worker.repair_resource_loss_timer--;
					}
				} break;
				default:
				{
				} break; // goto LABEL_35
			}

			// LABEL_35:
			if (target->hitpoints < units_dat::max_hitpoints[target->id]) {
				if (target->new_repair_timer) {
					target->new_repair_timer--;
				} else {
					if (!helper::subtract_repair_resources(unit, target)) {
						unit->sprite->play_iscript_animation(IscriptAnimation::WalkingToIdle, true);
						unit->order_to_idle();
						return;
					}
				}
				u32 hp_gain = target->build_repair_hp_gain;
				if (unit->id == UnitId::TerranTalos) { // Talos - Increased repair speed
					hp_gain *= 1.5;
				}
				hp_gain *= 0.75; // Reduce overall healing rate
				target->heal_hp(hp_gain, unit);
				if (target->hitpoints < units_dat::max_hitpoints[target->id]) {
					return; // don't goto LABEL_43
				}
			}
		}

		// LABEL_43:
		unit->sprite->play_iscript_animation(IscriptAnimation::WalkingToIdle, true);

		if (unit->order_queue_head || !target) {
			helper::force_order_done(unit);
			/*unit->userActionFlags |= 1;
			prepareForNextOrder(unit);*/
		} else {
			unit->main_order_state = 1;
			/*u8 orderId;

			if (unit->pAI) {
			orderId = OrderId::ComputerAI;
			} else {
			orderId = units_dat::ReturnToIdleOrder[unit->id];
			}

			unit->orderComputerCL(orderId);*/
		}

		if (target && !(target->status & UnitStatus::InTransport))
		{
			target->connected_unit = 0;
		}
	}

	// Hooked to allow for general Repair and some other shit - Pr0nogo
	s32 is_order_valid(char order_id, Unit* unit, s32 unused, Unit* target, s32 bErrorFlag) // 00474D90
	{
		s32 unused_result = 0;
		u16 error_string_id = 0;
		switch (order_id)
		{
			case OrderId::Repair1:
			{
				error_string_id = 884;
				if (!target)
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = error_string_id;
					}
					return 0;
				}
				error_string_id = 883;
				if (!(target->status & UnitStatus::Completed))
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = error_string_id;
					}
					return 0;
				}

				if (target->id == UnitId::ZergNydusCesiant && target->hitpoints >= units_dat::max_hitpoints[target->id])
				{
					return 0;
				}
				
				//if (target->hitpoints >= units_dat::max_hitpoints[target->id])
				//{
				//	if (bErrorFlag)
				//	{
				//		*(s16*)bErrorFlag = error_string_id;
				//	}
				//	return 0;
				//}
				return 1;
			} break;
			case OrderId::Harvest1:
			{
				if (!target)
				{
					return 1;
				}
				error_string_id = unit->get_race() + 893;
				if (target->id == UnitId::ResourceVespeneGeyser)
				{
					//if (bErrorFlag)
					//{
					//	*(s16*)bErrorFlag = error_string_id;
					//}
					//return 0;

					return 1;
				}
				if (!helper::unit_is_resource(target))
				{
					return 0;
				}
				error_string_id = 892;
				if (!(units_dat::base_property[target->id] & UnitProperty::ResourceContainer))
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = error_string_id;
					}
					return 0;
				}
				error_string_id = 896;
				if (unit->player != target->player && unit_id_is_capped_gas_structure(target->id))
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = error_string_id;
					}
					return 0;
				}
				return 1;
			} break;
			case OrderId::Pickup2:
			{
				error_string_id = 890;
				if (!target || !scbw::is_allied_to(target->player, unit->player))
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = error_string_id;
					}
					return 0;
				}
				if (helper::can_enter_transport(unit, target))
				{
					return 1;
				}
				if (units_dat::space_provided[unit->id] >= units_dat::space_required[target->id])
				{
					error_string_id = 891;
				}
				if (bErrorFlag)
				{
					*(s16*)bErrorFlag = error_string_id;
				}
				return 0;
			} break;
			case OrderId::HealMove:
			{
				if (!target)
				{
					return 1;
				}
				if (!helper::tech_use_allowed(unit, TechId::Healing, unit->player))
				{
					return 0;
				}
				error_string_id = 1325;
				if (!(target->status & 6) // GroundedBuilding and InAir
					&& target->hitpoints < units_dat::max_hitpoints[target->id])
				{
					return 1;
				}
				if (bErrorFlag)
				{
					*(s16*)bErrorFlag = error_string_id;
				}
				return 0;
			} break;
			default:
			{
				return 1;
			}
		}
		return unused_result;
	}

	// Hooked to investigate Repair crashes. - Pr0nogo
	s32 order_allowed(Unit* unit, u16 order_id, s32 player) // 0046DC20
	{
		s32 result = 0;
		*g_last_internal_error = 0;
		if (order_id != OrderId::None)
		{
			if (orders_dat::req_offset[order_id] == -1)
			{
				helper::parse_orders_dat_requirements();
			}
			if (player == unit->player)
			{
				if (order_id == OrderId::RallyPointToVisibleUnit || order_id == OrderId::RallyPointToTile || unit->status & UnitStatus::Completed)
				{
					if (unit->is_frozen())
					{
						*g_last_internal_error = 10;
						result = 0;
					} else if (unit->status & UnitStatus::Burrowed && !unit_id_is_burrowed_attacker(unit->id) && order_id != OrderId::Unburrow) {
						*g_last_internal_error = 11;
						result = 0;
					} else {
						if ((unit->id == UnitId::TerranMason || unit->id == UnitId::TerranTalos) && unit->main_order_id == OrderId::ConstructingBuilding && order_id != 1
						|| (scbw::unit_can_nuke(unit) && unit->main_order_id == OrderId::NukeTrack)
						||  unit->id == UnitId::ProtossArchon && unit->main_order_id == OrderId::CompletingArchonSummon
						||  unit->id == UnitId::ProtossMindTyrant && unit->main_order_id == OrderId::CompletingArchonSummon)
						{
							*g_last_internal_error = 8;
							result = 0;
						} else {
							if (orders_dat::req_offset[order_id])
							{
								result = helper::parse_requirement_opcodes(unit, order_id, player, g_orders_requirement_tree, orders_dat::req_offset[order_id]);
							} else {
								*g_last_internal_error = 23;
							}
						}
					}
				} else {
					*g_last_internal_error = 20;
					result = 0;
				}
			} else {
				*g_last_internal_error = 1;
				result = 0;
			}
		} else {
			*g_last_internal_error = 15;
			result = 0;
		}
		return result;
	}

	// Hooked to remove right-click error message on harvesting from geysers. -Pr0nogo
	s32 show_right_click_error_if_needed(Unit* target) // 00455A00
	{
		s32 sfx_id = -1;
		s32 text_id = -1;
		Unit** selected_units = &g_client_selection_group->unit[11];
		Unit** selected_units_2 = &g_client_selection_group->unit[11];
		do
		{
			Unit* unit = *selected_units;
			if (*selected_units)
			{
				u8 action = RightClickActions::NoCommand_AutoAttack;
				u16 unit_id = unit->id;
				if (unit_id_is_burrowed_attacker(unit_id) && unit->status & UnitStatus::Burrowed)
				{
					action = RightClickActions::NoMove_NormalAttack;
				} else {
					char right_click_order = helper::get_right_click_action_order(unit);
					if (!right_click_order)
					{
						if (hooks::unit_can_rally(unit)) {
							right_click_order = RightClickActions::NormalMove_NoAttack;
						}
					}
					action = right_click_order;
				}
				if (unit->status & UnitStatus::IsHallucination)
				{
					if (action == RightClickActions::Harvest || action == RightClickActions::HarvestAndRepair)
					{
						action = RightClickActions::NormalMove_NormalAttack;
					}
				}

				switch (action)
				{
				case RightClickActions::NormalMove_NormalAttack:
					if (!target)
					{
						return 1;
					}
					if (!helper::is_unit_enemy(*g_active_portrait_unit, target))
					{
						return 1;
					}
					if (units_dat::base_property[target->id] & UnitProperty::NeutralAccessories)
					{
						return 1;
					}
					if (helper::unit_can_attack_target(target, unit, 1))
					{
						return 1;
					}
					text_id = 879;
					sfx_id = unit->get_race() + 138;
				case RightClickActions::NoMove_NormalAttack:
					if (!target)
					{
						break;
					}
					if (units_dat::base_property[target->id] & UnitProperty::NeutralAccessories)
					{
						break;
					}

					if (helper::unit_can_attack_target(target, unit, 1))
					{
						if (helper::is_unit_target_out_of_max_range(unit, target))
						{
							text_id = 888;
						}
						else
						{
							if (!helper::target_is_inside_minimum_range(unit, target))
							{
								return 1;
							}
							text_id = 889;
						}
					} else {
						text_id = 879;
					}
					sfx_id = unit->get_race() + 138;
				case RightClickActions::Harvest:
				case RightClickActions::HarvestAndRepair:
					if (!target)
					{
						return 1;
					}
					if (units_dat::base_property[target->id] & UnitProperty::NeutralAccessories)
					{
						return 1;
					}
					if (target->id == UnitId::ResourceVespeneGeyser)
					{
					//	sfx_id = unit->get_race() + 1;
					//	text_id = unit->get_race()  + 893;
					} else {
						if (!helper::is_unit_enemy(*g_active_portrait_unit, target))
						{
							return 1;
						}
						if (units_dat::base_property[target->id] & UnitProperty::NeutralAccessories)
						{
							return 1;
						}
						if (helper::unit_can_attack_target(target, unit, 1))
						{
							return 1;
						}
						text_id = 879;
						sfx_id = unit->get_race() + 138;
					}
					break;
				default:
					return 1;
				}
			}
			selected_units = selected_units_2 - 1;
			--selected_units_2;
		} while (selected_units_2 >= g_client_selection_group->unit);
		if (sfx_id != -1)
		{
			if (sfx_id)
			{
				if (g_options_list->sfx)
				{
					s32 sfx_volume = sfx_dat::volume[sfx_id];
					u32 max_volume = 99;
					if (sfx_volume >= 99)
					{
						if (sfx_volume >= 0)
						{
							if (sfx_volume >= 99)
							{
								max_volume = sfx_dat::volume[sfx_id];
							}
						} else {
							max_volume = 0;
						}
					}
					if (*use_quiet_sounds)
					{
						max_volume /= 5;
					}
					if (max_volume > 10)
					{
						helper::play_transmission_from_location(sfx_id, max_volume, 0, 0);
					}
				}
			}
		}
		if (text_id != -1)
		{
			u32 text_id_zeroed = text_id - 1;
			if (text_id)
			{
				char* text_string = 0;
				if (text_id_zeroed < g_stat_txt_buffer->get_string_count()) {
					text_string = (char*)g_stat_txt_buffer->get_string(text_id_zeroed + 1);
				}
				if (text_string) {
					if (*g_active_player_id == *g_local_human_id)
					{
						strncpy(draw_out_text_left, text_string, 0xDAu);
						text_bytes[12] = 4;
						*unit_stat_update_timer = GetTickCount() + 7000;
						helper::update_text_display(12);
					}
					return 0;
				}
			}
		}
		return 0;
	}
}