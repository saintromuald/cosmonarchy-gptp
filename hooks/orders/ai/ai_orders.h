#pragma once
#include "SCBW/structures.h"

namespace hooks {

	void AI_TrainingUnit(CUnit* result, CUnit* parent);														// 004A2830
	void AI_CreateNuke(CUnit* silo, CUnit* result);															// 0045B7A0
	void AI_TrainingBroodling(CUnit* parent, CUnit* result);												// 0043E280
	void AI_AddUnitAi(CUnit* unit, AiTown* town);															// 00433DD0
	void AI_BuildTechUpgrade(AiTown* town, int32 method, int32 amount, int32 id, int32 priority, int32 need);			// 00431F90
	void AI_UnitWasHit(CUnit* self, CUnit* attacker, int32 mainTargetReactions, int32 dontCallHelp);			// 0043F320
	void AI_IterateBuildingAI(CUnit* result, CUnit* parent);												// 00435770

	// Attack extender
	uint32 AI_TrainAttackForce(AiCaptain* region);																// 0043E670
	void AI_GroupingForAttack(AiCaptain* region);															// 0043EEC0
	uint32 AI_GetMissingAttackForceUnits(AiCaptain* region, uint32* unitIds, uint32 isCampaign);						// 00438050
	void AI_AddToAttack(uint32 result, uint32 player, uint32 unitId);												// 00447230
	bool AI_AttackManager(uint32 _player, int32 x1, int32 y1, uint32 alywaysOverwriteLastAttack,						// 0043ABB0
		uint32 allowFallbackAirOnly);
	int32 AI_HasUnitsForAttackForce(uint32 player);																// 00437F20
//	void AI_RemoveFromAttack(int32 unitId, uint32 player);														// 00447040

	void injectAIOrdersHooks();
}