#include "medic_orders.h"
#include <BW/api.h>
#include "globals.h"
#include <math_utils.h>

namespace hooks
{
	void orders_medic_heal2(Unit* unit) { // 00463740
		if (!(unit->movement_flags & MovementFlags_Accelerating)) {
			Image* current_image = unit->sprite->images.head;
			while (current_image) {
				current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
				current_image = current_image->link.next;
			}
		}
		helper::force_order_done(unit);
	}

	void orders_heal_move(Unit* unit) { // 004637B0
		UnitExt* unit_ext = get_unit_ext(unit);
		if (!can_auto_heal(unit)) {
			return;
		}
		bool stupid_stop = false;
		Unit* target = unit->order_target.unit;
		Unit* old_target = unit->order_target.unit;

		if (target) {
			if (target->id == UnitId::ZergNydusCesiant && target->building.nydus_exit) {
				unit->order_to(OrderId::EnterNydusCanal, target);
				return;
			}

			if (helper::ai_order_heal(unit, target) != 0) {
				unit->order_computer(OrderId::MedicHeal1);
				unit->order_target.unit = target;
				unit->order_target.pt.x = target->position.x;
				unit->order_target.pt.y = target->position.y;
				helper::move_to_target(target, unit);
				stupid_stop = true;
			} else {
				unit->order_target.pt.x = target->position.x;
				unit->order_target.pt.y = target->position.y;
			}
		}
		if (!stupid_stop) {
			if (unit->recent_order_timer == 0) {
				if (!helper::set_move_target_xy(unit, unit->order_target.pt.x, unit->order_target.pt.y)) {
					stupid_stop = true;
				}
			}
			if (!stupid_stop) {
				if (unit->move_target.pt.x == unit->position.x && unit->move_target.pt.y == unit->position.y) {
					unit->order_to_idle();
				} else if (unit->main_order_timer == 0) {
					Point16 old_pos = {};
					unit->main_order_timer = 15;
					Unit* new_target = helper::acquire_medic_heal_target(unit);
					if (new_target) {
						old_pos.x = unit->order_target.pt.x;
						old_pos.y = unit->order_target.pt.y;
						helper::order_interrupt(unit, unit->order_target.unit, unit->order_target.pt.x, unit->order_target.pt.y, OrderId::MedicHeal1);

						helper::move_to_target(new_target, unit);
						helper::unit_order_move_to_target_unit(unit, new_target);
						if (old_target)
						{
							helper::update_target_something(unit, old_target, OrderId::HealMove);
						} else {
							helper::perform_another_order_at_coords(unit, old_pos.x, old_pos.y, OrderId::HealMove);
						}
					}
				}
			}
		}
	}

	// Idle/AI order probably -GPTP
	void orders_medic(Unit* unit) { // 00463900
		UnitExt* unit_ext = get_unit_ext(unit);
		if (!can_auto_heal(unit)) {
			return;
		}

		if (!unit->unit_ai && !helper::ai_cast_spell_behavior(unit, 0)) {
			Unit* target = helper::acquire_medic_heal_target(unit);
			if (target) {
				unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
				if (unit->main_order_id != OrderId::Die) {
					bool stupid_loop = !unit->order_queue_tail;
					while (!stupid_loop) {
						if (!orders_dat::can_be_interrupted[unit->order_queue_tail->order_id] && unit->order_queue_tail->order_id != OrderId::MedicHeal1) {
							stupid_loop = true;
						} else {
							helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
							stupid_loop = (!unit->order_queue_tail);
						}
					}
					unit->perform_another_order(OrderId::MedicHeal1, 0, 0, 0, UnitId::None);
				}

				prepare_for_next_order(unit);
				unit->order_target.unit = target;
				unit->order_target.pt.x = target->sprite->position.x;
				unit->order_target.pt.y = target->sprite->position.y;
				helper::move_to_target(target, unit);
			} else {
				if (unit->unit_ai) {
					if (*(u8*)((u32)unit->unit_ai + 8) != 1) {
						unit->order_computer(OrderId::ComputerAI);
					} else if (unit->sprite->position.x != *(u16*)((u32)unit->unit_ai + 0x12)
							|| unit->sprite->position.y != *(u16*)((u32)unit->unit_ai + 0x14)) {
						unit->order_to(OrderId::ComputerReturn, *(u16*)((u32)unit->unit_ai + 0x12), *(u16*)((u32)unit->unit_ai + 0x14));
					}
				}
			}
		}
	}

	void orders_medic_heal1(Unit* unit) { // 00464180
		Unit* target = unit->order_target.unit;
		u32 can_do_medic_heal = do_medic_heal(unit, target);
		if (!can_do_medic_heal) {
			helper::set_resource_target(unit);
			if (unit->order_queue_head) {
				unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
				prepare_for_next_order(unit);
			} else if (unit->unit_ai) {
				unit->order_computer(OrderId::ComputerAI);
			} else {
				unit->order_computer(units_dat::return_to_idle_order[unit->id]);
			}
		} else {
			if (unit->main_order_state == 0) {
				if (target && helper::move_towards(unit, target) && unit->get_movable_state() != MovableState_UnmovableAtDestination) {
					u32 range = Cleric::HealRange;
					auto& cleric_ext = unit->ext();
					if (cleric_ext.values[ValueTriggerHealRangeSet]) {
						range = cleric_ext.values[ValueTriggerHealRangeAmount];
					}
					if (unit->is_target_within_min_range(target, range)) {
						helper::set_resource_target(unit);
						unit->main_order_state = 1;
					}
				} else {
					unit->order_to_idle();
				}
			} else {
				if (can_do_medic_heal == 1) {
					if (!(unit->order_signal & 1)) {
						Image* current_image = unit->sprite->images.head;
						while (current_image) {
							current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
							current_image = current_image->link.next;
						}
						if (!unit->order_queue_head || unit->order_queue_head->order_id != OrderId::MedicHeal2) {
							helper::order_immediate(unit, OrderId::MedicHeal2);
						}
					}
					target = unit->order_target.unit;
					if (target && (target->position.x != unit->next_target_waypoint.x || target->position.y != unit->next_target_waypoint.y)) {
						unit->next_target_waypoint.x = target->position.x;
						unit->next_target_waypoint.y = target->position.y;
					}
				} else {
					unit->main_order_state = 0;
				}
			}
		}
	}

	bool ai_acquire_heal_target(Unit* cleric, Unit* target) { // 00463530
		if (cleric->is_frozen()) {
			return false;
		}

		if (!target || target == cleric || cleric->status & UnitStatus::IsHallucination) {
			return false;
		}
		UnitExt* target_ext = get_unit_ext(target);
		if (units_dat::base_property[target->id] & UnitProperty::Building
		||  target->status & UnitStatus::InAir
		|| !bw::is_allied_to(cleric->player, target->player)
		||  target->player >= PlayerLimit
		||  cleric->player >= PlayerLimit
		||  target->hitpoints >= units_dat::max_hitpoints[target->id]
		||  target->is_frozen()
		||  target_ext->timers[TimerReviveFlagClearMax])
		{
			return false;
		}
		return true;
	}

	// TODO: Figure out what these return values are used for, and make an enum. -Pr0nogo
	u32 do_medic_heal(Unit* cleric, Unit* target) { // 00463C40
		if (!can_auto_heal(cleric)) {
			return 0;
		}

		if (cleric->can_use_tech(TechId::Healing, cleric->player)) {
			if (target && helper::ai_order_heal(cleric, target)) {
				Unit* acquired = target;
				if (!acquired || acquired->is_being_healed) {
					acquired = helper::acquire_medic_heal_target(cleric);
					return 0;
				}
				helper::unit_order_move_to_target_unit(cleric, acquired);
				u32 range = Cleric::HealRange;
				auto& cleric_ext = cleric->ext();
				if (cleric_ext.values[ValueTriggerHealRangeSet]) {
					range = cleric_ext.values[ValueTriggerHealRangeAmount];
				}
				if (!cleric->is_target_within_min_range(acquired, range)) {
					if (!(cleric->status & UnitStatus::AttackMove)) {
						helper::move_to_target(acquired, cleric);
					}
					return 3;
				}
				bw::decloak_passive_proc(cleric);
				target->heal_hp(Cleric::HealAmount, cleric);

				// Nanoshield passive
				if (cleric->id == UnitId::TerranCleric) {
					auto& target_ext = target->ext();
					target_ext.timers[TimerClericHeal] = Cleric::NanoshieldTimer;
				}
				target->is_being_healed = 1;
				return 1;
			}
		}
		return 0;
	}

}