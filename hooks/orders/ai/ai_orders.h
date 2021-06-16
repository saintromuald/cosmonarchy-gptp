#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	void AI_BuildTechUpgrade(AiTown* town, char method, char amount, s16 id, char priority, bool need);	        // 00431F90
	void AI_UnitWasHit(Unit* self, Unit* attacker, s32 mainTargetReactions, s32 dontCallHelp);					// 0043F320
	void AI_AddMilitary(AiRegion* region, Unit* unit, b32 alwaysThisRegion);									// 0043DA20
	bool getBestSCVForRepairProc(Unit* repairman, Unit* repairTarget);											// 004402B0
	b32 AI_MilitaryShouldWaitForGroup(Unit* unit);																// 0043AA10
	AiTown* deleteAITownIfNeeded(AiTown* town);																	// 004347E0
	bool Ai_CanMineExtra(Unit* unit);																			// 00447A70
	void AI_orderUnitCloaking(Unit* unit);																		// 0043B970
	void AI_SendSuicide(char argType, s32 player);																// 0043E050
	u32 AI_RecalculateRegionStrength(AiRegion* region);															// 0043A390
	AiTown* AI_progressBuild(s32 playerId, s32* minerals, s32* gas);											// 00434480
	Unit* getClosestEnemyStructure(s32 x, s32 y, s32 playerId);													// 004A0F90
	s32 AIManageBuildingPlacement(Unit* builder, u32 unitId,													// 004465C0
		Point16 tryPosition, Point16* outPosition, s32 areaTiles);
	s16 getRegionIdFromUnit(Unit* unit);																		// 0049CB40

	void injectAIOrdersHooks();
}