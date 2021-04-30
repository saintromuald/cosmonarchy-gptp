#pragma once
#include "../SCBW/structures/CUnit.h"

namespace hooks {

CUnit* findBestTerranMineTarget(CUnit* mine);
void orders_TerranMine(CUnit* unit);
void orders_PlaceMine(CUnit* a1);

void injectSpiderMineHooks();

} //hooks
