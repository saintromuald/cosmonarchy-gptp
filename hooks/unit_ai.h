#pragma once
#include "SCBW/structures.h"

namespace hooks
{
	void ai_training_unit(Unit* result, Unit* parent);															// 004A2830
	void ai_create_nuke(Unit* silo, Unit* result);																// 0045B7A0
	void ai_add_military_to_region(AiRegion* region, Unit* unit, u32 must_use_region);							// 0043DA20
	void ai_training_simple_military(Unit* result, Unit* parent);												// 0043E280
	void ai_training_worker(Unit* newUnit, Unit* otherUnit);													// 00435700
	void ai_training_normal(Unit* newUnit, Unit* parentUnit);													// 00435DB0
	void* add_worker_structure_ai(Unit* unit, AiTown* town);													// 00433DD0
	void iterate_building_ai(Unit* result, Unit* parent);														// 00435770
	void remove_unit_ai(Unit* unit, UnitAi* unitAI);															// 004A1E50
	void delete_worker_ai(WorkerAi* workerAi, WorkerAiArray* workerAiArray);									// 00404470
	void delete_building_ai(BuildingAi* buildingAi, BuildingAiArray* buildingAiArray);							// 00404500
	s32 remove_worker_or_building_ai(Unit* unit, UnitAi* unitAI);												// 00434C90
	void ai_training_overlord(Unit* newUnit, Unit* oldUnit);													// 00435770
	void ai_order_lurker_burrow(Unit* unit);																	// 0043C230

	UnitAi* __stdcall find_nearest_military_or_guard_ai(AiRegion *ai_region, u32 a2, u32 a3, u32 x1, u32 y1, u32 a6); // 0043BAC0
	BuildingAi* create_building_ai(BuildingAiArray* ai_array);													// 004044C0

	void inject_unit_ai_hooks();
}