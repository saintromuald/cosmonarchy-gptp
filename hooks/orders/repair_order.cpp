#include "repair_order.h"
#include <SCBW/api.h>
#include <hooks/rally_point.h>

namespace hooks
{
	void orders_repair_1(Unit* unit) { // 004673D0
		Unit* target = unit->order_target.unit;
		if (!target
	//	||	target->hitpoints >= units_dat::MaxHitPoints[target->id] // Disabled to allow tasking repair on full HP targets
		||	target->stasis_timer != 0
		||	(unit->unit_ai && target->status & UnitStatus::IsBuilding && target->order_target.unit != unit)
		||	target->status & UnitStatus::InTransport
		||  !(target->status & UnitStatus::Completed)) {
			unit->sprite->play_iscript_animation(IscriptAnimation::WalkingToIdle, true);
			unit->order_to_idle();
			if (target && !(target->status & UnitStatus::InTransport)) {
				target->connected_unit = 0;
			}
			return;
		}

		switch (unit->main_order_state) {
			case 0: {
				s16 unkRepairInfo = 0;
				u32 repairCostMinerals = 0, repairCostGas = 0;

				helper::getRepairInfo(target, &repairCostMinerals, &repairCostGas, &unkRepairInfo);
				helper::verifyResources(repairCostMinerals, repairCostGas, unit->player);

				unit->worker.repair_resource_loss_timer = 0;
				unit->main_order_state = 1; // continue directly into this case
			} // fallthrough
			case 1: {
				if (!(target->status & UnitStatus::Completed)) {
					unit->order_to(OrderId::ConstructingBuilding, target);
					if (*is_placing_building) {
						if (!helper::can_still_place_building()) {
							helper::refresh_layers_3_and_4();
							helper::end_building_placement();
						}
					}
					return;
				}

				if (helper::order_to_move_to_target(unit, target)) {
					helper::unitOrderMoveToTargetUnit(unit, target);
					unit->main_order_state = 6;
				}
				return;
			} break;
			case 6: {
				if (!unit->get_movable_state()) { // not reached destination
					helper::move_towards(unit, target); // may cause unit to move or become idle
					return;
				}
				if (!(unit->is_target_within_min_range(target, 5))) {
					unit->main_order_state = 1;
					return;
				}

				helper::set_secondary_waypoint(unit, target->sprite->position.x, target->sprite->position.y);
				helper::unitOrderMoveToTargetUnit(unit, target);
				unit->main_order_state = 7;
			} break;
			case 7: {
				if (!(unit->is_target_within_min_range(target, 5))) {
					unit->main_order_state = 1;
					return;
				} else {
					helper::set_secondary_waypoint(unit, target->sprite->position.x, target->sprite->position.y);
					helper::unitOrderMoveToTargetUnit(unit, target);
					if (!helper::is_unit_positions_2_equal(unit)) {
						s32 angle_delta = scbw::get_angle(unit->sprite->position.x, unit->sprite->position.y, unit->next_target_waypoint.x, unit->next_target_waypoint.y);
						s32 current_angle = unit->current_direction_1 - angle_delta;
						if (current_angle < 0) {
							current_angle += 256;
						}
						if (current_angle > 128) {
							current_angle = 256 - current_angle;
						}
						u8 weapon = unit->get_ground_weapon();
						if (current_angle <= weapons_dat::AttackAngle[weapon]) {
							unit->play_iscript_animation(IscriptAnimation::AlmostBuilt);
							unit->main_order_state = 8;
						}
					} else {
						unit->play_iscript_animation(IscriptAnimation::AlmostBuilt);
						unit->main_order_state = 8;
					}
				}
			} break;
			case 8: {
				helper::set_secondary_waypoint(unit, target->sprite->position.x, target->sprite->position.y);
				helper::unitOrderMoveToTargetUnit(unit, target);
				if (!unit->is_target_within_min_range(target, 5) || target->main_order_id == OrderId::Liftoff) { 
					unit->play_iscript_animation(IscriptAnimation::GndAttkToIdle);
					unit->main_order_state = 1;
				}
			} break;
		}

		bool do_repair = false;
		//UnitExt* target_ext = globals.get_unit_ext(target);
		if (target->hitpoints < units_dat::MaxHitPoints[target->id]) {
			if (unit->worker.repair_resource_loss_timer == 0) { // target_ext->values[ValueNewRepairTimer] == 0
				unit->worker.repair_resource_loss_timer--; // target_ext->values[ValueNewRepairTimer] == units_dat::TimeCost[target->id] / 2; 
				do_repair = helper::subtract_repair_resources(unit, target); // This function calls get_repair_info which has some dumb handling that we might need.
				if (!do_repair)
				{
					unit->play_iscript_animation(IscriptAnimation::WalkingToIdle);
					if (unit->order_queue_head) {
						helper::force_order_done(unit);
					} else {
						unit->main_order_state = 1;
					}
					return;
				}
			} else {
				unit->worker.repair_resource_loss_timer--; // target_ext->values[ValueNewRepairTimer] -= 1;
				do_repair = true;
			}
		}
		if (do_repair) {
			u32 hpGain = target->build_repair_hp_gain;
			if (*bw_cheat_state & CheatFlags::OperationCwal) {
				hpGain *= 16;
			}
			// Talos - Advanced repair speed
			if (unit->id == UnitId::TerranTalos) {
				hpGain *= 1.5;
			}
			target->heal(hpGain, unit);

			if (target->hitpoints < units_dat::MaxHitPoints[target->id]) {
				return;
			}
		}

		unit->sprite->play_iscript_animation(IscriptAnimation::WalkingToIdle, true);
		if (unit->order_queue_head) {
			helper::force_order_done(unit);
		} else {
			unit->main_order_state = 1;
		}
		if (target && !(target->status & UnitStatus::InTransport)) {
			target->connected_unit = 0;
		}
	}

	// Hooked to allow for general Repair and some other shit - Pr0nogo
	s32 is_order_valid(char orderId, Unit* unit, s32 unused, Unit* target, s32 bErrorFlag) // 00474D90
	{
		s32 unusedResult = 0;
		u16 errorStringId = 0;
		switch (orderId)
		{
			case OrderId::Repair1:
			{
				errorStringId = 884;
				if (!target)
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = errorStringId;
					}
					return 0;
				}
				errorStringId = 883;
				if (!(target->status & UnitStatus::Completed))
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = errorStringId;
					}
					return 0;
				}
				//if (target->hitpoints >= units_dat::MaxHitPoints[target->id])
				//{
				//	if (bErrorFlag)
				//	{
				//		*(s16*)bErrorFlag = errorStringId;
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
				errorStringId = unit->get_race() + 893;
				if (target->id == UnitId::ResourceVespeneGeyser)
				{
					//if (bErrorFlag)
					//{
					//	*(s16*)bErrorFlag = errorStringId;
					//}
					//return 0;

					return 1;
				}
				if (!helper::unit_is_resource(target))
				{
					return 0;
				}
				errorStringId = 892;
				if (!(units_dat::BaseProperty[target->id] & UnitProperty::ResourceContainer))
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = errorStringId;
					}
					return 0;
				}
				errorStringId = 896;
				if (unit->player != target->player && scbw::unit_id_is_capped_gas_structure(target->id))
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = errorStringId;
					}
					return 0;
				}
				return 1;
			} break;
			case OrderId::Pickup2:
			{
				errorStringId = 890;
				if (!target || unit->player != target->player)
				{
					if (bErrorFlag)
					{
						*(s16*)bErrorFlag = errorStringId;
					}
					return 0;
				}
				if (helper::can_enter_transport(unit, target))
				{
					return 1;
				}
				if (units_dat::SpaceProvided[unit->id] >= units_dat::SpaceRequired[target->id])
				{
					errorStringId = 891;
				}
				if (bErrorFlag)
				{
					*(s16*)bErrorFlag = errorStringId;
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
				errorStringId = 1325;
				if (!(target->status & 6) // GroundedBuilding and InAir
					&& target->hitpoints < units_dat::MaxHitPoints[target->id])
				{
					return 1;
				}
				if (bErrorFlag)
				{
					*(s16*)bErrorFlag = errorStringId;
				}
				return 0;
			} break;
			default:
			{
				return 1;
			}
		}
		return unusedResult;
	}

	// Hooked to investigate Repair crashes - Pr0nogo
	s32 order_allowed(Unit* unit, u16 orderId, s32 player) // 0046DC20
	{
		s32 result = 0;
		*last_internal_error = 0;
		if (orderId < OrderId::None)
		{
			if (orders_dat::RequirementsOffset[orderId] == -1)
			{
				helper::parse_orders_dat_requirements();
			}
			if (player == unit->player)
			{
				if (orderId == OrderId::RallyPoint1 || orderId == OrderId::RallyPoint2 || unit->status & UnitStatus::Completed)
				{
					if (unit->is_frozen())
					{
						*last_internal_error = 10;
						result = 0;
					}
					else if (unit->status & UnitStatus::Burrowed && unit->id != UnitId::ZergLakizilisk && orderId != OrderId::Unburrow)
					{
						*last_internal_error = 11;
						result = 0;
					}
					else
					{
						if (unit->id == UnitId::TerranMason && unit->main_order_id == OrderId::ConstructingBuilding && orderId != 1
						|| (scbw::unit_can_nuke(unit) && unit->main_order_id == OrderId::NukeTrack)
						||  unit->id == UnitId::ProtossArchon && unit->main_order_id == OrderId::CompletingArchonSummon
						||  unit->id == UnitId::ProtossMindTyrant && unit->main_order_id == OrderId::CompletingArchonSummon)
						{
							*last_internal_error = 8;
							result = 0;
						}
						else
						{
							if (orders_dat::RequirementsOffset[orderId])
							{
								result = helper::parse_requirement_opcodes(unit,
									orderId,
									player,
									orders_dat::RequirementsOffset[orderId],
									(s32)orders_requirement_tree);
							}
							else
							{
								*last_internal_error = 23;
							}
						}
					}
				}
				else
				{
					*last_internal_error = 20;
					result = 0;
				}
			}
			else
			{
				*last_internal_error = 1;
				result = 0;
			}
		}
		else
		{
			*last_internal_error = 15;
			result = 0;
		}
		return result;
	}

	// Hooked to remove right-click error message on harvesting from geysers. -Pr0nogo
	s32 show_right_click_error_if_needed(Unit* target) // 00455A00
	{
		s32 sfx_id = -1;
		s32 text_id = -1;
		Unit** selected_units = &client_selection_group->unit[11];
		Unit** selected_units_2 = &client_selection_group->unit[11];
		do
		{
			Unit* unit = *selected_units;
			if (*selected_units)
			{
				u8 action = RightClickActions::NoCommand_AutoAttack;
				u16 unit_id = unit->id;
				if (unit_id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed)
				{
					action = RightClickActions::NoMove_NormalAttack;
				}
				else
				{
					char right_click_order = units_dat::RightClickAction[unit_id];
					if (!right_click_order)
					{
						if (unit->status & UnitStatus::GroundedBuilding)
						{
							if (hooks::unit_can_have_rally(unit))
							{
								right_click_order = RightClickActions::NormalMove_NoAttack;
							}
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
					if (!helper::is_unit_enemy(*active_portrait_unit, target))
					{
						return 1;
					}
					if (units_dat::BaseProperty[target->id] & UnitProperty::NeutralAccessories)
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
					if (units_dat::BaseProperty[target->id] & UnitProperty::NeutralAccessories)
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
							if (!helper::is_unit_target_inside_min_range(unit, target))
							{
								return 1;
							}
							text_id = 889;
						}
					}
					else
					{
						text_id = 879;
					}
					sfx_id = unit->get_race() + 138;
				case RightClickActions::Harvest:
				case RightClickActions::HarvestAndRepair:
					if (!target)
					{
						return 1;
					}
					if (units_dat::BaseProperty[target->id] & UnitProperty::NeutralAccessories)
					{
						return 1;
					}
					if (target->id == UnitId::ResourceVespeneGeyser)
					{
						//sfx_id = unit->get_race() + 1;
						//text_id = unit->get_race()  + 893;
					}
					else
					{
						if (!helper::is_unit_enemy(*active_portrait_unit, target))
						{
							return 1;
						}
						if (units_dat::BaseProperty[target->id] & UnitProperty::NeutralAccessories)
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
		} while (selected_units_2 >= client_selection_group->unit);
		if (sfx_id != -1)
		{
			if (sfx_id)
			{
				if (options_list->sfx)
				{
					s32 sfx_volume = sfx_dat::Volume[sfx_id];
					u32 max_volume = 99;
					if (sfx_volume >= 99)
					{
						if (sfx_volume >= 0)
						{
							if (sfx_volume >= 99)
							{
								max_volume = sfx_dat::Volume[sfx_id];
							}
						}
						else
						{
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
				if (text_id_zeroed < *statTxtTbl) {
					text_string = (char*)statTxtTbl + statTxtTbl[text_id_zeroed + 1];
				}
				if (text_string) {
					if (*ACTIVE_PLAYER_ID == *LOCAL_HUMAN_ID)
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