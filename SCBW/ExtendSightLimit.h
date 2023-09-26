/// This header file contains the structures and functions that allow you to
/// change the maximum sight distance limit in StarCraft.

/// To use, add the following code to the game_on() function in hooks/game_hooks.cpp:
///    set_max_sight_range<20>();  //Sets maximum sight distance to 20

#pragma once
#include "../definitions.h"
#include "../hook_tools.h"
#pragma pack(1)

namespace offsets
{
    const u32 SightStructArrayRef   = 0x00480709; // The pointer to the SightStruct array must be injected here
    const u32 InitializeSightValues = 0x0042AD60; // The internal function responsible for setting up the array
}

//Sight distance varies between 0 and MAX_SIGHT_RANGE, inclusive
template <u32 MAX_SIGHT_RANGE>
void set_max_sight_range()
{
    static SightRange sight_data[MAX_SIGHT_RANGE + 1] = {};

    for (u32 x = 0; x <= MAX_SIGHT_RANGE; ++x) {
        sight_data[x].tile_sight_width   = x * 2 + 3;
        sight_data[x].tile_sight_height  = x * 2 + 3;
        sight_data[x].unk_08 = 3;
        sight_data[x].unk_0C = 3;
    }

    memory_patch(offsets::SightStructArrayRef, &sight_data);

    const u16 map_tile_width = g_map_tile_size->width;
    const u32 sight_range_param = MAX_SIGHT_RANGE + 1;
    __asm {
        PUSHAD
        MOVZX EAX, map_tile_width
        PUSH EAX
        MOV EAX, sight_range_param
        LEA ECX, sight_data
        CALL [offsets::InitializeSightValues]
        POPAD
  }
}

#pragma pack()