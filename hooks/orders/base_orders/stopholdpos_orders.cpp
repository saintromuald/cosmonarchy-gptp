#include "stopholdpos_orders.h"
#include <SCBW/api.h>

namespace hooks
{
	void orders_medic_hold_position(Unit* unit) { // 00464050
		if (unit->main_order_state == 0) {
			helper::set_resource_target(unit);
			if (unit->next_target_waypoint.x != unit->move_target.pt.x
			||  unit->next_target_waypoint.y != unit->move_target.pt.y)
			{
				unit->next_target_waypoint.x = unit->move_target.pt.x;
				unit->next_target_waypoint.y = unit->move_target.pt.y;
			}
			unit->status |= UnitStatus::AttackMove;
			if (unit->subunit) {
				unit->subunit->status |= UnitStatus::AttackMove;
			}
			unit->main_order_state = 1;
			unit->sprite->play_iscript_animation(IscriptAnimation::WalkingToIdle, true);
		}

		u32 medic_heal_state = helper::do_medic_heal(unit, unit->order_target.unit);
		if (medic_heal_state == 3) {
			unit->order_target.unit = 0;
		}

		if (medic_heal_state == 0 || medic_heal_state == 2 || medic_heal_state == 3) {
			unit->sprite->play_iscript_animation(IscriptAnimation::WalkingToIdle, true);
		}

		if (medic_heal_state != 1) {
			// If timer is 0, seek a target in distance
			if (unit->main_order_timer == 0) {
				unit->order_target.unit = helper::acquire_medic_heal_target(unit);
				if (unit->order_target.unit) {
					unit->order_target.pt.x = unit->order_target.unit->position.x;
					unit->order_target.pt.y = unit->order_target.unit->position.y;
				}
			}
		} else {
			if (!(unit->order_signal & 1)) {
				Image* current_image = unit->sprite->images.head;
				while (current_image) {
					current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
					current_image = current_image->link.next;
				}

				if (!unit->order_queue_head || unit->order_queue_head->order_id != OrderId::MedicHeal2) {
					helper::order_immediate(unit, OrderId::MedicHeal2); // return to idle after heal
				}
			}

			if (unit->order_target.unit->position.x != unit->next_target_waypoint.x
			||	unit->order_target.unit->position.y != unit->next_target_waypoint.y )
			{
				unit->next_target_waypoint.x = (unit->order_target.unit)->position.x;
				unit->next_target_waypoint.y = (unit->order_target.unit)->position.y;
			}
		}
	}

	void orders_reaver_stop(Unit* unit) {
		Unit* out_hangar_child = unit->unit_with_hangar.out_hangar_child;
		
		// Change move target destination to current position
		helper::set_resource_target(unit);

		if (unit->move_target.pt.x != unit->next_target_waypoint.x ||
			unit->move_target.pt.y != unit->next_target_waypoint.y) {
			unit->next_target_waypoint.x = unit->move_target.pt.x;
			unit->next_target_waypoint.y = unit->move_target.pt.y;
		}
		while (out_hangar_child) {
			out_hangar_child->user_action_flags |= 1;
			if (out_hangar_child->main_order_id != OrderId::Die) {
				bool not_removable_order = false;
				while (out_hangar_child->order_queue_tail && !not_removable_order) {

					if (!orders_dat::can_be_interrupted[out_hangar_child->order_queue_tail->order_id]
					&&	 out_hangar_child->order_queue_tail->order_id != OrderId::SelfDestructing)
					{
						not_removable_order = true;
					} else {
						helper::remove_order_from_unit_queue(out_hangar_child->order_queue_tail, out_hangar_child);
					}
				}
				out_hangar_child->perform_another_order(OrderId::SelfDestructing, 0, 0, nullptr, UnitId::None);
			}
			prepare_for_next_order(out_hangar_child);
			out_hangar_child = out_hangar_child->hangar_unit.hangar_link.prev;
		}

		unit->user_action_flags |= 1;
		Point16 pos = {};
		if (unit->order_target.unit) {
			pos.x = (unit->order_target.unit)->sprite->position.x;
			pos.y = (unit->order_target.unit)->sprite->position.y;
		}

		if (unit->main_order_id != OrderId::Die) {
			bool not_removeable_order = false;
			while (unit->order_queue_tail && !not_removeable_order) {
				if (!orders_dat::can_be_interrupted[unit->order_queue_tail->order_id]
				&&	 unit->order_queue_tail->order_id != OrderId::Reaver) {
					not_removeable_order = true;
				} else {
					helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
				}
			}
			unit->perform_another_order(OrderId::Reaver, pos.x, pos.y, unit->order_target.unit, UnitId::None);
		}
		prepare_for_next_order(unit);
	}

	void orders_guard(Unit* unit) {
	//	unit->main_order_timer = helper::rand_between(0, 15, 29);
		if (g_player_table[unit->player].type == PlayerType::Computer) {
			unit->main_order_id = OrderId::ComputerAI;
			if (!unit->unit_ai) {
				if (units_dat::base_property[unit->id] & UnitProperty::Hero) {
					unit->main_order_id = OrderId::PlayerGuard;
				}
			} else if (unit->unit_ai->type == AiType::Guard) {
				unit->main_order_id = OrderId::GuardPost;
			}
		} else {
			unit->main_order_id = OrderId::PlayerGuard;
		}
	}

	// NOT HOOKED (since most of the action happen in another function)
	// Just here to show that the true code is in function_004774A0, though
	// a few things happen before
	void orders_turret_guard_internal(Unit* unit) {
		if (unit->subunit->next_target_waypoint.x != unit->next_target_waypoint.x ||
			unit->subunit->next_target_waypoint.y != unit->next_target_waypoint.y) {
			unit->next_target_waypoint.x = unit->subunit->next_target_waypoint.x;
			unit->next_target_waypoint.y = unit->subunit->next_target_waypoint.y;
		}
		orders_player_guard(unit);
	}

	// Guard related function, may be a poorly identified official guard order
	void orders_player_guard(Unit* unit) {
		// If unit->auto_target_unit, attack this target
		// then disable the unit...I think
		// That's why if it return true, nothing else is done
		// since has already attacked
		if (!helper::attack_recent_attacker(unit)) {
			if (unit->main_order_timer == 0) {
			//	unit->main_order_timer = 15;
				if (units_dat::base_property[unit->id] & UnitProperty::Subunit) {
					if (unit->subunit->next_target_waypoint.x != unit->next_target_waypoint.x ||
						unit->subunit->next_target_waypoint.y != unit->next_target_waypoint.y) {
						unit->next_target_waypoint.x = unit->subunit->next_target_waypoint.x;
						unit->next_target_waypoint.y = unit->subunit->next_target_waypoint.y;
					}
				}

				if (unit->get_seek_range() != 0) {
					Unit* target = helper::find_best_attack_target(unit);
					if (target) {
						// attack target I think, also used by attack_recent_attacker
						helper::order_to_attack_target(unit, target, 1, 0);
					}
				}
			}
		}
	}
}