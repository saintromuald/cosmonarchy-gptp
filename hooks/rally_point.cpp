#include "rally_point.h"
#include "../SCBW/api.h"
#include "globals.h"

namespace hooks
{
	// Orders newly-produced units to rally, based upon the properties of the building that produced it.
	// @param  unit      The unit that needs to receive rally orders.
	// @param  factory   The unit (building) that created the given unit.
	// Identical to sub_466F50 @ 00466F50

	void orderNewUnitToRallyHook(Unit* unit, Unit* factory)
	{
		// Do nothing if the rally target is the factory itself or the rally target position is 0
        if (factory) {
            UnitExt* factoryExt = globals.get_unit_ext(factory);
            if (factory->isFactory()
                && factory->rally.unit != factory
                && factory->rally.pt.x != 0)
            {
                // Following should be allowed only on friendly units
                if (factory->rally.unit && scbw::isAlliedTo((factory->rally.unit)->playerId, unit->playerId))
                {
                    unit->orderTo(OrderId::Follow, factory->rally.unit);
                }
                else
                {
                    unit->orderTo(OrderId::Move, factory->rally.pt.x, factory->rally.pt.y);
                }
            }
        }
	}

	// Called when the player sets the rally point on the ground.
	void setRallyPosition(Unit* unit, u16 x, u16 y)
	{
        UnitExt* unitExt = globals.get_unit_ext(unit);
        unit->rally.unit = nullptr;
        unit->rally.pt.x = x;
        unit->rally.pt.y = y;
	}

	// Called when the player sets the rally point on a unit.
	void setRallyUnit(Unit* unit, Unit* target)
	{
		if (target == nullptr)
        {
            target = unit;
        }

        UnitExt* unitExt = globals.get_unit_ext(unit);
        unit->rally.unit = target;
        unit->rally.pt.x = target->getX();
        unit->rally.pt.y = target->getY();
	}

    // Hooked to allow rally for morphing Larval Colonies - Pr0nogo
    // Possible downstream effect: need to clear rally for the creep colony if the morph cancelled?
    u32 BTNSCOND_Rally(u32 unused, u32 player, Unit* unit) // 00429520
    {
        *lastInternalError = 0;
        if (orders_dat::RequirementsOffset[OrderId::RallyPoint2] == -1)
        {
            helper::parseOrdersDatReqs();
        }

        u32 result;
        if (player == unit->playerId)
        {
            if (unit->isFrozen())
            {
                *lastInternalError = 10;
                result = 0;
            }
            else if (unit->status & UnitStatus::Burrowed && unit->id != UnitId::ZergLakizilisk) // ?
            {
                *lastInternalError = 11;
                result = 0;
            }
            else
            {
                u16 unitId = unit->id;
                if (unitId == UnitId::TerranMason && unit->mainOrderId == OrderId::ConstructingBuilding
                  || scbw::unitCanNuke(unit) && unit->mainOrderId == OrderId::NukeTrack
                  || unitId == UnitId::ProtossAugur && unit->mainOrderId == OrderId::CompletingArchonSummon)
                {
                    *lastInternalError = 8;
                    result = 0;
                }
                else
                {
                    result = orders_dat::RequirementsOffset[OrderId::RallyPoint2];
                    if (orders_dat::RequirementsOffset[OrderId::RallyPoint2])
                    {
                        if (unit->id == UnitId::ZergCreepColony && unit->buildQueue[unit->buildQueueSlot] == UnitId::ZergLarvalColony) // lame
                        {
                            result = 1;
                        }
                        else
                        {
                            result = helper::parseRequirementOpcodes(unit, OrderId::RallyPoint2, player, (u32)ordersRequirementTree, orders_dat::RequirementsOffset[OrderId::RallyPoint2]);
                        }
                    }
                    else
                    {
                        *lastInternalError = 23;
                    }
                }
            }
        }
        else
        {
            *lastInternalError = 1;
            result = 0;
        }
        return result;
    }

    Order* RemoveReferences(Unit* unit, Unit* dying)
    {
        if (unit->currentBuildUnit == dying)
        {
            unit->currentBuildUnit = 0;
        }
        if (unit->orderTarget.unit == dying)
        {
            unit->orderTarget.unit = 0;
        }
        if (unit->moveTarget.unit == dying)
        {
            unit->moveTarget.unit = 0;
        }
        if (unit->autoTargetUnit == dying)
        {
            unit->autoTargetUnit = 0;
        }
        if (unit->connectedUnit == dying)
        {
            unit->connectedUnit = 0;
        }
        if (unit->irradiatedBy == dying)
        {
            unit->irradiatedBy = 0;
        }
        if (units_dat::BaseProperty[unit->id] & UnitProperty::Worker)
        {
            if (unit->worker.targetResource.unit == dying)
            {
                unit->worker.targetResource.unit = 0;
            }
            if (unit->worker.repairResourceLossTimer)
            {
                if (unit->worker.harvest_target == dying)
                {
                    unit->worker.repairResourceLossTimer = 0;
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
        if (unit->isFactory())
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
                RemoveReferences(unit->subunit, dying);
            }
        }
        Order* result = unit->orderQueueHead;
        while (result)
        {
            Order* next = result->next;
            if (result->target.unit == dying)
            {
                helper::remove_order_from_unit_queue(result, unit);
            }
            result = next;
        }
        return 0; // used to return result, but it will always be 0. the return value is never used anyways
    }

    Unit* RightClickCommand(signed int x, signed int y, Unit *target, signed int fow_unit_id, char queued)
    {
        MovementGroup movement_group = {};
        movement_group.pos.x = x;
        movement_group.pos.y = y;
        helper::PrepareFormationMovement(&movement_group, 1);
        if (target)
        {
            fow_unit_id = UnitId::None;
        }
        else if (fow_unit_id != UnitId::None)
        {
            u32 tileId = x / 32 + mapTileSize->width * (y / 32);
            if (!((1 << *ACTIVE_NATION_ID) & (*activeTileHandle)[tileId]))
            {
                target = helper::FindFowUnit(y, x, fow_unit_id);
                fow_unit_id = UnitId::None;
            }
        }
        *unit_iterator_pos = 0;
        Unit* result = helper::GetNextCommandedUnit();
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
                        if (unit->isFactory())
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
            byte_68C300[0] = OrderId::Nothing2;
            u8 rightOrder = helper::GetRightClickOrder(order_type, unit, x, y, &target, fow_unit_id);
            if (rightOrder != OrderId::Nothing2)
            {
                if (rightOrder == OrderId::RallyPoint1)
                {
                    if (unit->isFactory())
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
                        if (unit->isFactory())
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
                                helper::TargetedOrder_Unit(target, unit->subunit, queued != 0, units_dat::AttackUnitOrder[subunit->id]);
                            }
                            rightOrder = units_dat::AttackUnitOrder[unit->id];
                        }
                        if (helper::OrderAllowed(unit, rightOrder, *ACTIVE_NATION_ID) == 1)
                        {
                            if (target)
                            {
                                helper::TargetedOrder_Unit(target, unit, queued != 0, rightOrder);
                                if (byte_68C300[0] != OrderId::Nothing2)
                                {
                                    helper::TargetedOrder_Unit(target, unit, queued != 0, byte_68C300[0]);
                                }
                            }
                            else
                            {
                                movement_group.pos.x = x;
                                movement_group.pos.y = y;
                                if (fow_unit_id == UnitId::None)
                                {
                                    helper::GetFormationMovementTarget(unit, &movement_group);
                                }
                                else
                                {
                                    movement_group.current_unit_target = movement_group.pos;
                                }

                                u16 target_x = movement_group.current_unit_target.x;
                                u16 target_y = movement_group.current_unit_target.y;
                                helper::TargetedOrder_Ground(rightOrder, fow_unit_id, unit, queued != 0, target_x, target_y);
                                if (byte_68C300[0] != OrderId::Nothing2)
                                {
                                    helper::TargetedOrder_Ground(byte_68C300[0], fow_unit_id, unit, queued != 0, target_x, target_y);
                                }
                            }
                            unit->userActionFlags |= 2u;
                        }
                    }
                }
            }
            result = helper::GetNextCommandedUnit();
        }
        return 0;
    }
}
