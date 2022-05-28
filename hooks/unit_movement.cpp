#include "unit_movement.h"
#include <SCBW/api.h>

namespace
{ // helper declarations

}

namespace hooks
{

	s32 unitMovementDormant(Unit* unit)
	{     // 0046A8D0
		s32 flags; // edx
		Sprite* sprite; // ecx

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
		if (!(flags & UnitStatus::IgnoreTileCollision) || !(flags & UnitStatus::IsNormal))
		{
			sprite = unit->sprite;
			if (sprite != nullptr && unit->moveTarget.pt.x == sprite->position.x
				&& unit->moveTarget.pt.y == sprite->position.y
				&& ((flags & UnitStatus::Unmovable) != 0) != -1
				&& unit->nextTargetWaypoint.x == unit->position.x
				&& unit->nextTargetWaypoint.y == unit->position.y)
			{
				return 0;
			}
		}
		unit->movementState = 9; // At rest
		return 1;
	}

	void MoveUnit(Unit* unit, s16 x, s16 y, bool ignoreCollision)
	{
		Point16 oldPos = { unit->sprite->position.x, unit->sprite->position.y };
		Point16 newPos = { x, y };
		helper::fix_target_location(&newPos, unit->id);
        unit->position.x = newPos.x;
        unit->position.y = newPos.y;
        unit->halt.x = newPos.x << 8;
        unit->halt.y = newPos.y << 8;
        helper::MoveSprite(unit->sprite, newPos.x, newPos.y);
        if (unit->sprite)
        {
            if (unit->mainOrderId)
            {
                helper::SetMoveTarget_xy(unit, newPos.x, newPos.y);
                if (newPos.x != unit->nextTargetWaypoint.x || newPos.y != unit->nextTargetWaypoint.y)
                {
                    unit->nextTargetWaypoint.x = newPos.x;
                    unit->nextTargetWaypoint.y = newPos.y;
                }
            }
        }
        if (!(units_dat::BaseProperty[unit->id] & UnitProperty::Subunit) && !ignoreCollision)
        {
            helper::updateUnitSortingInfo(newPos.x - oldPos.x, unit, newPos.y - oldPos.y);
            if (!ignoreCollision && !(unit->sprite->flags & SpriteFlags::Hidden))
            {
                helper::checkUnstack(unit);
            }
        }
	}
}