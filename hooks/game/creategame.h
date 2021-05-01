#pragma once
#include "SCBW/structures.h"

namespace hooks {

	s32 singleplayerMeleeCreate();				//004EE110
	s32 loadGameCreate(s32 a1, char a2);			//004EE520
	s32 levelCheatCreate(char a1, s32 a2);		//004EE5B0
	void injectConsoleHooks();
}