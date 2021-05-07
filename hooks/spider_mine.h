#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {
	Unit* findBestTerranMineTarget(Unit* mine);	// 00441270
	void orders_TerranMine(Unit* unit);			// 00463DF0
	void orders_PlaceMine(Unit* a1);				// 00464FD0
	char burrowGeneric(Unit* unit);				// 004E9A30

	void injectSpiderMineHooks();
}