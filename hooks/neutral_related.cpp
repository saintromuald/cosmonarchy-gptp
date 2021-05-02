#include "neutral_related.h"
#include <SCBW/api.h>

namespace
{ // helper declarations

}

namespace hooks
{
    bool unitNotNeutral(CHKUnit* unit)
    { // 004CBE20 
        u32 v1; // eax
        bool result; // eax

        result = 0;
        if (gameData->startingUnits)
        {
            if (unit->playerId != 0xb // PLAYER_Player12
                || ((v1 = (u16)unit->unitid, v1 < UnitId::ResourceMineralField) || v1 > UnitId::ResourceMineralFieldType3)
                && v1 != UnitId::ResourceVespeneGeyser
                /*              && v1 != UnitId::Critter_Rhynadon
                                && v1 != UnitId::Critter_Bengalaas
                                && v1 != UnitId::Critter_Ragnasaur
                                && v1 != UnitId::Critter_Scantid
                                && v1 != UnitId::Critter_Kakaru
                                && v1 != UnitId::Critter_Ursadon*/
                )
            {
                result = 1;
            }
        }
        return result;
    }
}

namespace
{ // helper definitions

}