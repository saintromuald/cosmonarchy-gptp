#include "placement.h"
#include "data_ext.h"

namespace hooks
{
	s32 init_placement_box_pos(s32 unused, Point16* placement_pos) { // 0048D660
        if (placement_pos->x >= 640 || placement_pos->y >= 400) {
            return 0;
        }

        u32 tile_x = (placement_pos->x + 16) / 32;
        u32 tile_y = (placement_pos->y + 16) / 32;

        *placeBuildingX = tile_x + *moveToTileX;
        *placeBuildingY = tile_y + *moveToTileY;
        placement_pos->x = *placeBuildingX * 32 - *moveToX;
        placement_pos->y = *placeBuildingY * 32 - *moveToY;
        return 1;
	}

    u8 draw_placement_box(Point16 unused) { // 0048DDC0
        Point16 placement_pos = unused;
        u8 result = *is_placing_building;
        if (!result) {
            return result;
        }
        unused.x = mouse->y;
        Point16 dims = units_dat::BuildingDimensions[*placeBuildingUnitType];
        placement_pos.x = mouse->x - dims.x / 2;
        placement_pos.y = mouse->y - dims.y / 2;
        if (helper::init_placement_box_pos(&placement_pos)) {
            *placeBuildingMessageId = helper::update_placement_state_for_placement_box();
            helper::refresh_layers_3_and_4();
            helper::unk_building_placement_related(1, &placement_pos);
            if (*isPlacingAddon) {
                placement_pos.x -= units_dat::AddonPlacement[*placeBuildingUnitType - UnitId::TerranCommandCenter].x;
                placement_pos.y -= units_dat::AddonPlacement[*placeBuildingUnitType - UnitId::TerranCommandCenter].y;
                helper::unk_building_placement_related(0, &placement_pos);
            }
            return helper::refresh_layers_3_and_4();
        }
        helper::unk_building_placement_related_mouse(0, 1);
        return helper::unk_building_placement_related_mouse(0, 0);
    }

    s32 update_placement_state_for_placement_box() // 0048DCE0
    {
        if (*active_portrait_unit == nullptr)
        {
            return 0;
        }

        u32 source = *placeBuildBaseType;
        u32 target = *placeBuildingUnitType;
        u16 addon_x = (*placeBuildingX - units_dat::AddonPlacement[2 * *placeBuildingUnitType - 212].x / 32);
        u16 addon_y = (*placeBuildingY - units_dat::AddonPlacement[2 * *placeBuildingUnitType - 211].y / 32);
        scbw::adjust_addon_coordinates(source, target, &addon_x, &addon_y);

        s32 can_build = helper::can_build_on_tile_location(*active_portrait_unit, *placingPlayer, *placeBuildingX, *placeBuildingY, *placeBuildingUnitType, 1, 1, 0, 0);
        s32 result = helper::can_build_on_tile_location(*active_portrait_unit, *placingPlayer, addon_x, addon_y, *placeBuildBaseType, 0, 1, 0, 0);
        // ^ this call might be needed, even if the result isn't returned, untested
        if (!*isPlacingAddon || can_build)
        {
            result = can_build;
        }
        return result;
    }

	// Hooked to enable unit mover for human players. - Pr0nogo
    s32 is_tile_blocked_by(Unit** unit_array, Unit* builder, s32 x_tile, s32 y_tile, s8 dont_ignore_reacting, s8 also_invisible) // 00473300
    {
        for (Unit** it = unit_array; *it; ++it) {
            Unit* other_unit = *it;
            bool needs_detection = false;
            if (builder) {
                if (scbw::is_allied_to(other_unit->player, builder->player)) {
                    continue;
                }
                if (other_unit->status & (UnitStatus::RequiresDetection | UnitStatus::Cloaked)) {
                    if (!(other_unit->visibility_status & (1 << builder->player))) {
                        needs_detection = true;
                    }
                }
            }
            if ((!dont_ignore_reacting && other_unit->status & UnitStatus::IsBuilding)
             || (needs_detection && !also_invisible)
             || other_unit == builder
             || other_unit->status & (UnitStatus::GroundedBuilding | UnitStatus::InAir)
             || other_unit->id == UnitId::Spell_DarkSwarm
             || other_unit->id == UnitId::Spell_DisruptionWeb
             || other_unit->sprite->flags & SpriteFlags::Hidden) {
                continue;
            }

            if (unitOrderingX[other_unit->finder_index.left  ].position < 32 * x_tile + 32
             && unitOrderingX[other_unit->finder_index.right ].position > 32 * x_tile
             && unitOrderingY[other_unit->finder_index.top   ].position < 32 * y_tile + 32
             && unitOrderingY[other_unit->finder_index.bottom].position > 32 * y_tile) {
                return needs_detection ? 1 : 4;
            }
        }
        return 0;
    }

    // Hooked to allow building on allied creep. -Pr0nogo
    int update_creep_building_placement_state(u8 player, u16 x_tile, u16 y_tile, Point16 size, u32 placement_state_entry, u32 check_temporary_creep, u32 check_creep) // 00473010
    {
        u32 human_only_player_bit = 0;
        if (playerTable[player].type == PlayerType::Human) {
            human_only_player_bit = 1 << player;
        }

        u8 result = 0;
        for (u32 y = 0; y < size.y; ++y) {
            for (u32 x = 0; x < size.x; ++x) {
                ActiveTile* tile = scbw::get_active_tile_at_tile(x_tile + x, y_tile + y);

                char state = 0;
                if (tile->explored_flags & human_only_player_bit) {
                    state  = 10;
                    result = 10;
                }
                bool creep = !tile->has_creep;
                if (check_temporary_creep) {
                    creep  = !tile->temporary_creep;
                }
                if ((check_temporary_creep || check_creep || !(tile->visibility_flags & human_only_player_bit)) && creep) {
                    state = 5;
                    if (result < 5) {
                        result = 5;
                    }
                }
                buildingPlacementState[6 * (y + 8 * placement_state_entry) + x] = state;
            }
        }
        return result;
    }

    // Same as above. -Pr0nogo
    u32 update_building_placement_state_map_tile_flags(u16 unit_id, u16 tile_x, u16 tile_y, u8 player, Point16 size, u32 placement_state_entry, u32 check_creep) // 00473A10
    {
        u32 human_only_player_bit = 0;
        if (playerTable[player].type == PlayerType::Human) {
            human_only_player_bit = 1 << player;
        }

        u8 result = 0;
        for (u32 y = 0; y < size.y; ++y) {
            for (u32 x = 0; x < size.x; ++x) {
                ActiveTile* tile = scbw::get_active_tile_at_tile(tile_x + x, tile_y + y);
                TileExt* tile_ext = get_tile_ext_at_tile(tile_x + x, tile_y + y);

                u32 flags = 0x20800000; // cliff_edge, always_unbuildable
                if (!units_dat::GroupFlags[unit_id].is_zerg && !can_place_on_creep_tile(tile_ext, player)) {
                    flags |= check_creep ? 0x40000000 : 0x400000;
                }

                u8 state = 0;
                if (tile->explored_flags & human_only_player_bit) {
                    //state = 10;

                    if (units_dat::BaseProperty[unit_id] & UnitProperty::ResourceDepot && helper::find_blocking_fow_resource(x + tile_x, y + tile_y, 3)) {
                        state = 2;
                    }
                } else if (!check_creep && tile->visibility_flags & human_only_player_bit
                || !(tile->value & flags)
                || tile->value < 0) { // originally also checked for if the unit id was any racial beacon
                    if (units_dat::BaseProperty[unit_id] & UnitProperty::ResourceDepot && helper::place_building_resource_restriction_check(check_creep, x + tile_x, y + tile_y)) {
                        state = 2;
                    }
                } else {
                    state = 4;
                }
                buildingPlacementState[6 * (y + (8 * placement_state_entry)) + x] = state;
                if (result < state) {
                    result = state;
                }
            }
        }
        return result;
    }
}