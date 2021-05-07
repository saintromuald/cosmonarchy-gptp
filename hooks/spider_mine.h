#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {
	CUnit* findBestTerranMineTarget(CUnit* mine);	// 00441270
	void orders_TerranMine(CUnit* unit);			// 00463DF0
	void orders_PlaceMine(CUnit* a1);				// 00464FD0
	char burrowGeneric(CUnit* unit);				// 004E9A30

	void injectSpiderMineHooks();
}