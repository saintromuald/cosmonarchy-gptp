#include "unit_movement.h"
#include <SCBW/api.h>

namespace { // helper declarations

}

namespace hooks {

    s32 unitMovementDormant(CUnit* unit) {     // 0046A8D0
        s32 flags; // edx
        CSprite* sprite; // ecx

        flags = unit->status;
/*      if (!(flags & UnitStatus::IgnoreTileCollision) || !(flags & UnitStatus::IsNormal)) {
            sprite = unit->sprite;
            if (unit->moveTarget.pt.x == sprite->position.x
                && unit->moveTarget.pt.y == sprite->position.y
                && ((flags & UnitStatus::Unmovable) != 0) != -1
                && unit->nextTargetWaypoint.x == unit->position.x
                && unit->nextTargetWaypoint.y == unit->position.y) {
                return 0;
            }
        }
        */
        if (!(flags & UnitStatus::IgnoreTileCollision) || !(flags & UnitStatus::IsNormal)) {
            sprite = unit->sprite;
            if (sprite!=NULL && unit->moveTarget.pt.x == sprite->position.x
                && unit->moveTarget.pt.y == sprite->position.y
                && ((flags & UnitStatus::Unmovable) != 0) != -1
                && unit->nextTargetWaypoint.x == unit->position.x
                && unit->nextTargetWaypoint.y == unit->position.y) {
                return 0;
            }
        }
        unit->movementState = 9; // At rest
        return 1;
    }
}

namespace { // helper definitions

}