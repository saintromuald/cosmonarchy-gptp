#pragma once
#include "../../../BW/structures/Unit.h"

namespace hooks
{

	void orders_feedback(Unit* unit);	//0x004F6D40

	void injectFeedbackSpellHook();

} //hooks