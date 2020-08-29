#pragma once
#include "SCBW/structures.h"

namespace hooks {

	int getResponseSfx(s32 a1, int a2);		// 0048EA20
	int getRandYesSFX(s32 a1);				// 0048EAB0
	void injectSfxHooks();
}