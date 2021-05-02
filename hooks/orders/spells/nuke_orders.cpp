#include "nuke_orders.h"
#include <SCBW/api.h>

namespace hooks
{
    void orders_NukeTrack(CUnit* unit)
    {

        if (unit->mainOrderState != 0)
        {

            if (unit->mainOrderState == 6)
            {

                if (
                    unit->connectedUnit == nullptr ||
                    unit->connectedUnit->mainOrderState == 5
                    )
                { //64D3F

                    CImage* current_image = unit->sprite->images.head;

                    if (unit->orderQueueHead == nullptr)
                        unit->orderSimple(units_dat::ReturnToIdleOrder[unit->id], false);

                    helper::actUnitReturnToIdle(unit);
                    helper::changeUnitButtonSet_Sub4E5D60(unit, unit->id);
                    scbw::refreshConsole();

                    while (current_image != nullptr)
                    {
                        current_image->playIscriptAnim(IscriptAnimation::WalkingToIdle);
                        current_image = current_image->link.next;
                    }

                    if (unit->building.ghostNukeMissile != nullptr)
                    {

                        current_image = (unit->building.ghostNukeMissile)->sprite->images.head;

                        while (current_image != nullptr)
                        {
                            current_image->playIscriptAnim(IscriptAnimation::Death);
                            current_image = current_image->link.next;
                        }

                        unit->building.ghostNukeMissile = nullptr;

                    }

                    //64DD6
                    helper::function_0043DB50(unit);

                }

            }

        }
        else
        { //64DE3

            CThingy* nukeDot;

            unit->sprite->playIscriptAnim(IscriptAnimation::SpecialState1, true);

            nukeDot = helper::createThingy(
                SpriteId::Nuke_Target_Dot,
                (unit->connectedUnit)->orderTarget.pt.x,
                (unit->connectedUnit)->orderTarget.pt.y,
                unit->playerId
            );

            unit->building.ghostNukeMissile = (CUnit*)nukeDot;

            if (nukeDot != nullptr)
            {
                nukeDot->sprite->elevationLevel = unit->sprite->elevationLevel + 1;
                helper::setThingyVisibilityFlags(nukeDot);
            }

            unit->mainOrderState = 6;

        }

    }

    ;

    void orders_NukeGround(CUnit* unit)
    {

        u32 sightRange, distance;

        if (
            unit->nextTargetWaypoint.x != unit->orderTarget.pt.x ||
            unit->nextTargetWaypoint.y != unit->orderTarget.pt.y
            )
        {
            unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
            unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
        }

        sightRange = unit->getSightRange(false) * 32;

        if (unit->pAI != nullptr)
        {

            distance = scbw::getDistanceFast(
                unit->orderTarget.pt.x * 256,
                unit->orderTarget.pt.y * 256,
                unit->halt.x,
                unit->halt.y
            );

            if (distance / 256 <= sightRange * 3)
                helper::AI_orderUnitCloaking(unit);

        }

        //79267
        distance = scbw::getDistanceFast(
            unit->orderTarget.pt.x * 256,
            unit->orderTarget.pt.y * 256,
            unit->halt.x,
            unit->halt.y
        );

        if (distance / 256 > sightRange)
        {

            if (unit->recentOrderTimer == 0)
                helper::SetMoveTarget_xy(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

        }
        else
        { //792AE

            s32 direction;
            bool bEndThere = false;

            helper::setNextWaypoint(unit);

            if (
                unit->nextTargetWaypoint.x != unit->position.x ||
                unit->nextTargetWaypoint.y != unit->position.y
                )
            {

                direction =
                    unit->currentDirection1 -
                    scbw::getAngle(
                        unit->sprite->position.x,
                        unit->sprite->position.y,
                        unit->nextTargetWaypoint.x,
                        unit->nextTargetWaypoint.y
                    );

                if (direction < -1 || direction > 1)
                    bEndThere = true;

            }

            //792F9
            if (!bEndThere)
            {

                bool bLoopEnd = false;
                CUnit* current_unit = firstPlayerUnit->unit[unit->playerId];

                while (!bLoopEnd)
                {

                    if (current_unit == nullptr)
                        bLoopEnd = true;
                    else
                        if (current_unit->id == UnitId::TerranNuclearSilo &&
                            current_unit->building.silo.isReady
                            )
                            bLoopEnd = true;
                        else
                            current_unit = current_unit->player_link.next;

                }

                if (current_unit == nullptr)
                    unit->orderToIdle();
                else
                { //79333

                    CUnit* nuke = current_unit->building.silo.nuke;

                    current_unit->building.silo.nuke = nullptr;
                    current_unit->building.silo.isReady = false;

                    scbw::playSound(SoundId::Terran_GHOST_TGhLas00_wav, unit);

                    nuke->orderTo(OrderId::NukeLaunch, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
                    nuke->connectedUnit = unit;
                    helper::setAllOverlayDirectionsGeneric(nuke, 0);
                    helper::showAndEnableUnit(nuke);

                    unit->connectedUnit = nuke;
                    unit->orderTo(OrderId::NukeTrack, unit->sprite->position.x, unit->sprite->position.y);

                    if (
                        unit->status & UnitStatus::DoodadStatesThing ||
                        unit->lockdownTimer != 0 ||
                        unit->stasisTimer != 0 ||
                        unit->maelstromTimer != 0
                        )
                    { //793C4

                        if (units_dat::BaseProperty[unit->id] & UnitProperty::Building)
                            unit->currentButtonSet = UnitId::Buttons_CancelNukeStrike;
                    }
                    else
                        unit->currentButtonSet = UnitId::Buttons_CancelNukeStrike;

                    scbw::refreshConsole();
                }
            }
        }
    }
}