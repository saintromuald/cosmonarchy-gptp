#include "supply_hooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>

namespace hooks
{
    // Until further notice, these hooks are all for the shared supply rewrite - Pr0nogo
    u32 getSuppliesAvailable(u8 playerId, s32 race) // 00488900
    {
        u32 result;
        u32 suppliesAvailable;
        race = (u8)race;
        if ((s8)race)
        {
            // Overly-complicated nonsense
            s32 raceId = race - 1;
            if (raceId)
            {
                if (raceId != 1)
                {
                    return 0;
                }
                suppliesAvailable = suppliesAvailable_Protoss[playerId];
                result = suppliesMax_Protoss[playerId];
            }
            else
            {
                suppliesAvailable = suppliesAvailable_Terran[playerId];
                result = suppliesMax_Terran[playerId];
            }
        }
        else
        {
            suppliesAvailable = suppliesAvailable_Zerg[playerId];
            result = suppliesMax_Zerg[playerId];
        }
        if (suppliesAvailable < result)
        {
            result = suppliesAvailable;
        }
        return result;
    }

    s32 getSuppliesRemaining(u8 playerId, u16 unitId) // 004889D0
    {
        GroupFlag  groupFlags = units_dat::GroupFlags[unitId];
        u32 supplyVar1 = 0;
        u32 supplyVar2 = 0;
        GroupFlag groupFlags2 = units_dat::GroupFlags[unitId];
        if (groupFlags.isZerg)
        {
            supplyVar1 = suppliesAvailable_Zerg[playerId];
            if (suppliesMax_Zerg[playerId] < supplyVar1)
            {
                supplyVar1 = hooks::getSuppliesMax(unitId, playerId);
                groupFlags = groupFlags2;
            }
            supplyVar2 = suppliesUsed_Zerg[playerId];
        }
        else
        {
            if (groupFlags.isTerran)
            {
                u32 supplyVar3 = suppliesAvailable_Terran[playerId];
                if (hooks::getSuppliesMax(unitId, playerId) >= supplyVar3)
                {
                    supplyVar2 = suppliesUsed_Terran[playerId];
                    supplyVar1 = supplyVar3;
                }
                else
                {
                    supplyVar1 = hooks::getSuppliesMax(unitId, playerId);
                    supplyVar2 = suppliesUsed_Terran[playerId];
                }
            }
            else
            {
                if (!(groupFlags.isProtoss))
                {
                    goto LABEL_15;
                }
                u32 supplyVar4 = suppliesAvailable_Protoss[playerId];
                if (hooks::getSuppliesMax(unitId, playerId) >= supplyVar4)
                {
                    supplyVar1 = supplyVar4;
                }
                else
                {
                    supplyVar1 = hooks::getSuppliesMax(unitId, playerId);
                }
                supplyVar2 = suppliesUsed_Protoss[playerId];
            }
            groupFlags = groupFlags2;
        }
LABEL_15:
        if (*CHEAT_STATE & 0x20000)
        {
            if (groupFlags.isZerg)
            {
                return suppliesMax_Zerg[playerId] - supplyVar2;
            }
            if (groupFlags.isTerran)
            {
                return suppliesMax_Terran[playerId] - supplyVar2;
            }
            if (groupFlags.isProtoss)
            {
                return suppliesMax_Protoss[playerId] - supplyVar2;
            }
            supplyVar1 = 0;
        }
        return supplyVar1 - supplyVar2;
    }

    s32 getSuppliesMax(u16 unitId, u8 playerId) // 00488990
    {
        GroupFlag groupFlags = units_dat::GroupFlags[unitId];
        if (groupFlags.isZerg)
        {
            return suppliesMax_Zerg[playerId];
        }
        if (groupFlags.isTerran)
        {
            return suppliesMax_Terran[playerId];
        }
        if (groupFlags.isProtoss)
        {
            return suppliesMax_Protoss[playerId];
        }
        return 0;
    }

    s32 getSuppliesUsed(s32 race, u8 playerId) // 004888C0
    {
        race = (u8)race;
        if (!(s8)race)
        {
            return suppliesUsed_Zerg[playerId];
        }
        s32 raceId = race - 1;
        if (!raceId)
        {
            return suppliesUsed_Terran[playerId];
        }
        if (raceId == 1)
        {
            return suppliesUsed_Protoss[playerId];
        }
        return 0;
    }

    char initializeUnitCounts() // 00488F90
    {
        memset(unitCountScore, 0, 0x540u);
        memset(allUnitCountTable, 0, 0xAB00u);

        s32 result = 0;
        if (multiPlayerMode && gameData->teamsEnabled)
        {
            s32 forceIterator = 1;
            do
            {
                result = unknown_57F41CB[(u8)forceIterator];
                u8 iteratorResult = unknown_57F41CB[(u8)forceIterator];
                if ((s8)result != -1)
                {
                    s32 supplyMax1 = 0;
                    s32 supplyMax2 = 0;
                    s32 supplyMax3 = 0;

                    if (400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 0) >= 400)
                    {
                        supplyMax1 = 400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 0);
                    }
                    else
                    {
                        supplyMax1 = 400;
                    }

                    supplyMax2 = iteratorResult;
                    suppliesMax_Zerg[supplyMax2] = supplyMax1;
                    if (400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 1) >= 400)
                    {
                        supplyMax3 = 400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 1);
                    }
                    else
                    {
                        supplyMax3 = 400;
                    }

                    suppliesMax_Terran[supplyMax2] = supplyMax3;
                    if (400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 2) >= 400)
                    {
                        result = 400 * (u8)helper::teamGame_CountRaces(forceIterator, (char*)0x190, 2);
                    }
                    else
                    {
                        result = 400;
                    }

                    suppliesMax_Protoss[supplyMax2] = result;
                }
                ++forceIterator;
            }
            while (forceIterator <= 4);
        }
        else
        {
            result = (int)suppliesMax_Terran;
            do
            {
                *(s32*)(result - 144) = 400;
                *(s32*)result = 400;
                *(s32*)(result + 144) = 400;
                result += 4;
            }
            while (result < (s32)suppliesAvailable_Protoss);
        }
        return result;
    }

    void StatRes_MainProc(BinDlg* dialog) // 004E5640
    {
        u8 v2;
        s32 v3;
        Unit* activeUnit;
        s8 activePlayer;
        u16 v6;
        u8 v7;
        s32 v8;
        u16 v9;
        u8 v10;
        s32 v11;
        s32 v12;
        s32 v13;
        u16 v14;
        s32 v15;
        s32 v16;
        BinDlg* v17;
        BinDlg* v18;
        s32 v19;

        if (dialog->controlType)
        {
            dialog = dialog->parent;
        }
        BinDlg* dialogVar = dialog->childrenDlg;
        if (dialogVar)
        {
            while (dialogVar->index != 1)
            {
                dialogVar = dialogVar->next;
                if (!dialogVar)
                {
                    goto LABEL_6;
                }
            }
        }
        else
        {
LABEL_6:
            dialogVar = 0;
        }
        v2 = *LOCAL_NATION_ID;
        v3 = 0;
        v19 = *LOCAL_NATION_ID;
        if (!IS_IN_REPLAY)
        {
            goto LABEL_15;
        }
        if (clientSelectionCount)
        {
            activeUnit = *activePortraitUnit;
            if (*clientSelectionCount != 1)
            {
                activeUnit = *clientSelectionGroup[0].unit;
            }
            activePlayer = activeUnit->playerId;
            if (activePlayer != 11)
            {
                v2 = activePlayer;
                *LOCAL_NATION_ID = activePlayer;
                if (activePlayer != *unknown_player_5153F8)
                {
                    v3 = 1;
                }
                *unknown_player_5153F8 = activePlayer;
LABEL_15:
                v8 = suppliesUsed_Zerg[v2];
                dialog = nullptr; // was lobyte = 0
                v6 = getSuppliesAvailable(v2, (s32)dialog);
                if (v6)
                {
                    helper::showSupplyInfo(dialogVar, 4, v8, v6);
                    dialogVar = dialogVar->next;
                    v7 = *LOCAL_NATION_ID;
                }
                v12 = v7;
                v13 = suppliesUsed_Terran[v7];
                v12 = 1;
                v9 = getSuppliesAvailable(v7, v12);
                if (v9)
                {
                    helper::showSupplyInfo(dialogVar, 5, v13, v9);
                    dialogVar = dialogVar->next;
                    v10 = *LOCAL_NATION_ID;
                }
                v16 = suppliesUsed_Protoss[v10];
                v11 = 2;
                v14 = getSuppliesAvailable(v10, v11);
                if (v14)
                {
                    helper::showSupplyInfo(dialogVar, 6, v16, v14);
                    dialogVar = dialogVar->next;
                    v15 = *LOCAL_NATION_ID;
                }
                helper::printStatResInfo(dialogVar, *consoleIndex + 1, (char**)&(*unknown_68C22C), (char*)gameGas[v15], v3);
                v18 = dialogVar->next;
                helper::printStatResInfo(v18, 0, (char**)&(*unknown_68C22C), (char*)gameMinerals[*LOCAL_NATION_ID], v3);
                v17 = v18->next;
                if (v18->next)
                {
                    do
                    {
                        if (v17->index > 5)
                        {
                            break;
                        }
                        helper::hideDialog(v17);
                        v17 = v17->next;
                    }
                    while (v17);
                }
                if (IS_IN_REPLAY)
                {
                    *LOCAL_NATION_ID = v19;
                }
                return;
            }
        }
        for (*unknown_player_5153F8 = -1; dialogVar; dialogVar = dialogVar->next)
        {
            if (dialogVar->index > 5)
            {
                break;
            }
            helper::hideDialog(dialogVar);
        }
    }

    s32 getSuppliesTemp() // 004246D0
    {
        Unit* activeUnit = *activePortraitUnit;
        GroupFlag groupFlags = units_dat::GroupFlags[activeUnit->id];
        s32 race = 0;
        if (groupFlags.isZerg) // redundant now
        {
            race = 0;
        }
        else if (groupFlags.isProtoss)
        {
            race = 2;
        }
        else
        {
            race = (groupFlags.isTerran) != 0 ? 1 : 4;
        }
        *suppliesAvailableTemp = getSuppliesAvailable(activeUnit->playerId, race);

        u8 playerId = activeUnit->playerId;
        if (groupFlags.isZerg)
        {
            race = 0;
        }
        else if (groupFlags.isProtoss)
        {
            race = 2;
        }
        else
        {
            race = (groupFlags.isTerran) != 0 ? 1 : 4;
        }

        s32 result = 0;
        race = (u8)race;
        if ((s8)race)
        {
            s32 raceId = race - 1;
            if (raceId)
            {
                if (raceId == 1)
                {
                    result = suppliesUsed_Protoss[playerId];
                    *suppliesUsedTemp = suppliesUsed_Protoss[playerId];
                }
                else
                {
                    result = 0;
                    *suppliesUsedTemp = 0;
                }
            }
            else
            {
                result = suppliesUsed_Terran[playerId];
                *suppliesUsedTemp = suppliesUsed_Terran[playerId];
            }
        }
        else
        {
            result = suppliesUsed_Zerg[playerId];
            *suppliesUsedTemp = suppliesUsed_Zerg[playerId];
        }
        return result;
    }

    bool redrawSupplyInfo() // 00425230 // Guessed name
    {
        if (!IS_IN_REPLAY && (u8)(*activePortraitUnit)->playerId != *LOCAL_NATION_ID)
        {
            return helper::unitStatCond_Standard();
        }

        u8 activePlayer = (*activePortraitUnit)->playerId;
        GroupFlag groupFlags = units_dat::GroupFlags[(u16)(*activePortraitUnit)->id];
        char isZerg = groupFlags.isZerg;
        char race = 0;

        if (groupFlags.isZerg) // redundant now
        {
            race = 0;
        }
        else if (groupFlags.isProtoss)
        {
            race = 2;
        }
        else
        {
            race = (groupFlags.isTerran) != 0 ? 1 : 4;
        }
        bool result = 1;

        if ((u16)suppliesAvailableTemp == getSuppliesAvailable(activePlayer, race))
        {
            char math = 0; // not initialized, defaulting to 0
            s32 suppliesVar = 0;
            char raceId = 0;

            if (isZerg) // redundant now
            {
                raceId = 0;
            }
            else
            {
                raceId = math & 4 ? 2 : (math & 2) != 0 ? 1 : 4;
            }
            if (raceId)
            {
                if (raceId == 1)
                {
                    suppliesVar = suppliesUsed_Terran[activePlayer];
                }
                else
                {
                    suppliesVar = raceId == 2 ? suppliesUsed_Protoss[activePlayer] : 0;
                }
            }
            else
            {
                suppliesVar = suppliesUsed_Zerg[activePlayer];
            }
            if ((u16)suppliesUsedTemp == suppliesVar)
            {
                result = 0;
            }
        }
        return result;
    }


}