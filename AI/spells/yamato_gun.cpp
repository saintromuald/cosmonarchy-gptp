#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{

    CUnit* findBestYamatoGunTarget(CUnit* caster, bool isUnderAttack)
    {
        auto match = [&](CUnit* target)
        {
            if (!isTargetWorthHitting(target, caster))
                return false;

            if ((target->status & UnitStatus::GroundedBuilding) && target->hasWeapon())
                return true;

            if (target->id == UnitId::bunker && target->hasLoadedUnit())
                return true;

            if (scbw::isZergEggNew(target))
                return false;

            const s32 targetLife = target->getCurrentLifeInGame();
            if (200 <= targetLife && targetLife <= 450
                && !(units_dat::BaseProperty[target->id] & UnitProperty::Hero))
                return true;

            return false;
        };

        if (isUnderAttack)
        {
            s32 bounds = 32 * 16;
            return scbw::UnitFinder::getNearestTarget(
                caster->getX() - bounds,
                caster->getY() - bounds,
                caster->getX() + bounds,
                caster->getY() + bounds,
                caster,
                match
            );
        }
        else
        {
            return scbw::UnitFinder::getNearestTarget(caster, match);
        }
    }

} //AI
