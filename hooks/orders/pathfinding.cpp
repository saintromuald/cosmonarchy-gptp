#include <SCBW/api.h>
#include "globals.h"
#include <MathUtils.h>

namespace hooks
{
    // Hooked to resolve contextual order issues - Pr0nogo
    s32 moveToTarget(Unit* target, Unit* unit) // 004EB720
    {
        Unit* v2;
        Sprite* v3;
        s16 v4;
        s32 result;
        unitPath* v6;
        s32 v7;
        Sprite* v8;
        s32 v9;
        char v10;
        Point16 v11;
        Order* v12;
        s32 v13;

        v2 = unit->orderTarget.unit;
        if (target == v2
            && ((v3 = v2->sprite, v4 = unit->orderTarget.pt.x, v4 == v3->position.x) && (v4 = unit->orderTarget.pt.y, v4 == v3->position.y)
                || helper::isPointAtUnitBorder(v4, v2, unit, *(s32*)&unit->orderTarget)))
        {
            unit->status &= 0xFFF7FFFF;
            result = 1;
        }
        else if (unit->pAI && unit->mainOrderId != 96 && helper::AI_PrepareMovingTo(unit, target->sprite->position.x, target->sprite->position.y))
        {
            result = 0;
        }
        else
        {
            v6 = (unitPath*)unit->path;
            if (v6)
                v6->isCalculating |= 1u;
            v8 = target->sprite;
            v9 = v8->position.x;
            v7 = v8->position.y;
            if (v9 != unit->orderTarget.pt.x || v7 != unit->orderTarget.pt.y || target != unit->orderTarget.unit)
            {
                unit->orderTarget.pt.y = v7;
                v10 = unit->movementFlags & 0xFB;
                unit->orderTarget.pt.x = v9;
                v11 = unit->orderTarget.pt;
                unit->orderTarget.unit = target;
                unit->movementFlags = v10 | 1;
                unit->nextTargetWaypoint = v11;
            }
            v12 = unit->orderQueueHead;
            v13 = unit->status & 0xFFF7FFFF;
            unit->recentOrderTimer = 15;
            unit->status = v13;
            if (!v12 || orders_dat::ButtonIcon[v12->orderId])
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