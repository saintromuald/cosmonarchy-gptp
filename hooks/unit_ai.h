#pragma once
#include "BW/structures.h"

namespace hooks
{
	void ai_training_unit(Unit* result, Unit* parent);															// 004A2830
	void ai_create_nuke(Unit* silo, Unit* result);																// 0045B7A0
	void ai_add_military_to_region(AiRegion* region, Unit* unit, u32 must_use_region, void* debug_caller_address);	// 0043DA20
	void ai_training_simple_military(Unit* result, Unit* parent);												// 0043E280
	void ai_training_worker(Unit* newUnit, Unit* otherUnit);													// 00435700
	void ai_training_normal(Unit* newUnit, Unit* parentUnit);													// 00435DB0
	void* add_worker_structure_ai(Unit* unit, AiTown* town);													// 00433DD0
	void iterate_building_ai(Unit* result, Unit* parent);														// 00435770
	void remove_unit_ai(Unit* unit, UnitAi* unitAI);															// 004A1E50
	void delete_worker_ai(WorkerAi* worker_ai, WorkerAiList* worker_ai_list);									// 00404470
	void delete_building_ai(BuildingAi* building_ai, BuildingAiList* building_ai_list);							// 00404500
	void delete_military_ai(MilitaryAi* military_ai, AiRegion* ai_region);										// 004371D0
	b32 __stdcall remove_from_ai_structs(Unit* unit, b32 was_killed);											// 00439D60
	void remove_worker_or_building_ai(Unit* unit, bool always_building);										// 00434C90
	void ai_training_overlord(Unit* newUnit, Unit* oldUnit);													// 00435770
	void ai_order_lurker_burrow(Unit* unit);																	// 0043C230

	UnitAi* __stdcall find_nearest_military_or_guard_ai(AiRegion *ai_region, u32 a2, u32 a3, u32 x1, u32 y1, u32 a6); // 0043BAC0
	BuildingAi* create_building_ai(BuildingAiArray* ai_array);													// 004044C0
	bool is_valid_for(u32 unit_id, Unit* unit, u32 must_be_active);												// 0043B6D0
	void __stdcall add_guard_ai_to_unit(Unit* unit);															// 00462960
	void ai_remove_cleric(Unit* unit);																			// 004477C0

	void inject_unit_ai_hooks();
}