#pragma once
#include <SCBW/api.h>

namespace hooks
{
	s32 InitPlacementBoxPos(s32 unused, Point16* placement);	// 0048D660
	u8 DrawPlacementBox(Point16 ecx0);							// 0048DDC0
	s32 updatePlacementStateForPlacementBox();					// 0048DCE0

	void injectPlacementHooks();
}