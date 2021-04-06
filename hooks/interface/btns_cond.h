#pragma once
#include "../../SCBW/structures/CUnit.h"

namespace hooks {

	int32 BTNSCOND_NoNydusExit(CUnit* unit);		//00428340
	int32 BTNSCOND_Movement(CUnit* unit);			//004283C0
	int32 BTNSCOND_HasScarabs(CUnit* unit);		//004286A0

	//The injector function that should be called in initialize.cpp
	void injectBtnsCondHook();

} //hooks
