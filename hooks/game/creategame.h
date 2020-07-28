#pragma once
#include "SCBW/structures.h"

namespace hooks {

	signed int singleplayerMeleeCreate();				//004EE110
	signed int loadGameCreate(int a1, char a2);			//004EE520
	signed int levelCheatCreate(char a1, int a2);		//004EE5B0
	void injectConsoleHooks();
}