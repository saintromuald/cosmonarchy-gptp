#pragma once
#include "SCBW/structures.h"

namespace hooks {

	void AI_TrainingUnit(CUnit* result, CUnit* parent);
	void injectAIOrdersHooks();
}