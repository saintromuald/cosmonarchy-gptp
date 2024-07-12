#include "load_unload_orders.h"
#include "hooks\load_unload_proc.h"
#include <BW/api.h>
#include <logger.h>
#include "globals.h"
#include "hooks\limits\ccmu.h"
#define WRITE_TO_LOG(x)gptp::g_logger<<x<<std::endl

namespace hooks {
// Unit is a transport
// This is the initial pickup order
void orders_pickup_1(Unit* unit) {
    Unit* unit_to_load = unit->order_target.unit;
    if (unit_to_load && bw::can_be_entered_by(unit, unit_to_load)) {
        helper::prepend_order_targeting_unit(unit, unit_to_load, OrderId::Pickup2);
        prepare_for_next_order(unit);
    } else if (unit->unit_ai) {
        Point16 pos = {0,0};
        unit->user_action_flags |= 1;
        if (unit->main_order_id != OrderId::Die) {
            bool bStopLoop = (unit->order_queue_tail == 0);
            while (!bStopLoop) {
                u8 order_id = (u8)unit->order_queue_tail->order_id;
                if (orders_dat::can_be_interrupted[order_id] == 0
                    && order_id != OrderId::ComputerAI) {
                    bStopLoop = true;
                }

                if (!bStopLoop) {
                    helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
                    bStopLoop = (unit->order_queue_tail == 0);
                }
            }
            unit->perform_another_order(OrderId::ComputerAI, 0, 0, 0, UnitId::None);
        }
        prepare_for_next_order(unit);
    }
}

// Unit is a non-mobile transport (bunker)
void orders_pickup_3(Unit* unit) {
    Unit* unit_to_load = unit->order_target.unit;
    if (!unit_to_load) {
        if (unit->order_queue_head) {
            unit->user_action_flags |= 1;
            prepare_for_next_order(unit);
        } else {
            if (unit->unit_ai) {
                unit->order_computer(OrderId::ComputerAI);
            } else {
                unit->order_computer(units_dat::return_to_idle_order[unit->id]);
            }
        }
    } else {
        if (bw::can_be_entered_by(unit, unit_to_load)) {
            unit_to_load->order_to(OrderId::EnterTransport, unit);
        }
        unit->order_to_idle();
    }
}

// Unit is a transport
// This is used after orders_pickup_1
void orders_pickup_2(Unit* unit) {
    Unit* unit_to_load = unit->order_target.unit;
    if (!unit_to_load) {
        if (unit->order_queue_head) {
            unit->user_action_flags |= 1;
            prepare_for_next_order(unit);
        } else {
            if (unit->unit_ai) {
                unit->order_computer(OrderId::ComputerAI);
            } else {
                unit->order_computer(units_dat::return_to_idle_order[unit->id]);
            }
        }
    } else {
        if (!unit->unit_ai || !unit_to_load->unit_ai || unit_to_load->main_order_id == OrderId::TransportLoad) {
            if (unit->main_order_state == 0) {
                if (bw::can_be_entered_by(unit, unit_to_load)) {
                    if (unit->main_order_timer == 0) {
                        unit->main_order_timer = 7;
                        if (!unit->is_target_within_min_range(unit_to_load, 1)) {
                            helper::order_to_move_to_target(unit, unit_to_load);
                        } else {
                            helper::make_to_hold_position(unit);
                            if (unit_to_load->main_order_id != OrderId::EnterTransport && unit_to_load->main_order_id != OrderId::TransportLoad) {
                                unit_to_load->order_to(OrderId::EnterTransport, unit);
                            }

                            if (unit->unit_ai) {
                                unit->main_order_state = 1;
                            } else {
                                helper::prepend_order_targeting_unit(unit, unit_to_load, OrderId::Pickup1);
                                prepare_for_next_order(unit);
                            }
                        }
                    }
                } else {
                    unit->order_to_idle();
                }
            }
        } else {
            unit->order_to_idle();
        }
    }
}

// Unit is a transport
// A big part of the code could not be tested since different inside height test never occured during testing
void orders_move_unload(Unit* unit) {
    u32 distance;
    if (unit->main_order_state == 0 &&
        unit->order_target.unit) {
        unit->main_order_state = 1;
        unit->order_target.pt.x = (unit->order_target.unit)->sprite->position.x;
        unit->order_target.pt.y = (unit->order_target.unit)->sprite->position.y;
        unit->order_target.unit = 0;
    }

    distance = bw::distance_between(unit->order_target.pt.x * 256, unit->order_target.pt.y * 256, unit->halt.x, unit->halt.y);
    distance &= 0xFFFFFF00;
    if (distance <= 0x1000) {
        if (unit->order_queue_head) {
            helper::insert_order_before(unit, unit->order_queue_head, OrderId::Unload, 0, 0, 0, UnitId::None);
            prepare_for_next_order(unit);
        } else {
            unit->perform_another_order(OrderId::Unload, 0, 0, 0, UnitId::None);
            prepare_for_next_order(unit);
        }
    } else {
        helper::set_move_target_xy(unit, unit->order_target.pt.x, unit->order_target.pt.y);
        if (unit->order_target.pt.x != unit->next_target_waypoint.x || unit->order_target.pt.y != unit->next_target_waypoint.y) {
            unit->next_target_waypoint.x = unit->order_target.pt.x;
            unit->next_target_waypoint.y = unit->order_target.pt.y;
        }
        if (unit->unit_ai) {
            u32 ground_height_at_pos_1 = bw::get_ground_height_at_pos(unit->sprite->position.x, unit->sprite->position.y);
            if (ground_height_at_pos_1 != bw::get_ground_height_at_pos(unit->order_target.pt.x, unit->order_target.pt.y)) {
                distance = bw::distance_between(unit->order_target.pt.x * 256, unit->order_target.pt.y * 256, unit->halt.x, unit->halt.y);
                distance &= 0xFFFFFF00;
                if (distance <= 0xCA000
                    && helper::ai_are_on_connected_regions(unit, unit->order_target.pt.x, unit->order_target.pt.y) != 0
                    && helper::get_first_loaded_worker(unit) == 0) {
                    s32 index = 0;
                    bool bStopLoop = false;
                    bool bStopAll = false;
                    bool bIsDistanceGreaterThanHaltDistance = helper::is_distance_greater_than_halt_distance(unit, unit->order_target.pt.x, unit->order_target.pt.y, 512);
                    Point16 pos = {0,0}; // unsure about this start value
                    while (!bStopLoop) {
                        Unit* unit_to_unload = helper::get_loaded_unit_from_index(unit, index);
                        if (unit_to_unload) {
                            if (unit_to_unload->id == UnitId::ProtossAccantorOld && bIsDistanceGreaterThanHaltDistance) {
                                bStopLoop = true;
                            } else if (helper::can_unload(unit, unit_to_unload, &pos) == 0) {
                                bStopAll = true;
                                bStopLoop = true;
                            }
                        }
                        if (!bStopAll) {
                            index++;
                            bStopLoop = (index < TransportCap);
                        }
                    }
                    if (!bStopAll) {
                        helper::order_immediate(unit, OrderId::Unload);
                        prepare_for_next_order(unit);
                    }
                }
            }
        }
    }
}

// Unit is a unit to be picked up by transport
// Breaks AI transport usage! -Pr0nogo
void orders_pickup_4(Unit* unit) {
    bool is_neutral = unit->player == 11;
    char unit_status = unit->status;
    if (is_neutral) {
        u8 idle_order;
        if (unit_status & UnitStatus::InTransport) {
            if (units_dat::base_property[UnitProperty::Building] && unit_is_combatant(unit)) {
                idle_order = OrderId::TurretGuard;
            } else {
                idle_order = OrderId::Nothing2;
            }
        } else {
            idle_order = units_dat::return_to_idle_order[unit->id];
        }
        unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
        helper::order(1, unit, idle_order, {}, 0, 228);
        helper::do_next_queued_order(unit);
        return;
    }
    if (!(unit_status & UnitStatus::InTransport) && !helper::ai_update_attack_target(unit, 0, 0, 1)) {
        Unit* order_target = unit->order_target.unit;
        Unit** order_target_offset = &unit->order_target.unit;
        if (order_target) {
            if (!helper::unit_is_active_transport(order_target) || order_target->order_target.unit != unit) {
                goto LABEL_16;
            }
        } else {
            if (!helper::ai_find_transport(unit, &unit->order_target.unit)) {
                helper::to_idle(unit);
                return;
            }
            order_target = *order_target_offset;
            if (!*order_target_offset) {
                return;
            }
        }
        if (!can_enter_transport(order_target, unit)) {
        LABEL_16:
            *order_target_offset = 0;
            return;
        }
        if (helper::is_target_within_min_range(unit, 1u, order_target)) {
            helper::load_unit_proc(order_target, unit);
            helper::issue_order_targeting_ground(order_target, OrderId::MoveUnload, unit->order_target.pt.x, unit->order_target.pt.y);
            if (order_target->unit_ai) {
                helper::do_order(order_target, OrderId::Move, order_target->sprite->position.x, order_target->sprite->position.y, 0);
            }
        } else {
            helper::prepend_order_targeting_unit(unit, order_target, 0x5Eu);
            helper::do_next_queued_order(order_target);
            if (helper::ai_are_on_connected_regions(unit, order_target->sprite->position.x, order_target->sprite->position.y)) {
                helper::set_secondary_waypoint(unit, order_target->sprite->position.x, order_target->sprite->position.y);
                helper::move_towards(order_target, unit);
            }
        }
    }

    /*
    if (unit->player == 11)
    {
        unit->user_action_flags |= 1;
        if (unit->status & UnitStatus::InTransport)
        {
            unit->order(OrderId::Nothing2, 0, 0, nullptr, UnitId::None, 1);
        } else {
            unit->order(units_dat::return_to_idle_order[unit->id], 0, 0, nullptr, UnitId::None, 1);
        }
        prepare_for_next_order(unit);
    } else if (!(unit->status & UnitStatus::InTransport) && helper::ai_update_attack_target(unit, 1, 0, 0) == 0) {
        Unit* target = unit->order_target.unit;
        bool jump_to_E7C2B = false;
        bool jump_to_E7C37 = false;
        bool jump_to_E7C44 = false;
        bool bEndThere = false;
        if (target == nullptr)
        {
            if (helper::ai_find_transport(unit, &target) == 0)
            {
                unit->order_to_idle();
                bEndThere = true;
            }
            else
            if (target != nullptr)
            {
                jump_to_E7C2B = true;
            }
        }
        if (!bEndThere)
        {
            if (!jump_to_E7C2B)
            { // 004E7C1B
                if (target != nullptr
                    && (!helper::unit_is_active_transport(target) || target->order_target.unit != unit))
                {
                    jump_to_E7C37 = true;
                }
                else
                {
                    jump_to_E7C2B = true;
                }
            }
            if (jump_to_E7C2B)
            {
                if (bw::can_be_entered_by(target, unit))
                {
                    jump_to_E7C44 = true;
                }
                else
                {
                    jump_to_E7C37 = true;
                }
            }
            if (jump_to_E7C37)
            {
                unit->order_target.unit = nullptr;
            }
            if (jump_to_E7C44)
            {
                if (target != nullptr)
                {
                    if (unit->is_target_within_min_range(target, 1))
                    { // 004E7C52
                        helper::load_unit_proc(target, unit);
                        target->order_to(OrderId::MoveUnload, unit->order_target.pt.x, unit->order_target.pt.y);
                        if (target->pAI != nullptr)
                        {
                            helper::do_order(target, OrderId::Move, target->sprite->position.x, target->sprite->position.y, false);
                        }
                    }
                    else
                    { // 004E7C94
                        helper::prepend_order_targeting_unit(target, unit, OrderId::Pickup2);
                        prepare_for_next_order(target);
                        if (helper::ai_are_on_connected_regions(unit, target->sprite->position.x, target->sprite->position.y) != 0)
                        {
                            helper::set_secondary_waypoint(unit, target->sprite->position.x, target->sprite->position.y);
                            helper::move_towards(unit, target);
                        }
                    }
                }
            }
        }
    }
    */
}

// Unit is a unit trying to enter a transport
void orders_enter_transport(Unit* unit) {
    Unit* target = unit->order_target.unit;
    if (target && !(target->status & UnitStatus::IsHallucination) && units_dat::space_provided[target->id] != 0 && bw::can_be_entered_by(target, unit)) {
        if (unit->main_order_state == 0 && target->main_order_id != OrderId::Pickup2) {
            if (target->status & UnitStatus::Reacts) {
                helper::prepend_order_targeting_unit(target, unit, OrderId::Pickup2);
                prepare_for_next_order(target);
            }
            unit->main_order_state = 1;
        }

        if (target->sprite->position.x != unit->next_target_waypoint.x
            || target->sprite->position.y != unit->next_target_waypoint.y) {
            unit->next_target_waypoint.x = target->sprite->position.x;
            unit->next_target_waypoint.y = target->sprite->position.y;
        }

        helper::move_towards(unit, target);
        if (unit->is_target_within_min_range(target, 1)) {
            helper::load_unit_proc(target, unit);
        }
    } else if (unit->order_queue_head) {
        unit->user_action_flags |= 1;
        prepare_for_next_order(unit);
    } else if (unit->unit_ai) {
        unit->order_computer(OrderId::ComputerAI);
    } else {
        unit->order_computer(units_dat::return_to_idle_order[unit->id]);
    }
}

void orders_unload(Unit* unit) {
    if (!(unit->status & UnitStatus::GroundedBuilding) && unit->main_order_timer != 0) {
        return;
    }

    bool unit_unloaded = false;
    for (u32 i = 0; i < TransportCap; ++i) {
        Unit* cargo = get_loaded_unit_from_index(unit, i);
        if (cargo) {
            if (helper::unload_unit_proc(cargo)) {
                unit_unloaded = true;
                if (!(unit->status & UnitStatus::GroundedBuilding)) {
                    break;
                }
            }
        }
    }

    if (!(unit->status & UnitStatus::GroundedBuilding)) {
        if (unit_unloaded) {
            if (unit->has_loaded_unit()) {
                return;
            }
        } else if (unit->unit_ai) {
            helper::ai_unload_failure(unit);
            return;
        }
    }

    if (unit->order_queue_head) {
        unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
        prepare_for_next_order(unit);
        return;
    }
    
    if (unit->unit_ai) {
        if (unit->main_order_id == OrderId::Die) {
            unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
            prepare_for_next_order(unit);
            return;
        }

        while (unit->order_queue_tail) {
            if (orders_dat::can_be_interrupted[unit->order_queue_tail->order_id]
                || unit->order_queue_tail->order_id == OrderId::ComputerAI) {
                helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
            } else {
                break;
            }
        }
        unit->order_to(OrderId::ComputerAI);
    } else {
        unit->order_to_idle();
    }
}

bool can_enter_transport(Unit* transport, Unit* unit) { // 004E6E00
    if (!(transport->status & UnitStatus::Completed)) {
        return 0;
    }
    if (transport->is_frozen()) {
        return 0;
    }
    if (!(unit->status & UnitStatus::Completed)) {
        return 0;
    }
    if (unit->status & UnitStatus::Burrowed) {
        return 0;
    }
    if (!bw::is_allied_to(unit->player, transport->player)) {
        return 0;
    }
    if (bw::transport_is_full(transport)) {
        return 0;
    }
    if (unit->status & UnitStatus::InAir) {
        return 0;
    }

    u32 space_provided = units_dat::space_provided[transport->id];
    s32 space_required = 0;
    Unit* loaded_unit;
    for (u32 index = 0; index < space_provided; ++index) {
        loaded_unit = helper::get_loaded_unit_from_index(transport, index);
        if (loaded_unit) {
            space_required += units_dat::space_required[loaded_unit->id];
        }
    }
    return space_provided - space_required >= units_dat::space_required[unit->id];
}
}