#pragma once
#include "SCBW/structures.h"

namespace hooks {

	void AI_TrainingUnit(CUnit* result, CUnit* parent);														// 004A2830
	void AI_CreateNuke(CUnit* silo, CUnit* result);															// 0045B7A0
	void AI_TrainingBroodling(CUnit* parent, CUnit* result);												// 0043E280
	void AI_AddUnitAi(CUnit* unit, AiTown* town);															// 00433DD0
	void AI_BuildTechUpgrade(AiTown* town, char method, char a3, s16 id, char priority, bool need);		// 00431F90
	void injectAIOrdersHooks();
}