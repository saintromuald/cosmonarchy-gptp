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
        u8 result;
        Point16 v4;
        Point16 v6;
        s16 v5;

        v6 = ecx0;
        result = *IS_PLACING_BUILDING;
        if (!*IS_PLACING_BUILDING)
        {
            return result;
        }
        ecx0.x = mouse->y;
        v6.x = mouse->x;
        v6.y = mouse->y;
        Point16 dims = units_dat::BuildingDimensions[*placeBuildingUnitType];
        v6.x -= dims.x / 2;
        v6.y -= dims.y / 2;
        v6.x += 24;
        v6.y += 24;
        if (helper::initPlacementBoxPos(&v6))
        {
            *placeBuildingMessageId = helper::updatePlacementStateForPlacementBox_Func();
            helper::refresh_layers_3_and_4();
            helper::unk_buildingPlacementRelated(1, &v6);
            if (*isPlacingAddon)
            {
                v6.x -= units_dat::AddonPlacement[*placeBuildingUnitType - UnitId::TerranCommandCenter].x;
                v6.y -= units_dat::AddonPlacement[*placeBuildingUnitType - UnitId::TerranCommandCenter].y;
                helper::unk_buildingPlacementRelated(0, &v6);
            }
            result = helper::refresh_layers_3_and_4();
        }
        else
        {
            helper::unk_buildingPlacementRelatedMouse(0, 1);
            result = helper::unk_buildingPlacementRelatedMouse(0, 0);
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
        scbw::adjustAddonCoordinates(source, target, &addon_x, &addon_y);

        s32 can_build = helper::canBuildOnTileLocation(*activePortraitUnit, *placingPlayer, *placeBuildingX, *placeBuildingY, *placeBuildingUnitType, 1, 1, 0, 0);
        s32 result = helper::canBuildOnTileLocation(*activePortraitUnit, *placingPlayer, addon_x, addon_y, *placeBuildBaseType, 0, 1, 0, 0);
        // ^ this call might be needed, even if the result isn't returned, untested
        if (!*isPlacingAddon || can_build)
        {
            result = can_build;
        }
        return result;
    }

	// Hooked to enable unit mover for human players - Pr0nogo


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
                if (scbw::isAlliedTo(other_unit->player, builder->player))
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
}