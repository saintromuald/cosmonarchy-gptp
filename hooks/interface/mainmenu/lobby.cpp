#include "lobby.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace { // Helper declarations

    void switchAllAvailableComputerSlotsToOpen();           // 004DBBE0
    bool setLastOpenSlotToComputer(s8 race);                // 004DBB70
}

namespace hooks {

    s32 initGame() { // 004EE110
        s32 v1; // esi

        if (multiPlayerMode) {
            return 0;
        }
        if (customSingleplayer[0]) {
            if (gameData->victoryCondition
            ||  gameData->startingUnits
            ||  gameData->tournamentModeEnabled) {
                switchAllAvailableComputerSlotsToOpen();
                playerTable[*LOCAL_NATION_ID].race = selectedSingleplayerRace[0];
                v1 = 0;
                do {
                    if (selectedSingleplayerRace[v1 + 1] != RaceId::None) {
                        setLastOpenSlotToComputer(selectedSingleplayerRace[v1 + 1]);
                    }
                    ++v1;
                } while (v1 < 8);
            }
        }
        return 1;
    }
}

namespace { // Helper definitions

    const u32 Func_switchAllAvailableComputerSlotsToOpen = 0x004DBBE0;
    void switchAllAvailableComputerSlotsToOpen() {
        __asm {
            PUSHAD
            CALL Func_switchAllAvailableComputerSlotsToOpen
            POPAD
        }
    }

    const u32 Func_setLastOpenSlotToComputer = 0x004DBB70;
    bool setLastOpenSlotToComputer(s8 race) {
        static s32 result;
        __asm {
            PUSHAD
            PUSH race
            CALL Func_setLastOpenSlotToComputer
            MOV result, EAX
            POPAD
        }
        return (bool)result;
    }
}