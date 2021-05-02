#pragma once
#include "SCBW/structures.h"

namespace hooks
{

    s32 getResponseSfx(s32 a1, s32 a2);		// 0048EA20
    s32 getRandYesSFX(s32 a1);				// 0048EAB0
    void injectSfxHooks();
}