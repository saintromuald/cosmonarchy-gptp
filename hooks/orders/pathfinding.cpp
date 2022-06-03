#include <SCBW/api.h>
#include "globals.h"
#include <math_utils.h>

namespace hooks
{
    // Hooked to resolve contextual order issues - Pr0nogo
    s32 move_to_target(Unit* target, Unit* unit) // 004EB720
    {
        Unit* orderTargetUnit = unit->order_target.unit;
        if (orderTargetUnit == nullptr
            || target == nullptr
            || unit == nullptr)
        {
            return 0;
        }

        Sprite* orderTargetSprite = orderTargetUnit->sprite;
        s16 orderTargetX = unit->order_target.pt.x;
        s16 orderTargetY = unit->order_target.pt.y;
        s32 result;

        if (target != nullptr) // todo: probably remove this, since target is already null checked
        {
            if (target == orderTargetUnit)
            {
                scbw::print("target == orderTargetUnit");
                unit->status &= 0xFFF7FFFF;
                return 1;
            }
            else if ((orderTargetX == orderTargetSprite->position.x) && (orderTargetY == orderTargetSprite->position.y))
            {
                scbw::print("targetX = spriteX && targetY = spriteY");
                unit->status &= 0xFFF7FFFF;
                return 1;
            }
            else if (helper::isPointAtUnitBorder(orderTargetY, orderTargetUnit, unit, *(s32*)&unit->order_target))
            {
                scbw::print("isPointAtUnitBorder == true");
                unit->status &= 0xFFF7FFFF;
                return 1;
            }
            else if (unit->unit_ai && unit->main_order_id != 96 && helper::AI_PrepareMovingTo(unit, target->sprite->position.x, target->sprite->position.y))
            {
                scbw::print("AI");
                return 0;
            }
        }

        unitPath* pathVar = (unitPath*)unit->path;
        if (pathVar)
        {
            pathVar->isCalculating |= 1u;
        }
        Sprite* targetSprite = target->sprite;
        s16 targetSpriteX = targetSprite->position.x;
        s16 targetSpriteY = targetSprite->position.y;
        if (targetSpriteX != unit->order_target.pt.x || targetSpriteY != unit->order_target.pt.y || target != unit->order_target.unit)
        {
            unit->order_target.pt.y = targetSpriteY;
            unit->order_target.pt.x = targetSpriteX;
            char movementFlagsVar = unit->movement_flags & 0xFB;
            Point16 targetPoint = unit->order_target.pt;
            unit->order_target.unit = target;
            unit->movement_flags = movementFlagsVar | 1;
            unit->next_target_waypoint = targetPoint;
        }
        Order* unitOrder = unit->order_queue_head;
        unit->recentOrderTimer = 15;
        unit->status = unit->status & 0xFFF7FFFF;
        if (!unitOrder || orders_dat::ButtonIcon[unitOrder->orderId])
        {
            unit->movement_flags &= 0xDFu;
            return 1;
        }
        unit->movement_flags |= 0x20u;
        return 1;
    }

    // Hooked because the helper is fucked - Pr0nogo
    b32 getActiveTileFlag(s16 y, s16 x, s32 includedFlags, s32 excludedFlags) // 00472F80
    {
        ActiveTile tile = scbw::get_stupid_reference_to_active_tile_at(x, y);
        return tile.value & includedFlags && !(tile.value & excludedFlags);
    }

    // Hooked to investigate crashes - Pr0nogo
    bool unitHasPathToDest(Unit* unit, s32 x, s32 y) // 0049CB60
    {
        if (!unit->sprite)
        {
            return false;
        }
        RegionArray* paths = *SAI_Paths;
        s32 regionId = helper::get_region_id(unit->sprite->position.x, unit->sprite->position.y) << 6;
        return paths->regions[helper::get_region_id(x, y)].groupIndex == *(s16*)((char*)&paths->regions[0].groupIndex + regionId);
    }

    // Hooked to investigate crashes - Pr0nogo
    bool unitHasPathToUnit(Unit* pather, Unit* target) // 0049CBB0
    {
        if (!pather->sprite)
        {
            return false;
        }
        Sprite* patherSprite = pather->sprite;
        RegionArray* paths = *SAI_Paths;
        s32 regionId = helper::get_region_id(target->sprite->position.x, target->sprite->position.y) << 6;
        return *(s16*)((char*)&paths->regions[0].groupIndex + regionId) == paths->regions[helper::get_region_id(patherSprite->position.x, patherSprite->position.y)].groupIndex;
    }    
}