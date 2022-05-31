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
        for (Unit** it = unit_array; *it; ++it) {
            Unit* other_unit = *it;
            bool needs_detection = false;
            if (builder) {
                if (scbw::is_allied_to(other_unit->player, builder->player)) {
                    continue;
                }
                if (other_unit->status & (UnitStatus::RequiresDetection | UnitStatus::Cloaked)) {
                    if (!(other_unit->visibilityStatus & (1 << builder->player))) {
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

            if (unitOrderingX[other_unit->finderIndex.left  ].position < 32 * x_tile + 32
             && unitOrderingX[other_unit->finderIndex.right ].position > 32 * x_tile
             && unitOrderingY[other_unit->finderIndex.top   ].position < 32 * y_tile + 32
             && unitOrderingY[other_unit->finderIndex.bottom].position > 32 * y_tile) {
                return needs_detection != 0 ? 1 : 4;
            }
        }
        return 0;
    }
}