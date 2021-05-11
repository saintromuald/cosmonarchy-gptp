#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void orders_Spell(Unit* unit);																			// 00492850
	s32 spellOrder(s32 defaultErrorString, Unit* caster, u16 techId, s32 range, s32* energyCost);			// 004926D0
	bool canTargetSpell(u16 techId, Unit* target, s16 fowUnitId, Unit* caster, s16 x, s16 y, s16* error);	// 00492020

	void injectCastOrderHooks();
}