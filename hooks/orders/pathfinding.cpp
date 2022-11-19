#include <SCBW/api.h>
#include "globals.h"
#include <math_utils.h>

namespace hooks
{
    static Box16 unit_get_box(Unit* unit) {
        Box16 box  = {};
        box.left   = unit->position.x - units_dat::unit_bounds[unit->id].left   - 1;
        box.top    = unit->position.y - units_dat::unit_bounds[unit->id].top    - 1;
        box.right  = unit->position.x + units_dat::unit_bounds[unit->id].right  + 2;
        box.bottom = unit->position.y + units_dat::unit_bounds[unit->id].bottom + 2;
        return box;
    }

    static bool unit_aabb(Unit* a, Unit* b) {
        Box16 box_a = unit_get_box(a);
        Box16 box_b = unit_get_box(b);
        return box_a.left <= box_b.right
            && box_b.left <= box_a.right
            && box_a.top  <= box_b.bottom
            && box_b.top  <= box_a.bottom;
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
        Box16 box = unit_get_box(target);
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
    b32 get_active_tile_flag(s16 y, s16 x, s32 includedFlags, s32 excludedFlags) // 00472F80
    {
        ActiveTile tile = scbw::get_stupid_reference_to_active_tile_at(x, y);
        return tile.value & includedFlags && !(tile.value & excludedFlags);
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
    void set_repulse_angle(Unit* unit) // 00453300
    {
        return;
        if (unit->status & UnitStatus::Reacts)
        {
            if (!(units_dat::base_property[unit->id] & UnitProperty::Building) && unit->id != UnitId::ProtossSteward)
            {
                if (!(unit->repulse_unk & 0xF0))
                {
                    u32 random = helper::randomize_short(37);
                    unit->repulse_angle = random;
                    s32 aids = (random >> 8) & 0xF0;
                    if (!aids)
                    {
                        aids = -16;
                    }
                    unit->repulse_unk |= aids;
                }
                unit->repulse_unk &= 0xF8u;
                Sprite* sprite = unit->sprite;
                unit->drift_pos.x = (char)sprite->position.x / 48;
                char pos_y_post = (char)sprite->position.y / 48;
                char* handle = (char*)g_repulse_handle;
                unit->drift_pos.y = pos_y_post;
                char* CUCKDEATH = &handle[171 * pos_y_post + (u8)unit->drift_pos.x];
                if ((u8)*CUCKDEATH < 0xFFu)
                {
                    ++* CUCKDEATH;
                }
            }
        }
    }
}