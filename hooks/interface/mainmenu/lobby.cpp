#include "lobby.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace { // Helper declarations

	// For initGame
	void switchAllAvailableComputerSlotsToOpen();           // 004DBBE0
	bool setLastOpenSlotToComputer(int8 race);                // 004DBB70

	// For startGame
	int32 bootReason(uint8 a1);                                  // 004B8870
	int32 isAuthedMap(void* a1);                              // 004C4870
	void setTimerFunc();                                    // 00472E00
	int32 getTickCount();                                     // 004FE0C4 // CAUSES CRASH when accessed, is it instead a variable to be defined in scbwdata?
}

namespace hooks {

	int32 initGame() { // 004EE110
		int32 v1; // esi

		if (*multiPlayerMode) {
			return 0;
		}
		if (customSingleplayer[0]) {
			if (gameData->victoryCondition
				|| gameData->startingUnits
				|| gameData->tournamentModeEnabled) {
				switchAllAvailableComputerSlotsToOpen();
				playerTable[*LOCAL_NATION_ID].race = selectedSingleplayerRace[0];
				v1 = 0;
				do {
					if (selectedSingleplayerRace[v1 + 1] != RaceId::None) {
						setLastOpenSlotToComputer(selectedSingleplayerRace[v1 + 1]);
						if (/*playerTable[v1].type == PlayerType::NotUsed || */playerTable[v1].type == PlayerType::OpenSlot) {
							playerTable[v1].type = PlayerType::Computer;
						}
					}


					++v1;
				} while (v1 < 8);
			}
		}
		return 1;
	}

	int32 startGame(void* a1, int32 a2) { // 00472060
		uint8 v2; // al
		int32 result; // eax
		int32 v4; // eax

		a1 = gameState;
		v2 = 4;
		if (*gameState != 4) {
			return bootReason(v2);
		}
		result = a2;
		if (a2) {
			return result;
		}
		if (!isAuthedMap(a1)) {
			v2 = 9;
			return bootReason(v2);
		}
		if (*IS_IN_REPLAY) {
			for (int i = 0; i < 12; i++) {
				playerReplayWatchers[i] = playerTable[i];
			}
		}
		*gameState = 5;
		*countdownTimerInterval = 24;
		if (2 * *provider_LatencyCalls + 4 >= 24) {
			*countdownTimerInterval = 2 * *provider_LatencyCalls + 4;
		}
		setTimerFunc();
		v4 = getTickCount(); // CRASH
		*countdownTimeTickCount = v4;
		*countdownTimeTickCount_0 = v4;
		result = v4 - 120000;
		*isInGame = 1;
		*elapstedTimeModifier = 0;
		*countdownTimeRemaining = result;
		for (int i = 0; i < 8; i++) {
			if (selectedSingleplayerRace[i] != RaceId::None) {
				if (/*playerTable[i].type == PlayerType::NotUsed || */playerTable[i].type == PlayerType::OpenSlot) {
					playerTable[i].type = PlayerType::Computer;
				}
			}
		}
		return result;
	}
}

namespace { // Helper definitions

	// For initGame
	const uint32 Func_switchAllAvailableComputerSlotsToOpen = 0x004DBBE0;
	void switchAllAvailableComputerSlotsToOpen() {
		__asm {
			PUSHAD
			CALL Func_switchAllAvailableComputerSlotsToOpen
			POPAD
		}
	}

	const uint32 Func_setLastOpenSlotToComputer = 0x004DBB70;
	bool setLastOpenSlotToComputer(int8 race) {
		static int32 result;
		__asm {
			PUSHAD
			PUSH race
			CALL Func_setLastOpenSlotToComputer
			MOV result, EAX
			POPAD
		}
		return (bool) result;
	}

	// For startGame
	const uint32 Func_bootReason = 0x004B8870;
	int32 bootReason(uint8 a1) {
		static int32 result;
		__asm {
			PUSHAD
			MOVZX AL, a1
			CALL Func_bootReason
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_isAuthedMap = 0x004C4870;
	int32 isAuthedMap(void* a1) {
		static int32 result;
		__asm {
			PUSHAD
			MOV EAX, a1
			CALL Func_isAuthedMap
			MOV result, EAX
			POPAD
		}
		return result;
	}

	const uint32 Func_setTimerFunc = 0x00472E00;
	void setTimerFunc() {
		__asm {
			PUSHAD
			CALL Func_setTimerFunc
			POPAD
		}
	}

	int32 getTickCount() {
		static int32 result;
		static uint32 Func_getTickCount = *(uint32*) 0x004FE0C4;
		__asm {
			PUSHAD
			CALL Func_getTickCount
			MOV result, EAX
			POPAD
		}
		return result;
	}
}