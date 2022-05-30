#include "placement.h"

namespace hooks
{
	s32 init_placement_box_pos(s32 unused, Point16* placement) // 0048D660
	{
        u16 x = placement->x;
        u16 y = placement->y;
        if (placement->x >= 640u || y >= 400u)
        {
            return 0;
        }
        s16 tempY = *moveToTileY + (y >> 5);
        *placeBuildingY = tempY;
        *placeBuildingX = *moveToTileX + (x >> 5);
        placement->x = 32 * (*moveToTileX + (x >> 5)) - *moveToX;
        placement->y = 32 * tempY - *moveToY;
        if ( placement->x < 0 )
        {
            *placeBuildingX = *placeBuildingX + 1;
            placement->x = placement->x + 32;
        }
        if (placement->y >= 0)
        {
            return 1;
        }
        placement->y = placement->y + 32;
        *placeBuildingY = tempY + 1;
        return 1;
	}

    u8 draw_placement_box(Point16 ecx0) // 0048DDC0
    {
        Point16 placement_box = ecx0;
        u8 result = *IS_PLACING_BUILDING;
        if (!result)
        {
            return result;
        }
        ecx0.x = mouse->y;
        placement_box.x = mouse->x;
        placement_box.y = mouse->y;
        Point16 dims = units_dat::BuildingDimensions[*placeBuildingUnitType];
        placement_box.x -= dims.x / 2;
        placement_box.y -= dims.y / 2;
        placement_box.x += 24;
        placement_box.y += 24;
        if (helper::init_placement_box_pos(&placement_box))
        {
            *placeBuildingMessageId = helper::update_placement_state_for_placement_box();
            helper::refresh_layers_3_and_4();
            helper::unk_building_placement_related(1, &placement_box);
            if (*isPlacingAddon)
            {
                placement_box.x -= units_dat::AddonPlacement[*placeBuildingUnitType - UnitId::TerranCommandCenter].x;
                placement_box.y -= units_dat::AddonPlacement[*placeBuildingUnitType - UnitId::TerranCommandCenter].y;
                helper::unk_building_placement_related(0, &placement_box);
            }
            result = helper::refresh_layers_3_and_4();
        }
        else
        {
            helper::unk_building_placement_related_mouse(0, 1);
            result = helper::unk_building_placement_related_mouse(0, 0);
        }
        return result;
    }

    s32 update_placement_state_for_placement_box() // 0048DCE0
    {
        if (*activePortraitUnit == nullptr)
        {
            return 0;
        }

        u32 source = *placeBuildBaseType;
        u32 target = *placeBuildingUnitType;
        u16 addon_x = (*placeBuildingX - units_dat::AddonPlacement[2 * *placeBuildingUnitType - 212].x / 32);
        u16 addon_y = (*placeBuildingY - units_dat::AddonPlacement[2 * *placeBuildingUnitType - 211].y / 32);
        scbw::adjust_addon_coordinates(source, target, &addon_x, &addon_y);

        s32 can_build = helper::can_build_on_tile_location(*activePortraitUnit, *placingPlayer, *placeBuildingX, *placeBuildingY, *placeBuildingUnitType, 1, 1, 0, 0);
        s32 result = helper::can_build_on_tile_location(*activePortraitUnit, *placingPlayer, addon_x, addon_y, *placeBuildBaseType, 0, 1, 0, 0);
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
        return 0;
        for (Unit** it = unit_array; *it; ++it)
        {
            Unit* other_unit = *it;
            bool needs_detection = false;
            if (builder)
            {
                if (other_unit->status & (UnitStatus::RequiresDetection | UnitStatus::Cloaked))
                {
                    if (!(other_unit->visibilityStatus & (1 << builder->player)))
                    {
                        needs_detection = true;
                    }
                }
                if (scbw::is_allied_to(other_unit->player, builder->player))
                {
                    return 0;
                }
            }
            if ((!dont_ignore_reacting && other_unit->status & UnitStatus::IsBuilding)
                || (needs_detection && !also_invisible)
                || other_unit == builder
                || other_unit->status & (UnitStatus::GroundedBuilding | UnitStatus::InAir)
                || other_unit->id == UnitId::Spell_DarkSwarm
                || other_unit->id == UnitId::Spell_DisruptionWeb
                || other_unit->sprite->flags & SpriteFlags::Hidden)
            {
                continue;
            }

            if (unitOrderingX[other_unit->finderIndex.left].position < 32 * x_tile + 32
                && unitOrderingX[other_unit->finderIndex.right].position > 32 * x_tile
                && unitOrderingY[other_unit->finderIndex.top].position < 32 * y_tile + 32
                && unitOrderingY[other_unit->finderIndex.bottom].position > 32 * y_tile)
            {
                return needs_detection != 0 ? 1 : 4;
            }
        }
        return 0;
    }

    // Same as above. -Pr0nogo
    s32 update_building_placement_state_units(s32 y_tile, Unit* builder, s32 x_tile, u8 player, u16 unit_id, Point16 size_wh, s32 placement_state_entry, char dont_ignore_reacting, char also_invisible, s32 without_vision) { // 00473720
        u32 player_shifted = 0;
        if (playerTable[player].type == 2)
        {
            player_shifted = 1 << player;
        }

        // This section is a mess
        u16 x_tile_var = x_tile;
        u16 y_tile_var = y_tile;
        Point16 x_dims = {};
        x_dims.x = units_dat::BuildingDimensions[unit_id].x;
        u32 x_bounds = 32 * x_tile + ((x_dims.x - x_dims.y) >> 1);
        x_dims.y = 32 * y_tile_var + units_dat::BuildingDimensions[unit_id].y / 2;
        Box16 bounds = {};
        bounds.x = x_bounds - units_dat::UnitBounds[unit_id].x;
        u16 x_minus_y = x_dims.x - units_dat::UnitBounds[unit_id].y; // unused?
        u16 x_plus_h = x_dims.x + units_dat::UnitBounds[unit_id].h;
        Box16 bounds_2 = {};
        bounds_2.x = x_bounds + units_dat::UnitBounds[unit_id].w;
        bounds_2.y = x_plus_h;
        Unit** units_in_bounds = helper::get_all_units_in_bounds(&bounds); // no bounds.y?

        u32* active_tile_handle = activeTileHandle[x_tile + y_tile * mapTileSize->width];
        u8 result_1 = 0;
        if (size_wh.y > 0)
        {
            u32 size_wh_iterated = size_wh.x;
            u32 secondary_iterator = 0;
            u32 tile_op_1 = 4 * (mapTileSize->width - size_wh.x);
            u32 tile_op_2 = 4 * (mapTileSize->width - size_wh.x);

            u32 x_tile_iterated = x_tile_var;
            u32 y_tile_iterated = y_tile_var;
            u32 primary_iterator = 0;
            u32 tertiary_iterator = 0;
            do
            {
                tertiary_iterator = 0;
                x_tile_iterated = x_tile_var;
                if (size_wh_iterated > 0)
                {
                    u32 cucked_ida_math = 6 * (secondary_iterator + 8 * placement_state_entry);
                    u32 cucked_ida_math_2 = cucked_ida_math;
                    u32 times_iterated = 0;
                    u8 result_2 = 0;
                    do
                    {
                        result_2 = 0;
                        if (!buildingPlacementState[cucked_ida_math + times_iterated]
                            && (without_vision || !(player_shifted & *active_tile_handle)))
                        {
                            if (*active_tile_handle & 0x8000000 && helper::does_building_block(builder, x_tile_iterated, y_tile_iterated))
                            {
                                result_2 = 4;
                            }
                            else
                            {
                                if (helper::is_tile_blocked_by(units_in_bounds, builder, x_tile_iterated, y_tile_iterated, dont_ignore_reacting, also_invisible))
                                {
                                    result_2 = 1; // originally took result of is_tile_blocked_by
                                }
                            }
                            if (result_1 <= result_2)
                            {
                                result_1 = result_2;
                            }
                            cucked_ida_math = cucked_ida_math_2;
                            buildingPlacementState[cucked_ida_math_2 + times_iterated] = result_2;
                        }
                        ++active_tile_handle;
                        times_iterated = ++tertiary_iterator;
                        size_wh_iterated = size_wh.x;
                        ++x_tile_iterated;
                    } while (tertiary_iterator < size_wh.x);
                    x_tile_var = x_tile;
                    tile_op_1 = tile_op_2;
                }
                active_tile_handle = (active_tile_handle + tile_op_1);
                secondary_iterator = primary_iterator + 1;
                primary_iterator = secondary_iterator;
                ++y_tile_iterated;
                secondary_iterator = secondary_iterator;
            } while (secondary_iterator < size_wh.y);
        }
        *tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsListLastIndex[tempUnitsListArraysCountsListLastIndex[0]--];
        return result_1;
    }

}