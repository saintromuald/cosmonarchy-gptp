#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{
	void ordersCastMindControl(Unit* unit);			// 004F6950
	void injectMindControlSpellHook();
}