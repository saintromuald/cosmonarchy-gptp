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
}