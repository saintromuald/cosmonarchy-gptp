#pragma once
#include "SCBW/api.h"
namespace hooks {
	void injectMaxHpHooks();
	uint32 maxhp(int id, int playerId);
	uint32 maxshields(int id, int playerId);
	uint32 getTotalHitPointsPlusShields(CUnit* a1);
	void drawUnitInfo(BinDlg* a1);
	void drawSupplyUnitInfo(int a1);																		// 00427540
	bool AI_TargetEnemyProc(CUnit* a1, CUnit* a2);															// 00440A20
	void drawBuildingInfo(void* dlg);																		// 00427890
	int calcUnitStrength(int a1, uint8 a2);																	// 00431270
	uint32 getUnitStrengthHook(CUnit* a1, int a2, bool a3);														// 0043183F
	bool AI_RestorationRequirementsProc(CUnit* a1, CUnit* a2);												// 004407E0
	bool AI_EMPShieldRequirementsProc(CUnit* a1, CUnit* a2);												// 004413D0
	char attackOverlayAndNotify(CUnit* a1, CUnit* a2, uint8 a3, char a4);										// 00479730
	void drawImageHPBar(int screenX, int screenY, GrpFrame* pFrame, Box32* grpRect, int colorData);			// 0047A820
	void setBuildShieldGain(CUnit* a1);																		// 0047B6A0
	int getUnitBulletDamage(CUnit* victim, CBullet* bullet);												// 0048ACD0
	int32 canCastSpell(CUnit* a1);																			// 00492140
	int32 modifyUnitShields(int a1, int a2);																	// 004C5A20
	void compileHealthBar(int a1, int a2);																	// 004D6010
	int32 AI_FindTransport(CUnit* a1, int a2);																// 004E6EF0
	int32 mapDataTransfer(char a1, int a2, int a3);															// 004EAFE0
}