#pragma once
#include "../../../SCBW/structures/Unit.h"

namespace hooks
{

	void orders_Feedback(CUnit* unit);	//0x004F6D40

	void injectFeedbackSpellHook();

} //hooks