#pragma once
#include "../../../SCBW/structures/CUnit.h"

namespace hooks {

	void orders_Hallucination1(CUnit* unit);	//0x004F6C40

	void injectHallucinationSpellHook();


} //hooks

namespace hallucinationHelpers {
	void UnitDestructor(CUnit* unit);
	bool function_004F66D0(CUnit* unit);
	CUnit* createHallucinationUnit(CUnit* target, u32 playerId);
}
