#pragma once
#include "SCBW/structures.h"

namespace hooks
{

    s32 initGame();							// 004EE110
    s32 startGame(void* a1, s32 a2);		// 00472060

    void injectLobbyHooks();
}