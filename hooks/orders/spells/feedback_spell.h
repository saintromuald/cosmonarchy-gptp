#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_Feedback(Unit* unit);	//0x004F6D40

	void injectFeedbackSpellHook();

} //hooks