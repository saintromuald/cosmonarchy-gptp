#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	void ai_training_unit(Unit* result, Unit* parent);															// 004A2830
	void AI_CreateNuke(Unit* silo, Unit* result);																// 0045B7A0
	void AI_AddMilitary(AiRegion* region, Unit* unit);															// 0043DA20
	void AI_TrainingSimpleMilitary(Unit* result, Unit* parent);													// 0043E280
	void AI_TrainingWorker(Unit* newUnit, Unit* otherUnit);														// 00435700
	void AI_TrainingNormal(Unit* newUnit, Unit* parentUnit);													// 00435DB0
	void* AI_AddWorkerStructureAI(Unit* unit, AiTown* town);													// 00433DD0
	void AI_IterateBuildingAI(Unit* result, Unit* parent);														// 00435770
	void AI_RemoveUnitAI(Unit* unit, UnitAi* unitAI);															// 004A1E50
	void delete_worker_ai(WorkerAi* workerAi, WorkerAiArray* workerAiArray);										// 00404470
	void delete_building_ai(BuildingAi* buildingAi, BuildingAiArray* buildingAiArray);							// 00404500
	s32 AI_RemoveWorkerOrBuildingAI(Unit* unit, UnitAi* unitAI);												// 00434C90
	void AI_TrainingOverlord(Unit* newUnit, Unit* oldUnit);														// 00435770
	void AI_orderLurkerBurrow(Unit* unit);																		// 0043C230

	UnitAi* __stdcall Ai_FindNearestMilitaryOrGuardAi(AiRegion *ai_region, u32 a2, u32 a3, u32 x1, u32 y1, u32 a6); // 0043BAC0
	BuildingAi* create_building_ai(BuildingAiArray* ai_array);													// 004044C0

	void injectUnitAIHooks();
}