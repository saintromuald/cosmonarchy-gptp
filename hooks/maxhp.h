#pragma once
#include "SCBW/api.h"
namespace hooks
{
	void inject_max_hp_hooks();
	u32 maxhp(s32 id, s32 player);
	u32 maxshields(s32 id, s32 player);
	u32 get_total_hp_plus_shields(Unit* a1);
	void drawUnitInfo(BinDlg* a1);
	void draw_supply_unit_info(s32 a1);																		// 00427540
	bool AI_TargetEnemyProc(Unit* a1, Unit* a2);															// 00440A20
	s32 calcUnitStrength(s32 a1, u8 a2);																	// 00431270
	u32 getUnitStrengthHook(Unit* a1, s32 a2, bool a3);													// 0043183F
	bool AI_RestorationRequirementsProc(Unit* a1, Unit* a2);												// 004407E0
	bool AI_EMPShieldRequirementsProc(Unit* a1, Unit* a2);												// 004413D0
	char attackOverlayAndNotify(Unit* a1, Unit* a2, u8 a3, char a4);										// 00479730
	void drawImageHPBar(s32 screenX, s32 screenY, GrpFrame* pFrame, Box32* grpRect, s32 colorData);			// 0047A820
	void setBuildShieldGain(Unit* a1);																		// 0047B6A0
	s32 getUnitBulletDamage(Unit* victim, Bullet* bullet);												// 0048ACD0
	s32 canCastSpell(Unit* a1);																			// 00492140
	s32 modifyUnitShields(s32 a1, s32 a2);																	// 004C5A20
	void compileHealthBar(s32 a1, s32 a2);																	// 004D6010
	s32 AI_FindTransport(Unit* a1, s32 a2);																// 004E6EF0
	s32 mapDataTransfer(char a1, s32 a2, s32 a3);															// 004EAFE0
}