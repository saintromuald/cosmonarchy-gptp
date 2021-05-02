#include "repair_order.h"
#include <SCBW/api.h>

namespace hooks
{
    void orders_Repair1(CUnit* unit)
    { // 004673D0
        bool jump_to_676C4 = false; // handle various early invalid cases
        bool bEndThere = false;
        CUnit* target = unit->orderTarget.unit;
        if (target == nullptr)
        {
            jump_to_676C4 = true;
        }
        else if (target->hitPoints >= units_dat::MaxHitPoints[target->id])
        {
            jump_to_676C4 = true;
        }
        else if (target->stasisTimer != 0)
        {
            jump_to_676C4 = true;
        }
        else if (unit->pAI != nullptr && target->status & UnitStatus::IsBuilding && target->orderTarget.unit != unit)
        {
            jump_to_676C4 = true;
        }
        else if (target->status & UnitStatus::InTransport)
        {
            jump_to_676C4 = true;
        }
        else if (!(target->status & UnitStatus::Completed))
        {
            jump_to_676C4 = true;
        }

        if (jump_to_676C4)
        {
            unit->sprite->playIscriptAnim(IscriptAnimation::WalkingToIdle, true);
            unit->orderToIdle();
            bEndThere = true;
        }

        if (!bEndThere)
        {
            bool jump_to_default_switch;
            if (!jump_to_676C4)
            {
                jump_to_default_switch = ((unit->mainOrderState > 8) || (unit->mainOrderState >= 2 && unit->mainOrderState <= 5));
                if (!jump_to_default_switch && unit->mainOrderState == 0)
                {
                    s16 unkRepairInfo = 0;
                    u32 repairCostMinerals = 0, repairCostGas = 0;

                    helper::getRepairInfo(target, &repairCostMinerals, &repairCostGas, &unkRepairInfo);
                    helper::verifyResources(repairCostMinerals, repairCostGas, unit->playerId);

                    unit->worker.repairResourceLossTimer = 0;
                    unit->mainOrderState = 1; // continue directly into this case
                }

                if (!jump_to_default_switch && unit->mainOrderState == 1)
                {
                    if (!(target->status & UnitStatus::Completed))
                    {
                        unit->orderTo(OrderId::ConstructingBuilding, target);
                        if (*IS_PLACING_BUILDING)
                        {
                            if (!helper::canStillPlaceBuilding())
                            {
                                helper::refreshLayers3And4();
                                helper::endBuildingPlacement();
                            }
                        }
                        bEndThere = true;
                    }
                    else
                    {
                        if (helper::orderToMoveToTarget(unit, target))
                        {
                            helper::unitOrderMoveToTargetUnit(unit, target);
                            unit->mainOrderState = 6; // don't continue directly into this case
                        }
                        bEndThere = true;
                    }
                }

                if (!bEndThere && !jump_to_default_switch && unit->mainOrderState == 6)
                {
                    if (unit->getMovableState() == 0)
                    { // not reached destination
                        helper::function_004EB900(unit, target); // may cause unit to move or become idle
                        bEndThere = true;
                    }
                    else
                    { // reached destination or unmovable
                        if (!(unit->isTargetWithinMinRange(target, 5)))
                        {
                            unit->mainOrderState = 1; // don't continue directly into this case
                            bEndThere = true;
                        }
                        else
                        {
                            helper::function_00494BB0(unit, target->sprite->position.x, target->sprite->position.y);
                            helper::unitOrderMoveToTargetUnit(unit, target);
                            unit->mainOrderState = 7; // don't continue directly into this case
                            jump_to_default_switch = true;
                        }
                    }
                }

                if (!bEndThere && !jump_to_default_switch && unit->mainOrderState == 7)
                {
                    if (!(unit->isTargetWithinMinRange(target, 5)))
                    {
                        unit->mainOrderState = 1;
                        bEndThere = true;
                    }
                    else
                    {
                        helper::function_00494BB0(unit, target->sprite->position.x, target->sprite->position.y);
                        helper::unitOrderMoveToTargetUnit(unit, target);
                        if (!helper::isUnitPositions2Equal(unit))
                        {
                            s32 angleReturned, currentAngle;
                            u8 weaponId;
                            angleReturned =
                                scbw::getAngle(
                                    unit->sprite->position.x,
                                    unit->sprite->position.y,
                                    unit->nextTargetWaypoint.x,
                                    unit->nextTargetWaypoint.y
                                );
                            currentAngle = unit->currentDirection1 - angleReturned;
                            if (currentAngle < 0)
                            {
                                currentAngle += 256;
                            }
                            if (currentAngle > 128)
                            {
                                currentAngle = 256 - currentAngle;
                            }
                            weaponId = unit->getGroundWeapon();
                            if (currentAngle > weapons_dat::AttackAngle[weaponId])
                            {
                                jump_to_default_switch = true;
                            }
                            else
                            {
                                unit->sprite->playIscriptAnim(IscriptAnimation::AlmostBuilt, true);
                                unit->mainOrderState = 8; // don't continue directly into this case
                                jump_to_default_switch = true;
                            }
                        }
                        else
                        {
                            unit->sprite->playIscriptAnim(IscriptAnimation::AlmostBuilt, true);
                            unit->mainOrderState = 8; // don't continue directly into this case
                            jump_to_default_switch = true;
                        }
                    }
                }

                if (!bEndThere && !jump_to_default_switch && unit->mainOrderState == 8)
                {
                    helper::function_00494BB0(unit, target->sprite->position.x, target->sprite->position.y);
                    helper::unitOrderMoveToTargetUnit(unit, target);
                    if (unit->isTargetWithinMinRange(target, 5) && target->mainOrderId != OrderId::Liftoff)
                    {
                        jump_to_default_switch = true;
                    }
                    else
                    {
                        CImage* current_image = unit->sprite->images.head;
                        while (current_image != nullptr)
                        {
                            current_image->playIscriptAnim(IscriptAnimation::GndAttkToIdle);
                            current_image = current_image->link.next;
                        }
                        unit->mainOrderState = 1; // don't continue directly into this case
                        jump_to_default_switch = true;
                    }
                }

                if (!bEndThere)
                { // jump_to_default_switch CASE
                    bool bDoRepair = false;
                    if (unit->worker.repairResourceLossTimer == 0)
                    {
                        unit->worker.repairResourceLossTimer--;
                        bDoRepair = helper::ord_repair_subtract(unit, target);
                        if (!bDoRepair)
                        {
                            CImage* current_image = unit->sprite->images.head;
                            while (current_image != nullptr)
                            {
                                current_image->playIscriptAnim(IscriptAnimation::WalkingToIdle);
                                current_image = current_image->link.next;
                            }
                            unit->orderToIdle();
                            bEndThere = true;
                        }
                    }
                    else
                    {
                        unit->worker.repairResourceLossTimer--;
                        bDoRepair = true;
                    }
                    if (bDoRepair)
                    { // //67692
                        u32 hpGain = target->buildRepairHpGain;
                        if (*CHEAT_STATE & CheatFlags::OperationCwal)
                        {
                            hpGain *= 16;
                        }
                        // Talos - Advanced repair speed
                        /*
                        if (unit->id == UnitId::TerranTalos) {
                            hpGain *= 1.5;
                        }
                        */
                        target->heal(hpGain, unit);
                        if (target->hitPoints < units_dat::MaxHitPoints[target->id])
                        {
                            bEndThere = true;
                        }
                        else
                        {
                            jump_to_676C4 = true;
                        }
                    }
                }
            }

            if (!bEndThere)
            { //jump_to_676C4 CASE
                CImage* current_image = unit->sprite->images.head;
                while (current_image != nullptr)
                {
                    current_image->playIscriptAnim(IscriptAnimation::WalkingToIdle);
                    current_image = current_image->link.next;
                }
                if (unit->orderQueueHead != nullptr)
                {
                    unit->userActionFlags |= 1;
                    prepareForNextOrder(unit);
                }
                else
                { //676FE
                    u8 orderId;
                    if (unit->pAI == nullptr)
                    {
                        orderId = units_dat::ReturnToIdleOrder[unit->id];
                    }
                    else
                    {
                        orderId = OrderId::ComputerAI;
                    }
                    unit->orderComputerCL(orderId);
                }
                //6771B
                if (target != nullptr && !(target->status & UnitStatus::InTransport))
                {
                    target->connectedUnit = nullptr;
                }
            }
        }
    }

    // Hooked to allow for general Repair and some other shit - Pr0nogo
    s32 isOrderValid(char orderId, CUnit* unit, s32 unused, CUnit* target, s32 bErrorFlag) // 00474D90
    {
        u16 errorStringId;
        bool ofsubCompare;
        u8 ofsubResult;
        s32 unusedResult;

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
                ofsubResult = helper::OFSUB(target->hitPoints, units_dat::MaxHitPoints[target->id]);
                ofsubCompare = target->hitPoints - units_dat::MaxHitPoints[target->id] < 0;
                if (ofsubCompare ^ ofsubResult)
                {
                    return 1;
                }
                if (bErrorFlag)
                {
                    *(s16*)bErrorFlag = errorStringId;
                }
                return 1;
            }
            case OrderId::Harvest1:
            {
                if (!target)
                {
                    return 1;
                }
                errorStringId = unit->getRace() + 893;
                if (target->id == UnitId::ResourceVespeneGeyser)
                {
                    if (bErrorFlag)
                    {
                        *(s16*)bErrorFlag = errorStringId;
                    }
                    return 0;
                }
                if (!helper::unitIsResource(target))
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
                if (unit->playerId != target->playerId)
                {
                    if (bErrorFlag)
                    {
                        *(s16*)bErrorFlag = errorStringId;
                    }
                    return 0;
                }
                return 1;
            }
            case OrderId::Pickup2:
            {
                errorStringId = 890;
                if (!target || unit->playerId != target->playerId)
                {
                    if (bErrorFlag)
                    {
                        *(s16*)bErrorFlag = errorStringId;
                    }
                    return 0;
                }
                if (helper::canEnterTransport(unit, target))
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
            }
            case OrderId::HealMove:
            {
                if (!target)
                {
                    return 1;
                }
                if (!helper::techUseAllowed(unit, TechId::Healing, unit->playerId))
                {
                    return 0;
                }
                errorStringId = 1325;
                if (!(target->status & 6)) // GroundedBuilding and InAir
                {
                    if (units_dat::BaseProperty[target->id] & UnitProperty::Organic)
                    {
                        ofsubResult = helper::OFSUB(target->hitPoints, units_dat::MaxHitPoints[target->id]);
                        ofsubCompare = target->hitPoints - units_dat::MaxHitPoints[target->id] < 0;
                        if (ofsubCompare ^ ofsubResult)
                        {
                            return 1;
                        }
                    }
                }
                if (bErrorFlag)
                {
                    *(s16*)bErrorFlag = errorStringId;
                }
                return 0;
            }
            default:
            {
                return 1;
            }
        }
        return unusedResult;
    }

    // Hooked to investigate Repair crashes - Pr0nogo
    s32 orderAllowed(CUnit* unit, u16 orderId, s32 player) // 0046DC20
    {
        s32 result;
        s16 unitId;

        *lastInternalErr = 0;
        if (orderId < OrderId::None)
        {
            if (orders_dat::RequirementsOffset[orderId] == -1)
            {
                helper::parseOrdersDatReqs();
            }
            if (player == unit->playerId)
            {
                if (orderId == OrderId::RallyPoint1 || orderId == OrderId::RallyPoint2 || unit->status & UnitStatus::Completed)
                {
                    if (unit->isFrozen())
                    {
                        *lastInternalErr = 10;
                        result = 0;
                    }
                    else if (unit->status & UnitStatus::Burrowed && unit->id != UnitId::ZergLakizilisk && orderId != OrderId::Unburrow)
                    {
                        *lastInternalErr = 11;
                        result = 0;
                    }
                    else
                    {
                        unitId = unit->id;
                        if (unitId == UnitId::TerranMason && unit->mainOrderId == OrderId::ConstructingBuilding && orderId != 1
                            || (scbw::unitCanNuke(unit) && unit->mainOrderId == OrderId::NukeTrack)
                            || unitId == UnitId::ProtossArchon && unit->mainOrderId == OrderId::CompletingArchonSummon
                            || unitId == UnitId::ProtossMindTyrant && unit->mainOrderId == OrderId::CompletingArchonSummon)
                        {
                            *lastInternalErr = 8;
                            result = 0;
                        }
                        else
                        {
                            if (orders_dat::RequirementsOffset[orderId])
                            {
                                result = helper::parseRequirementOpcodes(unit,
                                    orderId,
                                    player,
                                    orders_dat::RequirementsOffset[orderId],
                                    (s32)ordersRequirementTree);
                            }
                            else
                            {
                                *lastInternalErr = 23;
                            }
                        }
                    }
                }
                else
                {
                    *lastInternalErr = 20;
                    result = 0;
                }
            }
            else
            {
                *lastInternalErr = 1;
                result = 0;
            }
        }
        else
        {
            *lastInternalErr = 15;
            result = 0;
        }
        return result;
    }
}