#pragma once
#include "CMDRECV_Morph.h"
#include <SCBW/api.h>
#include "hooks/rally_point.h"

namespace hooks
{
	void cmdrecv_building_morph(u16 unit_id) {
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
		}
	}

	void cmdrecv_unit_morph(u16 unit_id) {
		*unit_iterator_pos = 0;
		while (Unit* current_unit = helper::get_active_player_next_selection()) {
			if (current_unit->can_make_unit(unit_id, *ACTIVE_NATION_ID)
				&& current_unit->main_order_id != OrderId::Morph1
				&& helper::has_money_can_make(current_unit, unit_id)) { // start construction if successful
				current_unit->user_action_flags |= 1;
				if (current_unit->main_order_id != OrderId::Die) {
					while (current_unit->order_queue_tail) {
						if (orders_dat::CanBeInterrupted[current_unit->order_queue_tail->order_id] || current_unit->order_queue_tail->order_id == OrderId::Morph1) {
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
		scbw::refresh_console();
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
			Unit* unit = players_selections->unit[*ACTIVE_PLAYER_ID][i];
			if (unit && !unit->is_dead()) {
				selection[size++] = unit;
			}
		}
		qsort(selection, size, sizeof(Unit*), compare_unit_train_priority);

		u8 unit_id = buffer[1];
		for (u32 i = 0; i < size; ++i) {
			Unit* unit = selection[i];
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
							if (helper::parse_requirement_opcodes(unit, OrderId::BuildingLiftoff, *ACTIVE_NATION_ID, orders_requirement_tree, orders_dat::RequirementsOffset[OrderId::BuildingLiftoff]) == 1) {
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
	void btnsact_rally_point(u32 xd) { // 004244A0
		*order_id_ground = OrderId::RallyPointToTile;
		*order_id_obstructed = OrderId::RallyPointToTile;
		bool network_table_1 = *network_table < 0x3Eu;
		bool network_table_2 = *network_table == 62;
		*is_targeting = 1;
		*order_id_unit = OrderId::RallyPointToVisibleUnit;

		const char* string = (const char*)(*network_table + network_table[63]);
		if (network_table_1 || network_table_2) {
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
}