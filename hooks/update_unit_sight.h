#pragma once
#include <SCBW/api.h>
#include <SCBW/extender/TileHeight_extender.h>
#include <SCBW/scbwdata.h>
#include <SCBW/structures.h>
#include <SCBW/structures/Unit.h>

extern u8 vision_timers[8][256 * 256];

namespace hooks {
	void __fastcall gndSightCB_Edge(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* maskData, u32 relFlags, u32 newVisionFlags);
	void __fastcall gndSightCB_Inside(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* maskData, u32 relFlags, u32 newVisionFlags);
	void __fastcall airSightCB_Edge(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* maskData, u32 relFlags, u32 newVisionFlags);
	void __fastcall airSightCB_Inside(u32 tileX, u32 tileY, SightRange* sightVals, ActiveTile* maskData, u32 relFlags, u32 newVisionFlags);
	void refresh_vision_timers(u32 testTilePos, u8 visionFlags);
	void update_fog();
	void reveal_sight_at_location(u32 sightRange, u32 visionFlags, s32 ptX, s32 ptY, bool isAirUnit);
	s32 get_ground_height_at_px(s32 offsetY, s32 offsetX);

	void inject_sight_update_hooks();
}