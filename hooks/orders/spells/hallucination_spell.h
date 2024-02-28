#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_hallucination1(Unit* unit);									// 004F6C40
	void apply_hallucination(Unit* unit, Unit* source, u32 time);
	Unit* create_hallucination(Unit* source, s32 player, u32 time = 1350);	// 004F6B90

	void inject_hallucination_spell_hooks();
}