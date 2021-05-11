#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_Spell(Unit* unit);																	// 00492850
	s32 spellOrder(s32 defaultErrorString, Unit* caster, u16 techId, s32 range, s32* energyCost);	// 004926D0

	void injectCastOrderHooks();
}