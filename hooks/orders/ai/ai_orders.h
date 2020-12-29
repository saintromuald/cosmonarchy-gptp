#pragma once
#include "SCBW/structures.h"

namespace hooks {

	void AI_TrainingUnit(CUnit* result, CUnit* parent);					// 004A2830
	void AI_CreateNuke(CUnit* silo, CUnit* result, CUnit* a3);			// 0045B7A0
	void injectAIOrdersHooks();
}