#pragma once
#include "SCBW/api.h"
namespace hooks {
	void injectMaxHpHooks();
	u32 maxhp(int id, int playerId);
	u32 maxshields(int id, int playerId);
	u32 getTotalHitPointsPlusShields(CUnit* a1);
	void drawUnitInfo(BinDlg* a1);
}