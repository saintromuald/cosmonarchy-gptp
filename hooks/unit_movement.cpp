#include "unit_movement.h"
#include <SCBW/api.h>

namespace hooks
{

	s32 unit_movement_dormant(Unit* unit) // 0046A8D0
	{
		s32 flags; // edx
		Sprite* sprite; // ecx

		flags = unit->status;
		/*      if (!(flags & UnitStatus::IgnoreTileCollision) || !(flags & UnitStatus::IsNormal)) {
					sprite = unit->sprite;
					if (unit->move_target.pt.x == sprite->position.x
						&& unit->move_target.pt.y == sprite->position.y
						&& ((flags & UnitStatus::Unmovable) != 0) != -1
						&& unit->next_target_waypoint.x == unit->position.x
						&& unit->next_target_waypoint.y == unit->position.y) {
						return 0;
					}
				}
				*/
		if (!(flags & UnitStatus::IgnoreTileCollision) || !(flags & UnitStatus::IsNormal))
		{
			sprite = unit->sprite;
			if  (sprite != nullptr && unit->move_target.pt.x == sprite->position.x
			&&   unit->move_target.pt.y == sprite->position.y
			&& ((flags & UnitStatus::Unmovable) != 0) != -1
			&&   unit->next_target_waypoint.x == unit->position.x
			&&   unit->next_target_waypoint.y == unit->position.y)
			{
				return 0;
			}
		}
		unit->movement_state = 9; // At rest
		return 1;
	}

	void move_unit(Unit* unit, s16 x, s16 y, bool ignore_collision) // 004EB9F0
	{
		Point16 old_pos = { unit->sprite->position.x, unit->sprite->position.y };
		Point16 new_pos = { x, y };
		helper::fix_target_location(&new_pos, unit->id);
        unit->position.x = new_pos.x;
        unit->position.y = new_pos.y;
        unit->halt.x = new_pos.x << 8;
        unit->halt.y = new_pos.y << 8;
        helper::move_sprite(unit->sprite, new_pos.x, new_pos.y);
        if (unit->sprite)
        {
            if (unit->main_order_id)
            {
                helper::set_move_target_xy(unit, new_pos.x, new_pos.y);
                if (new_pos.x != unit->next_target_waypoint.x || new_pos.y != unit->next_target_waypoint.y)
                {
                    unit->next_target_waypoint.x = new_pos.x;
                    unit->next_target_waypoint.y = new_pos.y;
                }
            }
        }
        if (!(units_dat::base_property[unit->id] & UnitProperty::Subunit) && !ignore_collision)
        {
            helper::update_unit_sorting_info(new_pos.x - old_pos.x, unit, new_pos.y - old_pos.y);
            if (!ignore_collision && !(unit->sprite->flags & SpriteFlags::Hidden))
            {
                helper::check_unstack(unit);
            }
        }
	}
}