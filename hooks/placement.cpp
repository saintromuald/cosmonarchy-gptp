#include "placement.h"

namespace hooks
{
	s32 InitPlacementBoxPos(s32 unused, Point16* placement) // 0048D660
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

    u8 DrawPlacementBox(Point16 ecx0) // 0048DDC0
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
            helper::refreshLayers3And4();
            helper::unk_buildingPlacementRelated(1, &v6);
            if (*isPlacingAddon)
            {
                v6.x -= units_dat::AddonPlacement[*placeBuildingUnitType - UnitId::TerranCommandCenter].x;
                v6.y -= units_dat::AddonPlacement[*placeBuildingUnitType - UnitId::TerranCommandCenter].y;
                helper::unk_buildingPlacementRelated(0, &v6);
            }
            result = helper::refreshLayers3And4();
        }
        else
        {
            helper::unk_buildingPlacementRelatedMouse(0, 1);
            result = helper::unk_buildingPlacementRelatedMouse(0, 0);
        }
        return result;
    }

    s32 updatePlacementStateForPlacementBox() // 0048DCE0
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
	s32 updateBuildingPlacementState_Units(s32 yTile, Unit* builder, s32 xTile, u8 player, u16 unitId, s32 sizeWidthHeight, s32 placementStateEntry,
		char bDontIgnoreReacting, char bAlsoInvisible, s32 withoutVision) // 00473720
	{
		s32 v14;
		s16 v15;
		s32 sizeWH;
		s32 v17;
		Point16 v18;
		s32 v20;
		s32 v21;
		u8 v22;
		u8 v24;
		s16 a1;
		Point16 v28;
		s32 v32;
		s32 v37;

		s32 result = 0;
		ActiveTile** activeTile = &activeTileArray[xTile + yTile * mapTileSize->width];
		s32 humanVisionSomething = 0;
		if (playerTable[player].type == PlayerType::Human)
		{
			humanVisionSomething = 1 << player;
		}
		Point32 dimensions;
		dimensions.x = units_dat::BuildingDimensions[unitId].x;
		dimensions.y = 32 * yTile + units_dat::BuildingDimensions[unitId].y / 2;
		v14 = 32 * xTile + dimensions.x;
		a1 = v14 - units_dat::UnitBounds[unitId].x;
		v28 = dimensions.x - units_dat::UnitBounds[unitId].y;
		v15 = dimensions.x + units_dat::UnitBounds[unitId].h;
		v28.x = v14 + units_dat::UnitBounds[unitId].w;
		v28.y = v15;
		Unit** allUnitsInBounds = helper::getAllUnitsInBounds((Box16*)&a1);
		s32 iterateOffset = 0;
		s16 xTileIterate;
		s32 yTileIterate = yTile;
		if (sizeWidthHeight > 0) // SHIWORD(sizeWidthHeight)
		{
			sizeWH = (s16)sizeWidthHeight;
			v17 = 0;
			v18.x = mapTileSize->width;
			v18.y = sizeWidthHeight;
			v28.x = mapTileSize->width;
			v28.y = sizeWidthHeight;
			do
			{
				v32 = 0;
				xTileIterate = xTile;
				if (sizeWH > 0)
				{
					v20 = 6 * (v17 + 8 * placementStateEntry);
					v37 = v20;
					v21 = 0;
					do
					{
						v22 = 0;
						if (!buildingPlacementState[v20 + v21] && (withoutVision || !(humanVisionSomething & (*activeTile)->visibilityFlags)))
						{
							if ((*activeTile)->isWalkable & 0x8000000 && helper::doesBuildingBlock(builder, xTileIterate, yTileIterate))
							{
								v22 = 4;
							}
							else
							{
								v24 = helper::isTileBlockedBy(allUnitsInBounds, builder, xTileIterate, yTileIterate, bDontIgnoreReacting, bAlsoInvisible);
								if (v24)
								{
									v22 = v24;
								}
							}
							if (result <= v22)
							{
								result = v22;
							}
							v20 = v37;
							buildingPlacementState[v37 + v21] = v22;
						}
						++activeTile;
						v21 = (u16)++v32;
						sizeWH = (s16)sizeWidthHeight;
						++xTileIterate;
					} while ((u16)v32 < (s32)(s16)sizeWidthHeight);
					v18 = v28;
				}
				activeTile = activeTile + v18.x;
				v17 = iterateOffset + 1;
				iterateOffset = v17;
				++yTileIterate;
			} while ((u16)v17 < (s32)sizeWidthHeight); // SHIWORD(sizeWidthHeight)
		}
		*tempUnitsListCurrentArrayCount = tempUnitsListArraysCountsListLastIndex[tempUnitsListArraysCountsListLastIndex[0]--];
		return result;
	}
}