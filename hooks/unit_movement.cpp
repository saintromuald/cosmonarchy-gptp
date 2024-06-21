#include "unit_movement.h"
#include <BW/api.h>

namespace hooks {
	b32 unit_movement_dormant(Unit* unit) { // 0046A8D0 
		if (!(unit->status & UnitStatus::IgnoreTileCollision) || !(unit->status & UnitStatus::IsNormal)) {
			if (AssertSprite(unit->sprite)
            &&  unit->move_target.pt.x == unit->sprite->position.x
			&&  unit->move_target.pt.y == unit->sprite->position.y
			&&  unit->next_target_waypoint.x == unit->position.x
			&&  unit->next_target_waypoint.y == unit->position.y)
			{
				return 0;
			}
		}
		unit->movement_state = UnitMovement_AtRest; // At rest
		return 1;
	}

	void move_unit(Unit* unit, s16 x, s16 y, bool ignore_collision) { // 004EB9F0
		Point16 old_pos = { unit->sprite->position.x, unit->sprite->position.y };
		Point16 new_pos = { x, y };
		helper::fix_target_location(&new_pos, unit->id);
        unit->position.x = new_pos.x;
        unit->position.y = new_pos.y;
        unit->halt.x = new_pos.x << 8;
        unit->halt.y = new_pos.y << 8;
        helper::move_sprite(unit->sprite, new_pos.x, new_pos.y);
        if (unit->sprite) {
            if (unit->main_order_id) {
                helper::set_move_target_xy(unit, new_pos.x, new_pos.y);
                if (new_pos.x != unit->next_target_waypoint.x || new_pos.y != unit->next_target_waypoint.y) {
                    unit->next_target_waypoint.x = new_pos.x;
                    unit->next_target_waypoint.y = new_pos.y;
                }
            }
        }
        if (!(units_dat::base_property[unit->id] & UnitProperty::Subunit) && !ignore_collision) {
            helper::update_unit_sorting_info(new_pos.x - old_pos.x, unit, new_pos.y - old_pos.y);
            if (!ignore_collision && !(unit->sprite->flags & SpriteFlags::Hidden)) {
                helper::check_unstack(unit);
            }
        }
	}

	// Hooked by DarkenendFantasies to allow selectively enabling or disabling new collisions.
	bool can_collide_with(Unit* unit, Unit* colliding_unit) { // 0042DEE0
		u32 collider_status = colliding_unit->status;
		if (collider_status & UnitStatus::UNKNOWN5) {
			u8 player = colliding_unit->player;
			if (player == 11) {
				player = colliding_unit->sprite->player;
			}

			if (!bw::are_mutual_allies(unit->player, player)) {
				return false;
			}
		}
		if (unit_paths_over_small_units(unit) || unit_paths_over_small_units(colliding_unit)) {
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
	s32 unit_movement_start_path(Unit* unit) { // 0046BD90
		s32 result = helper::unit_movement_another_path(unit, unit->move_target.pt);
		if (result) {
			u8 action_flags = unit->user_action_flags;
			if (action_flags & UserActionFlags_UMStartPath && (unit->user_action_flags = action_flags & 0xFD, unit->flingy_movement_type == MovementType_Iscript)) {
				if (!unit->path) { // New
					return false;
				}
				unit->path->collision_recheck_timer = 2;
				unit->movement_state = UnitMovement_UIOrderDelay;
				result = true;
			} else {
				unit->movement_state = UnitMovement_FollowPath;
				result = true;
			}
		} else {
			unit->movement_state = UnitMovement_FailedPath;
		}
		return result;
	}

	// Hooked to resolve a nullsprite crash. -Pr0nogo
	b32 move_towards(Unit* target, Unit* unit) { // 004EB900
		if (!AssertSprite(unit->sprite) || !AssertSprite(target->sprite)) { // New
			return 0;
		}
		if (unit->recent_order_timer) {
			return 1;
		}
		if (!((1 << unit->player) & target->sprite->visibility_status) && !helper::is_target_within_min_range(unit, 448, target)) {
			if (helper::is_unit_movable(unit) == 1) {
				helper::to_idle(unit);
				return 0;
			}
			return 1;
		}
		return helper::move_to_target(target, unit);
	}

	// Hooked to troubleshoot nullsprite crashes. -Pr0nogo
	s32 unit_movement_initialize(Unit* unit) { // 0046C320
		Sprite* sprite = unit->sprite;
		u32 pathing_flags = unit->pathing_flags;
		unit->contour_bounds.y = 0;
		unit->contour_bounds.x = 0;
		unit->contour_bounds.h = 0;
		unit->contour_bounds.w = 0;

		u32 is_pathing_enabled = pathing_flags & 0xFC;
		if (sprite) { // Nice meme that lard nullchecks sprite after this.
			is_pathing_enabled = pathing_flags & 0xFC | (sprite->elevation < 12);
		}
		unit->pathing_flags = is_pathing_enabled;

		bool is_subunit = units_dat::base_property[unit->id] & UnitProperty::Subunit;
		if (!is_subunit) {
			if (unit->status & UnitStatus::NoBrkCodeStart) {
				unit->movement_state = UnitMovement_InitSeq;
				return 1;
			}
		}

		if (!sprite || !unit->main_order_id && unit->main_order_state == 1) {
			unit->movement_state = UnitMovement_Lump;
			return 1;
		}
		if (unit->status & UnitStatus::InBuilding) {
			unit->movement_state = UnitMovement_Bunker;
			return 1;
		}

		if (!(sprite->flags & SpriteFlags::Hidden)) {
			if (!(unit->status & UnitStatus::Burrowed)) {
				u32 status = unit->status;
				if ((status >> 17) & 1 || status & UnitStatus::CanTurnAroundToAttack) {
					if ((status >> 17) & 1) {
						unit->movement_state = (is_pathing_enabled & 1) != 0 ? UnitMovement_AtRest : UnitMovement_Flyer;
					} else {
						unit->movement_state = 2 * (is_subunit == 0) + 3;
					}
					return 1;
				} else if (is_pathing_enabled & 1 && (unit->movement_flags & MovementFlags_Accelerating || !helper::is_unit_movable(unit))) {
					unit->movement_state = UnitMovement_LumpWannabe;
					return 1;
				} else {
					unit->movement_state = UnitMovement_Lump;
					return 1;
				}
			}
			unit->movement_state = UnitMovement_Lump;
			return 1;
		}
		if (unit->movement_flags & MovementFlags_Accelerating || !helper::is_unit_movable(unit)) {
			helper::unit_movement_instant_stop(unit);
			helper::progress_flingy(unit);
			helper::finish_unit_movement(unit);
		}
		unit->movement_state = UnitMovement_Hidden;
		return 1;
	}

	// Hooked to troubleshoot issues with moving units a la Anthelion. -Pr0nogo
	void refresh_unit_after_move(Unit* unit) { // 00494160
		Unit* subunit = unit->subunit;
		if (subunit && subunit->is_subunit()) {
			s32 x_halt = unit->halt.x;
			s32 y_halt = unit->halt.y;
			subunit->halt.x = x_halt;
			subunit->halt.y = y_halt;
			subunit->position.x = x_halt >> 8;
			subunit->position.y = y_halt >> 8;
			helper::move_sprite(subunit->sprite, x_halt >> 8, y_halt >> 8);
		}
		
		if (unit->sprite->flags & SpriteFlags::Hidden) {
			unit->sprite->flags &= ~SpriteFlags::Hidden;
			if (subunit && subunit->is_subunit()) {
				subunit->sprite->flags &= ~SpriteFlags::Hidden;
			}
			helper::clist_remove_hidden_unit_entry(unit);
			Unit* visible_unit = g_first_visible_unit;
			if (visible_unit) {
				if (g_last_active_unit == visible_unit) {
					g_last_active_unit = unit;
				}
				Unit* next_visible_unit = visible_unit->link.next;
				unit->link.prev = visible_unit;
				unit->link.next = next_visible_unit;
				if (next_visible_unit) {
					next_visible_unit->link.prev = unit;
				}
				visible_unit->link.next = unit;
			} else {
				g_last_active_unit = unit;
				g_first_visible_unit = unit;
			}
		}

		helper::refresh_unit_vision(unit);
		helper::update_visibility(unit);
		helper::update_unit_order_finder(unit);
		if (unit->status & UnitStatus::GroundedBuilding) {
			helper::set_building_tile_flag(unit, unit->sprite->position.x, unit->sprite->position.y);
		}

		helper::check_unstack(unit);
		if (unit->status & UnitStatus::InAir) {
			helper::set_repulse_angle(unit);
		}
		helper::toggle_unit_path(unit);
		unit->movement_state = UnitMovement_Init;
		u8 pathing_flags = unit->pathing_flags;
		unit->pathing_flags = (pathing_flags ^ (unit->sprite->elevation < 12)) & 1 ^ pathing_flags;
		if (subunit && units_dat::base_property[subunit->id] & UnitProperty::Subunit) {
			helper::toggle_unit_path(subunit);
			subunit->movement_state = 0;
			subunit->pathing_flags ^= (subunit->pathing_flags ^ (subunit->sprite->elevation < 12)) & 1;
		}
	}
}