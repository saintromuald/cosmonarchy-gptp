#include <SCBW/api.h>
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
        box.right  = unit->get_right()  + 2;
        box.bottom = unit->get_bottom() + 2;
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
        if (position.x < box.left) {
            position.x = box.left;
        }
        if (position.x > box.right) {
            position.x = box.right;
        }
        if (position.y < box.top) {
            position.y = box.top;
        }
        if (position.y > box.bottom) {
            position.y = box.bottom;
        }
        return position;
    }

    // Hooked to resolve contextual order issues - Pr0nogo
    s32 move_to_target(Unit* target, Unit* unit) { // 004EB720
        Point16 new_target = get_target_point(unit, target);
        if (target == unit->move_target.unit
        && ((unit->position.x == new_target.x && unit->position.y == new_target.y)
            || helper::isPointAtUnitBorder(0, unit->move_target.unit, unit, unit->move_target.pt))) {
            unit->status &= ~UnitStatus::Unmovable;
            return true;
        }
        if (unit->unit_ai
        &&  unit->main_order_id != OrderId::Pickup4
        &&  helper::ai_prepare_moving_to(unit, target->position.x, target->position.y)) {
            return false;
        }

        UnitPath* path = (UnitPath*)unit->path;
        if (path) {
            path->is_calculating |= 1;
        }
        if (new_target.x != unit->move_target.pt.x
        ||  new_target.y != unit->move_target.pt.y
        ||  target != unit->move_target.unit) {
            unit->move_target.unit = target;
            unit->move_target.pt = new_target;
            unit->next_movement_waypoint = new_target;
            unit->movement_flags &= ~4;
            unit->movement_flags |= 1;
        }
        unit->recent_order_timer = 15;
        unit->status &= ~UnitStatus::Unmovable;
        Order* unit_order = unit->order_queue_head;
        if (!unit_order || orders_dat::button_icon[unit_order->order_id]) {
            unit->movement_flags &= ~0x20;
        } else {
            unit->movement_flags |= 0x20;
        }
        return true;
    }

    // Hooked because the helper is fucked - Pr0nogo
    b32 get_active_tile_flag(s16 y, s16 x, s32 included_flags, s32 excluded_flags) { // 00472F80
        ActiveTile* tile = scbw::get_active_tile_at_coords(x, y);
        return tile->value & included_flags && !(tile->value & excluded_flags);
    }

    // Hooked to investigate crashes - Pr0nogo
    bool unit_has_path_to_dest(Unit* unit, s32 x, s32 y) // 0049CB60
    {
        if (!unit->sprite)
        {
            return false;
        }
        RegionArray* paths = *g_sai_paths;
        s32 regionId = helper::get_region_id(unit->sprite->position.x, unit->sprite->position.y) << 6;
        return paths->regions[helper::get_region_id(x, y)].group_index == *(s16*)((char*)&paths->regions[0].group_index + regionId);
    }

    // Hooked to investigate crashes - Pr0nogo
    bool unit_has_path_to_unit(Unit* pather, Unit* target) // 0049CBB0
    {
        if (!pather->sprite)
        {
            return false;
        }
        Sprite* patherSprite = pather->sprite;
        RegionArray* paths = *g_sai_paths;
        s32 regionId = helper::get_region_id(target->sprite->position.x, target->sprite->position.y) << 6;
        return *(s16*)((char*)&paths->regions[0].group_index + regionId) == paths->regions[helper::get_region_id(patherSprite->position.x, patherSprite->position.y)].group_index;
    }

    // Hooked to OoooooO. -Pr0nogo
    void set_repulse_angle(Unit* unit) { // 00453300
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

        Vector32f repulsion = {};
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

                Vector32f diff = {};
                diff = scbw::get_normalized_vector(scbw::get_vector_between_points(target->position, unit->position));
                if (abs(diff.x) < 0.001 && abs(diff.y) < 0.001) {
                    continue;
                }

                repulsion.x += diff.x;
                repulsion.y += diff.y;
            }

            repulsion = scbw::get_normalized_vector(repulsion);

            if (abs(repulsion.x) < 0.001 && abs(repulsion.y) < 0.001) {
                Vector32 random_repulse = g_angle_distance[scbw::rand_between(0, 255)];
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
            target = scbw::clamp_point(target);

            unit->move_target.pt.x = target.x;
            unit->move_target.pt.y = target.y;
            unit->next_movement_waypoint.x = target.x;
            unit->next_movement_waypoint.y = target.y;
        }
    }
}