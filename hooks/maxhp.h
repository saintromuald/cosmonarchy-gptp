#pragma once
#include "SCBW/api.h"
namespace hooks {
	void injectMaxHpHooks();
	u32 maxhp(int id, int playerId);
	u32 maxshields(int id, int playerId);
	u32 getTotalHitPointsPlusShields(CUnit* a1);
	void drawUnitInfo(BinDlg* a1);
	void drawSupplyUnitInfo(int a1);						// 00427540
	char drawWireframeShield(int a1, CUnit* a2);			// 004566B0
	bool AI_TargetEnemyProc(CUnit* a1, CUnit* a2);			// 00440A20
}