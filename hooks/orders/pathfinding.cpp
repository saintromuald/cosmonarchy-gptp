#include <BW/api.h>
#include "globals.h"
#include <math_utils.h>

namespace hooks
{
    static bool box_aabb(Box32 a, Box32 b) {
        return a.left <= b.right
            && b.left <= a.right
            && a.top  <= b.bottom
            && b.top  <= a.bottom;
    }

    static Box32 get_unit_box(Unit* unit) {
        Box32 box  = {};
        box.left   = unit->get_left()   - 1;
        box.top    = unit->get_top()    - 1;
        box.right  = unit->get_right()  + 1;
        box.bottom = unit->get_bottom() + 1;
        return box;
    }

    static bool unit_aabb(Unit* a, Unit* b) {
        return box_aabb(get_unit_box(a), get_unit_box(b));
    }

    // Hooked to improve functionality -Veeq7
    b32 __fastcall is_point_at_unit_border(u32 unused, Unit* target_unit, Unit* unit, Point16 target_point) { // 0042DA90
        if (unit_aabb(unit, target_unit)) {
            return true;
        }
        return false;
    }

    static Point16 get_target_point(Unit* unit, Unit* target) {
        Point16 position = unit->position;
        Box32 box = get_unit_box(target);
        if (position.x < box.left - units_dat::unit_bounds[unit->id].right) {
            position.x = box.left - units_dat::unit_bounds[unit->id].right;
        }
        if (position.x > box.right + units_dat::unit_bounds[unit->id].left) {
            position.x = box.right + units_dat::unit_bounds[unit->id].left;
        }
        if (position.y < box.top - units_dat::unit_bounds[unit->id].bottom) {
            position.y = box.top - units_dat::unit_bounds[unit->id].bottom;
        }
        if (position.y > box.bottom + units_dat::unit_bounds[unit->id].top) {
            position.y = box.bottom + units_dat::unit_bounds[unit->id].top;
        }
        return position;
    }

    // Hooked to resolve contextual order issues - Pr0nogo
    s32 move_to_target(Unit* target, Unit* unit) { // 004EB720
        Point16 new_target = get_target_point(unit, target);
        if (target == unit->move_target.unit
        && ((unit->move_target.pt.x == new_target.x && unit->move_target.pt.y == new_target.y)
          || helper::is_point_at_unit_border(0, unit->move_target.unit, unit, unit->move_target.pt)))
        {
            unit->status &= ~UnitStatus::Unmovable;
            return true;
        }
        if (unit->unit_ai && unit->main_order_id != OrderId::TransportLoad && helper::ai_prepare_moving_to(unit, new_target.x, new_target.y)) {
            return false;
        }

        UnitPath* path = unit->path;
        if (path) {
            path->is_calculating |= 1;
        }
        if (unit->move_target.pt != new_target || unit->move_target.unit != target) {
            unit->move_target.unit = target;
            unit->move_target.pt = new_target;
            unit->next_movement_waypoint = new_target;
            unit->movement_flags &= ~MovementFlags_Braking;
            unit->movement_flags |= MovementFlags_OrderedAtLeastOnce;
        }
        unit->recent_order_timer = 15;
        unit->status &= ~UnitStatus::Unmovable;
        Order* next_order = unit->order_queue_head;
        if (!next_order || orders_dat::Unknown7[next_order->order_id]) {
            unit->movement_flags &= ~MovementFlags_Lifted;
        } else {
            unit->movement_flags |= MovementFlags_Lifted;
        }
        return true;
    }

    // Hooked because the helper is fucked - Pr0nogo
    b32 get_active_tile_flag(s16 y, s16 x, s32 included_flags, s32 excluded_flags) { // 00472F80
        ActiveTile* tile = bw::get_active_tile_at_coords(x, y);
        return tile->vision_flags & included_flags && !(tile->vision_flags & excluded_flags);
    }

    // Hooked to investigate crashes - Pr0nogo
    bool unit_has_path_to_dest(Unit* unit, s32 x, s32 y) { // 0049CB60
        Point16 target = {x, y};
        return bw::get_region(unit->position)->group_index == bw::get_region(target)->group_index;
    }

    // Hooked to investigate crashes - Pr0nogo
    bool unit_has_path_to_unit(Unit* unit, Unit* target) { // 0049CBB0
        return bw::get_region(unit->position)->group_index == bw::get_region(target->position)->group_index;
    }

    // Hooked to disable functionality. Hook is not used, pending our reimplementation. -Pr0nogo
    void set_repulse_angle(Unit* unit) { // 00453300
        // Empty
    }

    void repulse_unit(Unit* unit, b32 should_repulse) { // 004535A0
        UnitExt* unit_ext = get_unit_ext(unit);
        if (unit->pathing_flags & 1
        ||  unit->status & UnitStatus::CanNotReceiveOrders
        ||  unit->status & UnitStatus::NoBrkCodeStart
        ||  unit->status & UnitStatus::Unmovable
        ||  !(unit->status & UnitStatus::Reacts)
        ||  unit->status & UnitStatus::IsGathering
        ||  units_dat::base_property[unit->id] & UnitProperty::Building
        ||  unit->id == UnitId::ProtossSteward
        ||  unit->move_target.pt.x != unit->position.x
        ||  unit->move_target.pt.y != unit->position.y) {
            return;
        }

        EntryArray units = {};
        Box32 unit_box = get_unit_box(unit);
        u32 dist = 8;
        Box32 search_box = unit_box;
        search_box.x -= dist;
        search_box.y -= dist;
        search_box.w += dist * 2;
        search_box.h += dist * 2;
        unit_search_rect(&units, search_box);

        Vec32f repulsion = {};
        bool collides = false;
        if (unit_ext->timers[TimerUseLastRepulsion]) {
            repulsion = unit_ext->last_repulsion;
            for (u32 i = 0; i < units.count; ++i) {
                Unit* target = units.units[i];
                if (unit == target || target->pathing_flags & 1) {
                    continue;
                }
                if (box_aabb(unit_box, get_unit_box(target))) {
                    collides = true;
                }
            }
        } else {
            for (u32 i = 0; i < units.count; ++i) {
                Unit* target = units.units[i];
                if (unit == target || target->pathing_flags & 1) {
                    continue;
                }
                if (box_aabb(unit_box, get_unit_box(target))) {
                    collides = true;
                }

                Vec32f dir = bw::dir_between(target->position, unit->position);
                if (abs(dir.x) < 0.001 && abs(dir.y) < 0.001) {
                    continue;
                }

                repulsion.x += dir.x;
                repulsion.y += dir.y;
            }

            repulsion = bw::normalize(repulsion);

            if (abs(repulsion.x) < 0.001 && abs(repulsion.y) < 0.001) {
                u32 rand_amount = bw::rand_between(0, 255);
                Vec32 random_repulse = g_angle_distance[rand_amount];
                repulsion.x = random_repulse.x / 256.0;
                repulsion.y = random_repulse.y / 256.0;
            }

            unit_ext->last_repulsion = repulsion;
            unit_ext->timers[TimerUseLastRepulsion] = 24;
        }

        if (collides) {
            Point16 target = {};
            target.x = unit->position.x + repulsion.x * (unit->flingy_top_speed / 256.0 / 2.0);
            target.y = unit->position.y + repulsion.y * (unit->flingy_top_speed / 256.0 / 2.0);
            target = bw::clamp_point(target);

            unit->move_target.pt.x = target.x;
            unit->move_target.pt.y = target.y;
            unit->next_movement_waypoint.x = target.x;
            unit->next_movement_waypoint.y = target.y;
        }
    }
}