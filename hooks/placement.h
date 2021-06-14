#pragma once
#include <SCBW/api.h>

namespace hooks
{
	s32 InitPlacementBoxPos(s32 unused, Point16* placement);	// 0048D660
	u8 DrawPlacementBox(Point16 ecx0);							// 0048DDC0
	s32 updatePlacementStateForPlacementBox();					// 0048DCE0
	s32 updateBuildingPlacementState_Units(s32 yTile, Unit* builder, s32 xTile, u8 player, u16 unitId, s32 sizeWidthHeight, s32 placementStateEntry,
		char bDontIgnoreReacting, char bAlsoInvisible, s32 withoutVision); // 00473720

	void injectPlacementHooks();
}