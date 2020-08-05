#pragma once
#include "SCBW/structures.h"

namespace hooks {

	int getResponseSfx(signed int a1, int a2);		// 0048EA20
	int getRandYesSFX(signed int a1);				// 0048EAB0
	void injectSfxHooks();
}