#pragma once
#include "../definitions.h"
#include "../hook_tools.h"

#define MAX_SIGHT_RANGE 20

#pragma pack(1)
struct SightStruct {
    u32 tileSightWidth;
    u32 tileSightHeight;
    u32 unknown1;
    u32 unknown2;
    u32 unknown3;
    u32 unknown4;
    u32 unknown5;
};
#pragma pack()
C_ASSERT(sizeof(SightStruct) == 28);

extern SightStruct g_sight_ranges[MAX_SIGHT_RANGE + 1];

void set_max_sight_range();

