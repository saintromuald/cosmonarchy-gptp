#pragma once
#include "SCBW/structures.h"

namespace hooks
{

	void AI_TrainingUnit(Unit* result, Unit* parent);															// 004A2830
	void AI_CreateNuke(Unit* silo, Unit* result);																// 0045B7A0
	void AI_TrainingBroodling(Unit* result, Unit* parent);														// 0043E280
	char AI_AddUnitAI(Unit* unit, AiTown* town);																// 00433DD0
	void AI_BuildTechUpgrade(AiTown* town, s32 method, s32 amount, s32 id, s32 priority, s32 need);	            // 00431F90
	void AI_UnitWasHit(Unit* self, Unit* attacker, s32 mainTargetReactions, s32 dontCallHelp);					// 0043F320
	void AI_IterateBuildingAI(Unit* result, Unit* parent);														// 00435770
	char AI_TrainingWorker(Unit* child, Unit* parent);															// 00435700
//	void AI_AddMilitary(AiRegion* region, Unit* unit, b32 alwaysThisRegion);									// 0043DA20

	// Attack extender
	u32 AI_TrainAttackForce(AiRegion* region);																    // 0043E670
	void AI_GroupingForAttack(AiRegion* region);																// 0043EEC0
	u32 AI_GetMissingAttackForceUnits(AiRegion* region, u32* unitIds, u32 isCampaign);				            // 00438050
	void AI_AddToAttack(u32 result, u32 player, u32 unitId);											        // 00447230
	bool AI_AttackManager(u32 _player, s32 x1, s32 y1, u32 alywaysOverwriteLastAttack,				            // 0043ABB0
		u32 allowFallbackAirOnly);
	s32 AI_HasUnitsForAttackForce(u32 player);																    // 00437F20
	void AI_orderUnitCloaking(Unit* unit);																		// 0043B970
	WorkerAi* deleteWorkerAI(WorkerAi* workerAi, WorkerAiArray* workerAiList);									// 00404470
	void AI_SendSuicide(char argType, s32 player);																// 0043E050
	u32 AI_RecalculateRegionStrength(AiRegion* region);															// 0043A390
	AiTown* AI_progressBuild(s32 playerId, s32* minerals, s32* gas);											// 00434480
	Unit* getClosestEnemyStructure(s32 x, s32 y, s32 playerId);													// 004A0F90
	s32 AIManageBuildingPlacement(Unit* builder, u32 unitId,													// 004465C0
		Point16 tryPosition, Point16* outPosition, s32 areaTiles);
	s16 getRegionIdFromUnit(Unit* unit);																		// 0049CB40

	void injectAIOrdersHooks();
}