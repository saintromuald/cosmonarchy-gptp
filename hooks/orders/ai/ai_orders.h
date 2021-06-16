#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	void AI_UnitWasHit(Unit* self, Unit* attacker, s32 mainTargetReactions, s32 dontCallHelp);					// 0043F320
	bool getBestSCVForRepairProc(Unit* repairman, Unit* repairTarget);											// 004402B0
	b32 AI_MilitaryShouldWaitForGroup(Unit* unit);																// 0043AA10
	AiTown* deleteAITownIfNeeded(AiTown* town);																	// 004347E0
	bool AI_CanMineExtra(Unit* unit);																			// 00447A70
	void AI_OrderUnitCloaking(Unit* unit);																		// 0043B970
	void AI_SendSuicide(char argType, s32 player);																// 0043E050
	u32 AI_RecalculateRegionStrength(AiRegion* region);															// 0043A390
	Unit* getClosestEnemyStructure(s32 x, s32 y, s32 playerId);													// 004A0F90
	s16 getRegionIdFromUnit(Unit* unit);																		// 0049CB40

	void injectAIOrdersHooks();
}