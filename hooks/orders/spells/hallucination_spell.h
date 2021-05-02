#pragma once
#include "../../../SCBW/structures/CUnit.h"

namespace hooks
{

	void orders_Hallucination1(CUnit* unit);				// 004F6C40
	CUnit* createHallucination(CUnit* source, s32 player);	// 004F6B90
	void injectHallucinationSpellHook();


} //hooks
