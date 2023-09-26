#include "ExtendSightLimit.h"

SightRange g_sight_ranges[MAX_SIGHT_RANGE + 1] = {};

void set_max_sight_range() {
    static const u32 SightStructArrayRef   = 0x00480709;
    static const u32 InitializeSightValues = 0x0042AD60;

    for (u32 x = 0; x <= MAX_SIGHT_RANGE; ++x) {
        g_sight_ranges[x].tile_sight_width   = x * 2 + 3;
        g_sight_ranges[x].tile_sight_height  = x * 2 + 3;
        g_sight_ranges[x].field_8 = 3;
        g_sight_ranges[x].field_C = 3;
    }

    memory_patch(SightStructArrayRef, &g_sight_ranges);

    const u16 map_tile_width = g_map_tile_size->width;
    const u32 sight_range_param = MAX_SIGHT_RANGE + 1;
    __asm {
        PUSHAD
        MOVZX EAX, map_tile_width
        PUSH EAX
        MOV EAX, sight_range_param
        LEA ECX, g_sight_ranges
        CALL [InitializeSightValues]
        POPAD
	}
}