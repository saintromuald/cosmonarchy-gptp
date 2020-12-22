#include "neutral_related.h"
#include <SCBW/api.h>

namespace { // helper declarations

}

namespace hooks {
    bool unitNotNeutral(CHKUnit* unit) { // 004CBE20 
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

    bool shouldAttackUnit(CUnit* attacker, CUnit* target) { // 00431550 | 
        u8 targetPlayer; // al
        s32 flags; // eax
        bool result; // eax

        scbw::printText("running");
        result = 0;
        if (attacker && target) {
            targetPlayer = target->playerId;
            if (targetPlayer == 11)
                targetPlayer = target->sprite->playerId;
            if (!(scbw::isAlliedTo(attacker->playerId, target->playerId))) {
                flags = target->status;
                if (!(flags & UnitStatus::UNKNOWN5) && (!((flags & UnitStatus::RequiresDetection) && (flags & UnitStatus::Cloaked))
                    || (1 << (char)attacker) & target->visibilityStatus))
                {
                    if (!(flags & UnitStatus::IsHallucination)
                    || ((units_dat::MaxHitPoints[target->id] + units_dat::MaxShieldPoints[target->id]) / 256) <= target->getCurrentLifeInGame())
                        result = 1;
                }
            }
        }
        return result;
    }
}

namespace { // helper definitions

}