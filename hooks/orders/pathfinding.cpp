#include <SCBW/api.h>
#include "globals.h"
#include <MathUtils.h>

namespace hooks
{
    // Hooked to resolve contextual order issues - Pr0nogo
    s32 moveToTarget(Unit* target, Unit* unit) // 004EB720
    {
        Unit* orderTargetUnit = unit->orderTarget.unit;
        if (orderTargetUnit == nullptr
            || target == nullptr
            || unit == nullptr)
        {
            return 0;
        }
        Sprite* orderTargetSprite = orderTargetUnit->sprite;
        s16 orderTargetX = unit->orderTarget.pt.x;
        s16 orderTargetY = unit->orderTarget.pt.y;
        s32 result;

        bool debugBool = false;
        if (target != nullptr)
        {
            if (target == orderTargetUnit)
            {
                scbw::print("target == orderTargetUnit");
                bool debugBool = true;
            }
            else
                if ((orderTargetX == orderTargetSprite->position.x) && (orderTargetY == orderTargetSprite->position.y))
                {
                    scbw::print("targetX = spriteX && targetY = spriteY");
                    bool debugBool = true;
                }
                else
                    if (helper::isPointAtUnitBorder(orderTargetY, orderTargetUnit, unit, *(s32*)&unit->orderTarget))
                    {
                        scbw::print("isPointAtUnitBorder == true");
                        bool debugBool = true;
                    }
            else if (unit->pAI && unit->mainOrderId != 96 && helper::AI_PrepareMovingTo(unit, target->sprite->position.x, target->sprite->position.y))
            {
                scbw::print("AI");
                result = 0;
            }
        }

        if (debugBool)
        {
            unit->status &= 0xFFF7FFFF;
            result = 1;
        }

        else
        {
            unitPath* pathVar = (unitPath*)unit->path;
            if (pathVar)
            {
                pathVar->isCalculating |= 1u;
            }
            Sprite* targetSprite = target->sprite;
            s16 targetSpriteX = targetSprite->position.x;
            s16 targetSpriteY = targetSprite->position.y;
            if (targetSpriteX != unit->orderTarget.pt.x || targetSpriteY != unit->orderTarget.pt.y || target != unit->orderTarget.unit)
            {
                unit->orderTarget.pt.y = targetSpriteY;
                unit->orderTarget.pt.x = targetSpriteX;
                char movementFlagsVar = unit->movementFlags & 0xFB;
                Point16 targetPoint = unit->orderTarget.pt;
                unit->orderTarget.unit = target;
                unit->movementFlags = movementFlagsVar | 1;
                unit->nextTargetWaypoint = targetPoint;
            }
            Order* unitOrder = unit->orderQueueHead;
            unit->recentOrderTimer = 15;
            unit->status = unit->status & 0xFFF7FFFF;
            if (!unitOrder || orders_dat::ButtonIcon[unitOrder->orderId])
            {
                unit->movementFlags &= 0xDFu;
                result = 1;
            }
            else
            {
                unit->movementFlags |= 0x20u;
                result = 1;
            }
        }
        return result;
    }
}