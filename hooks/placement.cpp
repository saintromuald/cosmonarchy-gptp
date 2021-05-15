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
                s32 tmp = (4 * *placeBuildingUnitType - 424);
                v4 = *(Point16*)&tmp;
                v5 = *(s16*)((u8*)&units_dat::AddonPlacement[1] + *(u32*)&v6);
                v6.x -= *(s16*)((u8*)units_dat::AddonPlacement + *(u32*)&v6);
                v6.y -= v5;
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
}