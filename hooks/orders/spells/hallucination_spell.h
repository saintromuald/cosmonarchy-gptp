#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_Hallucination1(Unit* unit);				// 004F6C40
	Unit* createHallucination(Unit* source, s32 player);	// 004F6B90
	void injectHallucinationSpellHook();


} //hooks
