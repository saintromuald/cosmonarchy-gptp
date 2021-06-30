#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	void AI_UnitWasHit(Unit* self, Unit* attacker, s32 mainTargetReactions, s32 dontCallHelp);							// 0043F320
	bool getBestSCVForRepairProc(Unit* repairman, Unit* repairTarget);													// 004402B0
	b32 AI_MilitaryShouldWaitForGroup(Unit* unit);																		// 0043AA10
	AiTown* deleteAITownIfNeeded(AiTown* town);																			// 004347E0
	bool AI_CanMineExtra(Unit* unit);																					// 00447A70
	void AI_OrderUnitCloaking(Unit* unit);																				// 0043B970
	void AI_SendSuicide(char argType, s32 player);																		// 0043E050
	u32 AI_RecalculateRegionStrength(AiRegion* region);																	// 0043A390
	Unit* getClosestEnemyStructure(s32 x, s32 y, s32 playerId);															// 004A0F90
	s16 getRegionIdFromUnit(Unit* unit);																				// 0049CB40
	bool AI_IsBuildingSafe(Unit* unit);																					// 00438910
	bool AI_AreOnConnectedRegions(Unit* unit1, Unit* unit2);															// 00438E10

	s32 getBunkerPlacementPosition(u32 placementId, u32* regionId);														// 004442E0
	void AI_GetBuildLocation(u32 unitId, char* placementData, u32 playerId, Point16 desiredPos, s32 maxRange);			// 004461B0
	u32 getTurretPlacementPosition(char* placementData, u32 playerId, Point16 desiredPos, u16 unitId, u32 unkFlag);		// 00443AC0
	bool AI_CheckMilitaryGrouping(Unit* unit);																			// 00439550

	void injectAIOrdersHooks();
}