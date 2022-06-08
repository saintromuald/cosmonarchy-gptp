#include "rally_point.h"
#include "../SCBW/api.h"
#include "globals.h"

namespace hooks
{
	// Orders newly-produced units to rally, based upon the properties of the building that produced it.
	// @param  unit      The unit that needs to receive rally orders.
	// @param  factory   The unit (building) that created the given unit.
	// Identical to sub_466F50 @ 00466F50

	void order_new_unit_to_rally(Unit* unit, Unit* factory)
	{
		// Do nothing if the rally target is the factory itself or the rally target position is 0
        if (factory) {
            UnitExt* factoryExt = globals.get_unit_ext(factory);
            if (factory->is_factory()
                && factory->rally.unit != factory
                && factory->rally.pt.x != 0)
            {
                // Following should be allowed only on friendly units
                if (factory->rally.unit && scbw::is_allied_to((factory->rally.unit)->player, unit->player))
                {
                    unit->order_to(OrderId::Follow, factory->rally.unit);
                }
                else
                {
                    unit->order_to(OrderId::Move, factory->rally.pt.x, factory->rally.pt.y);
                }
            }
        }
	}

	// Called when the player sets the rally point on the ground.
	void set_rally_position(Unit* unit, u16 x, u16 y)
	{
        UnitExt* unitExt = globals.get_unit_ext(unit);
        unit->rally.unit = nullptr;
        unit->rally.pt.x = x;
        unit->rally.pt.y = y;
	}

	// Called when the player sets the rally point on a unit.
	void set_rally_unit(Unit* unit, Unit* target)
	{
		if (target == nullptr)
        {
            target = unit;
        }

        UnitExt* unitExt = globals.get_unit_ext(unit);
        unit->rally.unit = target;
        unit->rally.pt.x = target->get_x();
        unit->rally.pt.y = target->get_y();
	}

    // Hooked to allow rally for morphing Larval Colonies - Pr0nogo
    // Possible downstream effect: need to clear rally for the creep colony if the morph cancelled?
    u32 btnscond_rally(u32 unused, u32 player, Unit* unit) // 00429520
    {
        *last_internal_error = 0;
        if (orders_dat::RequirementsOffset[OrderId::RallyPoint2] == -1)
        {
            helper::parse_orders_dat_requirements();
        }

        u32 result;
        if (player == unit->player)
        {
            if (unit->is_frozen())
            {
                *last_internal_error = 10;
                result = 0;
            }
            else if (unit->status & UnitStatus::Burrowed && unit->id != UnitId::ZergLakizilisk) // ?
            {
                *last_internal_error = 11;
                result = 0;
            }
            else
            {
                u16 unit_id = unit->id;
                if (unit_id == UnitId::TerranMason && unit->main_order_id == OrderId::ConstructingBuilding
                  || scbw::unit_can_nuke(unit) && unit->main_order_id == OrderId::NukeTrack
                  || unit_id == UnitId::ProtossAugur && unit->main_order_id == OrderId::CompletingArchonSummon)
                {
                    *last_internal_error = 8;
                    result = 0;
                }
                else
                {
                    result = orders_dat::RequirementsOffset[OrderId::RallyPoint2];
                    if (orders_dat::RequirementsOffset[OrderId::RallyPoint2])
                    {
                        if (unit->id == UnitId::ZergCreepColony && unit->build_queue[unit->build_queue_slot] == UnitId::ZergLarvalColony) // lame
                        {
                            result = 1;
                        }
                        else
                        {
                            result = helper::parse_requirement_opcodes(unit, OrderId::RallyPoint2, player, (u32)orders_requirement_tree, orders_dat::RequirementsOffset[OrderId::RallyPoint2]);
                        }
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
            *last_internal_error = 1;
            result = 0;
        }
        return result;
    }

    Order* remove_references(Unit* unit, Unit* dying) // 0049E590
    {
        if (unit->current_build_unit == dying)
        {
            unit->current_build_unit = 0;
        }
        if (unit->order_target.unit == dying)
        {
            unit->order_target.unit = 0;
        }
        if (unit->move_target.unit == dying)
        {
            unit->move_target.unit = 0;
        }
        if (unit->auto_target_unit == dying)
        {
            unit->auto_target_unit = 0;
        }
        if (unit->connected_unit == dying)
        {
            unit->connected_unit = 0;
        }
        if (unit->irradiate_caster == dying)
        {
            unit->irradiate_caster = 0;
        }
        if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
        {
            if (unit->worker.target_resource.unit == dying)
            {
                unit->worker.target_resource.unit = 0;
            }
            if (unit->worker.is_harvesting)
            {
                if (unit->worker.harvest_target == dying)
                {
                    unit->worker.is_harvesting = 0;
                    unit->worker.harvest_target = 0;
                }
            }
        }
        else if (unit->status & UnitStatus::GroundedBuilding)
        {
            if (unit->building.addon == dying)
            {
                unit->building.addon = 0;
            }
        }
        if (unit->is_factory())
        {
            UnitExt* unitExt = globals.get_unit_ext(unit);
            if (unit->rally.unit == dying)
            {
                unit->rally.unit = 0;
                unit->rally.pt.x = dying->sprite->position.x;
                unit->rally.pt.y = dying->sprite->position.y;
            }
        }
        if (unit->subunit)
        {
            if (units_dat::BaseProperty[unit->subunit->id] & UnitProperty::Subunit)
            {
                remove_references(unit->subunit, dying);
            }
        }
        Order* result = unit->order_queue_head;
        while (result)
        {
            Order* next = result->next;
            if (result->target.unit == dying)
            {
                helper::remove_order_from_unit_queue(result, unit);
            }
            result = next;
        }
        return 0; // used to return result, but it will always be 0. The return value is never used anyways. -Veeq7
    }

    Unit* right_click_command(u32 x, u32 y, Unit *target, u32 fow_unit_id, char queued) // 004560D0
    {
        MovementGroup movement_group = {};
        movement_group.pos.x = x;
        movement_group.pos.y = y;
        helper::prepare_formation_movement(&movement_group, 1);
        if (target)
        {
            fow_unit_id = UnitId::None;
        }
        else if (fow_unit_id != UnitId::None)
        {
            u32 tileId = x / 32 + map_tile_size->width * (y / 32);
            if (!((1 << *ACTIVE_NATION_ID) & (*activeTileArrayInt)[tileId]))
            {
                target = helper::find_fow_unit(y, x, fow_unit_id);
                fow_unit_id = UnitId::None;
            }
        }
        *unit_iterator_pos = 0;
        Unit* result = helper::get_next_commanded_unit();
        u8 order_type;
        for (Unit* unit = result; result; unit = result)
        {
            u16 id = unit->id;
            if (id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed)
            {
                order_type = 3;
            }
            else
            {
                order_type = units_dat::RightClickAction[id];
                if (!order_type)
                {
                    if (unit->status & UnitStatus::GroundedBuilding)
                    {
                        if (unit->is_factory())
                        {
                            order_type = 2;
                        }
                    }
                }
            }
            if (unit->status & UnitStatus::IsHallucination)
            {
                if (order_type == 4 || order_type == 5)
                {
                    order_type = 1;
                }
            }
            unk_order_array[0] = OrderId::Nothing2;
            u8 rightOrder = helper::get_right_click_order(order_type, unit, x, y, &target, fow_unit_id);
            if (rightOrder != OrderId::Nothing2)
            {
                if (rightOrder == OrderId::RallyPoint1)
                {
                    if (unit->is_factory())
                    {
                        Unit* rallyTarget = target;
                        if (!rallyTarget)
                        {
                            rallyTarget = unit;
                        }

                        UnitExt* unitExt = globals.get_unit_ext(unit);
                        unit->rally.unit = rallyTarget;
                        unit->rally.pt.x = rallyTarget->sprite->position.x;
                        unit->rally.pt.y = rallyTarget->sprite->position.y;
                    }
                }
                else if (unit != target)
                {
                    if (rightOrder == OrderId::RallyPoint2)
                    {
                        if (unit->is_factory() || unit->id == UnitId::ZergEgg || unit->id == UnitId::ZergCocoon)
                        {
                            UnitExt* unitExt = globals.get_unit_ext(unit);
                            unit->rally.unit = 0;
                            unit->rally.pt.x = x;
                            unit->rally.pt.y = y;
                        }
                    }
                    else
                    {
                        if (rightOrder == OrderId::Attack1)
                        {
                            Unit* subunit = unit->subunit;
                            if (subunit)
                            {
                                helper::targeted_order_unit(target, unit->subunit, queued != 0, units_dat::AttackUnitOrder[subunit->id]);
                            }
                            rightOrder = units_dat::AttackUnitOrder[unit->id];
                        }
                        if (helper::order_is_allowed(unit, rightOrder, *ACTIVE_NATION_ID) == 1)
                        {
                            if (target)
                            {
                                helper::targeted_order_unit(target, unit, queued != 0, rightOrder);
                                if (unk_order_array[0] != OrderId::Nothing2)
                                {
                                    helper::targeted_order_unit(target, unit, queued != 0, unk_order_array[0]);
                                }
                            }
                            else
                            {
                                movement_group.pos.x = x;
                                movement_group.pos.y = y;
                                if (fow_unit_id == UnitId::None)
                                {
                                    helper::get_formation_movement_target(unit, &movement_group);
                                }
                                else
                                {
                                    movement_group.current_unit_target = movement_group.pos;
                                }

                                u16 target_x = movement_group.current_unit_target.x;
                                u16 target_y = movement_group.current_unit_target.y;
                                helper::targeted_order_ground(rightOrder, fow_unit_id, unit, queued != 0, target_x, target_y);
                                if (unk_order_array[0] != OrderId::Nothing2)
                                {
                                    helper::targeted_order_ground(unk_order_array[0], fow_unit_id, unit, queued != 0, target_x, target_y);
                                }
                            }
                            unit->user_action_flags |= 2u;
                        }
                    }
                }
            }
            result = helper:: get_next_commanded_unit();
        }
        return 0;
    }
}
