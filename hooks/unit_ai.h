#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	void AI_TrainingUnit(Unit* result, Unit* parent);															// 004A2830
	void AI_CreateNuke(Unit* silo, Unit* result);																// 0045B7A0
	void AI_AddMilitary(AiRegion* region, Unit* unit, b32 alwaysThisRegion);									// 0043DA20
	void AI_TrainingSimpleMilitary(Unit* result, Unit* parent);													// 0043E280
	char AI_TrainingWorker(Unit* newUnit, Unit* otherUnit);														// 00435700
	void AI_TrainingNormal(Unit* newUnit, Unit* parentUnit);													// 00435DB0
	char AI_AddWorkerStructureAI(Unit* unit, AiTown* town);														// 00433DD0
	void AI_IterateBuildingAI(Unit* result, Unit* parent);														// 00435770
	void AI_RemoveUnitAI(Unit* unit, UnitAi* unitAI);															// 004A1E50
	void deleteWorkerAI(WorkerAi* workerAi, WorkerAiArray* workerAiArray);										// 00404470
	void deleteBuildingAI(BuildingAi* buildingAi, BuildingAiArray* buildingAiArray);							// 00404500
	s32 AI_RemoveWorkerOrBuildingAI(Unit* unit, UnitAi* unitAI);												// 00434C90

	void injectUnitAIHooks();
}