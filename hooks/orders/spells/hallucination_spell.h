#pragma once
#include "../../../SCBW/structures/CUnit.h"

namespace hooks {

	void orders_Hallucination1(CUnit* unit);				// 004F6C40
	CUnit* createHallucination(CUnit* source, int32 player);	// 004F6B90
	void injectHallucinationSpellHook();


} //hooks

namespace {
	void UnitDestructor(CUnit* unit);
	bool function_004F66D0(CUnit* unit);
	CUnit* createHallucinationUnit(CUnit* target, uint32 playerId);
}
