#pragma once
#include "SCBW/structures.h"

namespace hooks {

	int32 initGame();							// 004EE110
	int32 startGame(void* a1, int32 a2);		// 00472060

	void injectLobbyHooks();
}