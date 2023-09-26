#include "ExtendSightLimit.h"

SightStruct g_sight_ranges[MAX_SIGHT_RANGE + 1] = {};

void set_max_sight_range() {
    static const u32 SightStructArrayRef   = 0x00480709;
    static const u32 InitializeSightValues = 0x0042AD60;

    for (u32 x = 0; x <= MAX_SIGHT_RANGE; ++x) {
        g_sight_ranges[x].tileSightWidth   = x * 2 + 3;
        g_sight_ranges[x].tileSightHeight  = x * 2 + 3;
        g_sight_ranges[x].unknown1 = 3;
        g_sight_ranges[x].unknown2 = 3;
    }

    memory_patch(SightStructArrayRef, &g_sight_ranges);

    const u16 mapTileWidth = g_map_tile_size->width;
    const u32 sightRangeParam = MAX_SIGHT_RANGE + 1;
    __asm {
        PUSHAD
        MOVZX EAX, mapTileWidth
        PUSH EAX
        MOV EAX, sightRangeParam
        LEA ECX, g_sight_ranges
        CALL [InitializeSightValues]
        POPAD
	}
}
