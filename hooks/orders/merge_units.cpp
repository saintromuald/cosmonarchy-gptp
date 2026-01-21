#include "merge_units.h"
#include <BW/api.h>
#include <algorithm>

#ifndef MAXINT32
#define MAXINT32    (0xFFFFFFFF >> 1)
#endif
#ifndef MAXINT16
#define MAXINT16    (0xFFFF >> 1)
#endif

namespace hooks {
	Unit* templar_merge_partner(Unit* unit, Unit** units_list, u32 units_list_length) { // 004BFA80
		Unit* nearest_unit = 0;
		u32 best_distance = MAXINT32;
		for (u32 i = 0; i < units_list_length; i++) {
			if (units_list[i]) {
				u32 x_distance = std::max(unit->sprite->position.x, units_list[i]->sprite->position.x) - std::min(unit->sprite->position.x, units_list[i]->sprite->position.x);
				x_distance *= x_distance;
				
				u32 y_distance = std::max(unit->sprite->position.y, units_list[i]->sprite->position.y) - std::min(unit->sprite->position.y, units_list[i]->sprite->position.y);
				y_distance *= y_distance;
				if ((x_distance + y_distance) < best_distance) {
					Unit* new_nearest_unit = units_list[i];
					units_list[i] = nearest_unit;
					nearest_unit = new_nearest_unit;
					best_distance = x_distance + y_distance;

				}
			}
		}
		return nearest_unit;
	}

	void orders_completing_archon_summon(Unit* unit) {
		if (unit->main_order_state == 0) {
			if (helper::advance_remaining_build_time(unit)) {
				unit->sprite->flags &= ~SpriteFlags::IscriptCode;
				if (unit->sprite->images.head) {
					Image* current_image = unit->sprite->images.head;
					do {
						current_image->play_iscript_animation(IscriptAnimation::Init);
						current_image = current_image->link.next;
					}
					while (current_image);
				}
				unit->main_order_state = 1;
			}
		} else if (unit->main_order_state == 1 && (unit->order_signal & 4)) {
				unit->order_signal -= 4;
				unit->status |= UnitStatus::Reacts;

				helper::delete_pathing(unit);
				helper::change_unit_buttonset(unit, unit->id);

				bw::refresh_console();
				helper::play_unit_ready_sound(unit);
				unit->order(units_dat::human_idle_order[unit->id], 0, 0, nullptr, UnitId::None, 0);
				helper::force_order_done(unit);
				if (unit->id == UnitId::ProtossAugur) {
					unit->status |= UnitStatus::Completed;
				}
			}

	}

	void orders_warping_mind_tyrant(Unit* unit) {
		Unit* target = unit->order_target.unit;
		if (!target || unit->player != target->player || target->main_order_id != OrderId::WarpingDarkArchon || target->order_target.unit != unit) {
			helper::set_resource_target(unit);
			if (unit->move_target.pt.x != unit->next_target_waypoint.x || unit->move_target.pt.y != unit->next_target_waypoint.y) {
				unit->next_target_waypoint.x = unit->move_target.pt.x;
				unit->next_target_waypoint.y = unit->move_target.pt.y;
			}
			if (unit->order_queue_head) {
				unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
				prepare_for_next_order(unit);
			} else {
				if (unit->unit_ai) {
					unit->order_computer(OrderId::ComputerAI);
				} else {
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				}
			}
		} else {
			if (unit->status & UnitStatus::IsNormal) {
				if (unit->is_target_within_min_range(target, (2 * unit->current_speed_2) / 256)) {
					unit->status &= ~UnitStatus::IsNormal;
					helper::order_immediate(unit, OrderId::ResetCollision1);
				}
			}
			u32 distance = bw::distance_between(unit->sprite->position.x, unit->sprite->position.y, target->sprite->position.x, target->sprite->position.y);
			if (distance > 20) {
				if (unit->main_order_state != 0 && unit->get_movable_state() == 2) {
					helper::make_to_hold_position(unit);
					unit->order_to_idle();
				} else {
					unit->main_order_state = 1;
					if (distance > 19) {
						helper::delayed_set_move_target_xy(unit, target->sprite->position.x, target->sprite->position.y);
					} else {
						bw::set_merge_move_target(unit, target);
					}
				}
			} else {
				helper::merge_status_into_unit(unit, target);
				helper::replace_unit_with_id(unit, UnitId::ProtossMindTyrant);
				helper::end_cloak(unit, SoundId::Terran_PHOENIX_TPhClo01_WAV);
				unit->sprite->flags &= ~SpriteFlags::IscriptCode;
				if (unit->sprite->images.head) {
					Image* current_image = unit->sprite->images.head;
					do {
						current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
						current_image = current_image->link.next;
					}
					while (current_image);
				}
				helper::change_unit_buttonset(unit, UnitId::None);
				bw::refresh_console();

				unit->kill_count += target->kill_count;
				unit->energy = unit->get_max_energy();
				target->silent_remove();
				unit->status &= ~UnitStatus::Reacts;
				helper::delete_pathing(unit);
				unit->order_computer(OrderId::CompletingArchonSummon);
			}
		}
	}

	void orders_warping_archon(Unit* unit) {
		Unit* target = unit->order_target.unit;
		if (!target || unit->player != target->player || target->main_order_id != OrderId::WarpingArchon || target->order_target.unit != unit) {
			helper::set_resource_target(unit);
			if (unit->move_target.pt.x != unit->next_target_waypoint.x || unit->move_target.pt.y != unit->next_target_waypoint.y) {
				unit->next_target_waypoint.x = unit->move_target.pt.x;
				unit->next_target_waypoint.y = unit->move_target.pt.y;
			}
			if (unit->order_queue_head) {
				unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
				prepare_for_next_order(unit);
			} else {
				if (unit->unit_ai) {
					unit->order_computer(OrderId::ComputerAI);
				} else {
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				}
			}
		} else {
			if (unit->status & UnitStatus::IsNormal) {
				if (unit->is_target_within_min_range(target, (2 * unit->current_speed_2) / 256)) {
					unit->status &= ~UnitStatus::IsNormal;
					helper::order_immediate(unit, OrderId::ResetCollision1);
				}
			}
			u32 distance = bw::distance_between(unit->sprite->position.x, unit->sprite->position.y, target->sprite->position.x, target->sprite->position.y);
			if (distance > 2) {
				if (unit->main_order_state != 0 && unit->get_movable_state() == 2) {
					helper::make_to_hold_position(unit);
					unit->order_to_idle();
				} else {
					unit->main_order_state = 1;
					if (distance > 10) {
						helper::delayed_set_move_target_xy(unit, target->sprite->position.x, target->sprite->position.y);
					} else {
						bw::set_merge_move_target(unit, target);
					}
				}
			} else {
				helper::merge_status_into_unit(unit, target);
				helper::replace_unit_with_id(unit, UnitId::ProtossArchon);

				unit->sprite->flags &= ~SpriteFlags::IscriptCode;
				if (unit->sprite->images.head) {
					Image* current_image = unit->sprite->images.head;
					do {
						current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
						current_image = current_image->link.next;
					}
					while (current_image);
				}
				helper::change_unit_buttonset(unit, UnitId::None);
				bw::refresh_console();

				unit->kill_count += target->kill_count;
				target->silent_remove();
				unit->status &= ~UnitStatus::Reacts;
				helper::delete_pathing(unit);
				unit->order_computer(OrderId::CompletingArchonSummon);
			}
		}
	}

}