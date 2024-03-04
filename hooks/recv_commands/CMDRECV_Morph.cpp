#pragma once
#include "CMDRECV_Morph.h"
#include <BW/api.h>
#include "hooks/rally_point.h"

namespace hooks
{
	void cmdrecv_building_morph(u16 unit_id) { // 004C1910
		*g_unit_iterator_pos = 0;
		while (Unit* unit = helper::get_active_player_next_selection()) {
			if (unit->id != UnitId::None
			&&	unit->can_make_unit(unit_id, *g_active_nation_id) == 1
			&&	unit->main_order_id != OrderId::Morph2
			&&	helper::has_money_can_make(unit, unit_id))
			{
				// start construction if successful
				unit->order_computer(OrderId::Morph2);
				bw::refresh_console();
			}
		}
	}

	void cmdrecv_unit_morph(u16 unit_id) {
		*g_unit_iterator_pos = 0;
		while (Unit* current_unit = helper::get_active_player_next_selection()) {
			if (current_unit->can_make_unit(unit_id, *g_active_nation_id)
				&& current_unit->main_order_id != OrderId::Morph1
				&& helper::has_money_can_make(current_unit, unit_id)) { // start construction if successful
				current_unit->user_action_flags |= 1;
				if (current_unit->main_order_id != OrderId::Die) {
					while (current_unit->order_queue_tail) {
						if (orders_dat::can_be_interrupted[current_unit->order_queue_tail->order_id] || current_unit->order_queue_tail->order_id == OrderId::Morph1) {
							helper::remove_order_from_unit_queue(current_unit->order_queue_tail, current_unit);
						} else {
							break;
						}
					}
					current_unit->perform_another_order(OrderId::Morph1, 0, 0, nullptr, UnitId::None, 0);
				}
				prepare_for_next_order(current_unit);
			}
		}
		bw::refresh_console();
	}

	s32 compare_unit_train_priority(const void* a, const void* b) {
		Unit* unit_a = *(Unit**)a;
		Unit* unit_b = *(Unit**)b;

		u32 count_a = 0;
		u32 count_b = 0;
		for (u32 i = 0; i < 5; ++i) {
			if (unit_a->build_queue[i] != UnitId::None) {
				count_a++;
			}
			if (unit_b->build_queue[i] != UnitId::None) {
				count_b++;
			}
		}

		return count_a - count_b;
	}

	// Hooked for multi-unit select. -Pr0nogo
	void cmdrecv_train(char* buffer) { // 004C1C20
		u32 size = 0;
		Unit* selection[SelectionCap] = {};
		for (u32 i = 0; i < SelectionCap; ++i) {
			Unit* unit = players_selections->unit[*g_active_player_id][i];
			if (unit && unit->sprite) { // sprite is new
				selection[size++] = unit;
			}
		}
		qsort(selection, size, sizeof(Unit*), compare_unit_train_priority);

		u16 unit_id = *(u16*)(buffer + 1);
		for (u32 i = 0; i < size; ++i) {
			Unit* unit = selection[i];
			if (helper::create_unit_allowed(unit_id, unit, *g_active_nation_id) == 1) {
				if (helper::has_money_can_make(unit, unit_id)) {
					helper::set_secondary_order(unit, OrderId::Train);
				}
				bw::refresh_console();
			}
		}
	}

	// Hooked for multi-building select. -Pr0nogo
	void cmdrecv_liftoff(char* buffer) { // 004C1620
		*g_unit_iterator_pos = 0;
		while (Unit* unit = helper::get_active_player_next_selection()) {
			if (!helper::unit_is_training_or_morphing(unit) && unit->building.tech_id == TechId::None && unit->building.upgrade_id == UpgradeId::None) {
				if (unit->status & UnitStatus::GroundedBuilding) {
					if (units_dat::base_property[unit->id] & UnitProperty::FlyingBuilding) {
						*g_last_internal_error = 0;
						if (orders_dat::req_offset[OrderId::BuildingLiftoff] == 65535) {
							helper::parse_orders_dat_requirements();
						}
						if (unit->player != *g_active_nation_id) {
							*g_last_internal_error = 1;
							continue;
						}
						if (!(unit->status & UnitStatus::Completed)) {
							*g_last_internal_error = 23;
							continue;
						}
						if (unit->status & UnitStatus::Frozen || unit->lockdown_timer || unit->stasis_timer || unit->maelstrom_timer) {
							*g_last_internal_error = 10;
							continue;
						}
						if (unit->status & UnitStatus::Burrowed && unit->id != UnitId::TerranMinistry) {
							*g_last_internal_error = 11;
							continue;
						}
						if ((unit->id == UnitId::TerranMason || unit->id == UnitId::TerranTalos) && unit->main_order_id == OrderId::ConstructingBuilding
							|| bw::unit_can_nuke(unit) && unit->main_order_id == OrderId::NukeTrack
							|| unit->id == UnitId::ProtossAugur && unit->main_order_id == OrderId::CompletingArchonSummon)	{
							*g_last_internal_error = 8;
						} else if (orders_dat::req_offset[OrderId::BuildingLiftoff]) {
						//	if (helper::parse_requirement_opcodes(unit, OrderId::BuildingLiftoff, *g_active_nation_id, g_orders_requirement_tree, orders_dat::req_offset[OrderId::BuildingLiftoff]) == 1) {
								u16* buffer_u16 = (u16*)(buffer + 1);
								u16 x = buffer_u16[0];
								u16 y = buffer_u16[1];
								if (x < *g_map_width_pixels && y < *g_map_height_pixels) {
									UnitExt* unit_ext = get_unit_ext(unit);
									if (!unit_ext->timers[TimerLiftoffDelay]) {
										unit_ext->timers[TimerLiftoffDelay] = 12 + 1;
										helper::play_sound_from_unit(SoundId::TerranLiftoffStart, unit, 1, 0);
									}
									helper::issue_order_targeting_ground(unit, OrderId::BuildingLiftoff, x, y);
									helper::do_order(unit, units_dat::return_to_idle_order[unit->id], x, y - 42, 0);
								}
						//	}
						} else {
							*g_last_internal_error = 20;
						}
					}
				}
			}
		}
	}

	// Hooked for multi-building select. -Pr0nogo
	void btnsact_rally_point(u32 xd) { // 004244A0
		*g_order_id_ground = OrderId::RallyPointToTile;
		*g_order_id_obstructed = OrderId::RallyPointToTile;
		*g_is_targeting = 1;
		*g_order_id_unit = OrderId::RallyPointToVisibleUnit;

		const char* string = g_network_table_DF->get_string(63);
		if (g_network_table_DF->get_string_count() <= 62) {
			string = "";
		}

		helper::send_message_something(xd, string);
		helper::set_ingame_input_procs();
		*g_minimap_cursor = 2;
		*g_buttonset_special_buttonset_id = 229;
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
				if (bw::is_allied_to(unit->player, target_unit->player)) {
					if (target_unit->hitpoints < target_unit->get_max_hp_raw()) {
						if (helper::is_order_valid(OrderId::Repair1, unit, target_unit, target_unit, 0)) {
							return OrderId::Repair1;
						}
					}
					if (helper::unit_is_active_transport(target_unit) && helper::can_enter_transport(target_unit, unit)) {
						return OrderId::EnterTransport;
					}
					if (helper::is_order_valid(OrderId::Repair1, unit, target_unit, target_unit, 0)) {
						return OrderId::Repair1;
					}
				}
			} else if (target_unit->player == unit->player && target_unit->get_race() == RaceId::Terran
				&&   !(units_dat::base_property[target_unit->id] & UnitProperty::Addon)) {
				if (!target_unit->connected_unit || target_unit->connected_unit->order_target.unit != target_unit) {
					return OrderId::ConstructingBuilding;
				}
			}
			return helper::order_acquire_generic_idle_unit_proc(unit, x_pos, y_pos, target, fow_unit_id);
		}
		return result;
	}

	// Hooked for multi-building select. -Pr0nogo
	s32 btnscond_silo_has_no_nuke(u16 unit_id, u32 player, Unit* silo) { // 00428960
		for (u32 i = 0; i < *g_client_selection_count; ++i) {
			Unit* unit = g_client_selection_group->unit[i];
			if (unit && unit->id == UnitId::TerranAnvil && helper::create_unit_allowed(unit_id, unit, player)) {
				return true;
			}
		}
		return helper::create_unit_allowed(unit_id, silo, player);
	}
}