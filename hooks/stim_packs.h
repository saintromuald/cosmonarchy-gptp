#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {
	void cmdact_stimpack();					//004234D0
	void use_stimpack_ai(Unit* unit);	//004554A0
	void cmdrecv_stimpack();				//004C2F30

	void injectStimPacksHooks();
}