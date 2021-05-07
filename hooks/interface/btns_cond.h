#pragma once
#include "../../SCBW/structures/Unit.h"

namespace hooks
{

	s32 BTNSCOND_NoNydusExit(Unit* unit);		//00428340
	s32 BTNSCOND_Movement(Unit* unit);			//004283C0
	s32 BTNSCOND_HasScarabs(Unit* unit);		//004286A0

	//The injector function that should be called in initialize.cpp
	void injectBtnsCondHook();

} //hooks
