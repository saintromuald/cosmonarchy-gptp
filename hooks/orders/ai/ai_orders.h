#pragma once
#include "SCBW/structures.h"

namespace hooks {

	int askForHelp_AI(CUnit* a1, CUnit* a2, int a3);				// 0043ADA0
	void injectAIOrdersHooks();
}