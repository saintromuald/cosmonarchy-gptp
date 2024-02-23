#include "unit_movement.h"
#include <SCBW/api.h>

namespace hooks
{
	s32 unit_movement_dormant(Unit* unit) // 0046A8D0
	{
		u32 flags = unit->status;
		if (!(flags & UnitStatus::IgnoreTileCollision) || !(flags & UnitStatus::IsNormal))
		{
			Sprite* sprite = unit->sprite;
			if  (sprite
            &&   unit->move_target.pt.x == sprite->position.x
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

	// Hooked by DarkenendFantasies to allow selectively enabling or disabling new collisions.
	bool can_collide_with(Unit* unit, Unit* colliding_unit) // 0042DEE0
	{
		u32 collider_status = colliding_unit->status;
		if (collider_status & UnitStatus::UNKNOWN5) {
			u8 player = colliding_unit->player;
			if (player == 11) {
				player = colliding_unit->sprite->player;
			}

			if (!scbw::are_mutual_allies(unit->player, player)) {
				return false;
			}
		}
		if (unit_paths_over_small_units(unit) || unit_paths_over_small_units(colliding_unit))
		{
			if ((unit_collides_with_titanic(colliding_unit) && !unit_collides_with_titanic(unit))
			||   unit_collides_with_titanic(unit)		    && !unit_collides_with_titanic(colliding_unit)) {
				return false;
			}
		}

		u16 unit_id = unit->id;
		u16 collider_id = colliding_unit->id;
		if ((unit_id_is_larva(unit_id) && !(collider_status & UnitStatus::GroundedBuilding)) || unit_id_is_larva(collider_id)) {
			return false;
		}

		if (unit_id_is_tiny_unit(unit_id) && !unit_id_is_tiny_unit(collider_id) && !(collider_status & UnitStatus::GroundedBuilding)
		||	unit_id_is_tiny_unit(collider_id) && !unit_id_is_tiny_unit(unit_id)) {
			return false;
		}

		if (!(unit->status & UnitStatus::IsGathering) || collider_status & UnitStatus::GroundedBuilding) {
			return true;
		}

		if (!(collider_status & UnitStatus::IsGathering) || unit->main_order_id == OrderId::ReturnGas) {
			return false;
		}

		return colliding_unit->main_order_id == OrderId::CanEnterGasMine;
	}

	// Hooked to investigate crashes. -Pr0nogo
	s32 unit_movement_start_path(Unit* unit) // 0046BD90
	{
		s32 result = helper::unit_movement_another_path(unit, unit->move_target.pt);
		if (result)
		{
			u8 action_flags = unit->user_action_flags;
			if (action_flags & UserActionFlags_UMStartPath
			&& (unit->user_action_flags = action_flags & 0xFD, unit->flingy_movement_type == MovementType_Iscript))
			{
				if (!unit->path) { // New
					return 0;
				}
				unit->path->collision_recheck_timer = 2;
				unit->movement_state = UnitMovement_UIOrderDelay;
				result = 1;
			} else {
				unit->movement_state = UnitMovement_FollowPath;
				result = 1;
			}
		} else {
			unit->movement_state = UnitMovement_FailedPath;
		}
		return result;
	}

	// Hooked to resolve a nullsprite crash. -Pr0nogo
	b32 move_towards(Unit* target, Unit* unit) // 004EB900
	{
		if (!unit->sprite) { // New
			return 0;
		}
		if (unit->recent_order_timer)
		{
			return 1;
		}
		if (!((1 << unit->player) & target->sprite->visibility_status) && !helper::is_target_within_min_range(unit, 448, target))
		{
			if (helper::is_unit_movable(unit) == 1)
			{
				helper::to_idle(unit);
				return 0;
			}
			return 1;
		}
		return helper::move_to_target(target, unit);
	}
}