#include "supply_hooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <windows.h>

namespace hooks
{
    // Until further notice, these hooks are all for the shared supply rewrite - Pr0nogo
    u32 getSuppliesAvailable(u8 playerId, s32 race)
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
}