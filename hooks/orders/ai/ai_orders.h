#pragma once
#include "SCBW/structures.h"

namespace hooks
{

	void AI_TrainingUnit(Unit* result, Unit* parent);															// 004A2830
	void AI_CreateNuke(Unit* silo, Unit* result);																// 0045B7A0
	void AI_TrainingBroodling(Unit* result, Unit* parent);													// 0043E280
	char AI_AddUnitAi(Unit* unit, AiTown* town);																// 00433DD0
	void AI_BuildTechUpgrade(AiTown* town, s32 method, s32 amount, s32 id, s32 priority, s32 need);	            // 00431F90
	void AI_UnitWasHit(Unit* self, Unit* attacker, s32 mainTargetReactions, s32 dontCallHelp);			    // 0043F320
	void AI_IterateBuildingAI(Unit* result, Unit* parent);													// 00435770

	// Attack extender
	u32 AI_TrainAttackForce(AiCaptain* region);																    // 0043E670
	void AI_GroupingForAttack(AiCaptain* region);																// 0043EEC0
	u32 AI_GetMissingAttackForceUnits(AiCaptain* region, u32* unitIds, u32 isCampaign);				            // 00438050
	void AI_AddToAttack(u32 result, u32 player, u32 unitId);											        // 00447230
	bool AI_AttackManager(u32 _player, s32 x1, s32 y1, u32 alywaysOverwriteLastAttack,				            // 0043ABB0
		u32 allowFallbackAirOnly);
	s32 AI_HasUnitsForAttackForce(u32 player);																    // 00437F20    
	void AI_orderUnitCloaking(Unit* unit);                                                                     // 0043B970

	void injectAIOrdersHooks();
}