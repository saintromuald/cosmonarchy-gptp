#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks {
	void CMDACT_Stimpack();					//004234D0
	void useStimPacksAIHook(Unit* unit);	//004554A0
	void CMDRECV_StimPack();				//004C2F30

	void injectStimPacksHooks();
}