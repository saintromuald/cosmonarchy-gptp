#include "spells.h"
#include <AI/ai_common.h>

//V241 for VS2008

namespace AI
{

    CUnit* findBestSpawnBroodlingsTarget(CUnit* caster, bool isUnderAttack)
    {

        s32 bounds;

        if (isUnderAttack)
            bounds = 32 * 9;
        else
            bounds = 32 * 64;

        return scbw::UnitFinder::getNearestTarget(
            caster->getX() - bounds, caster->getY() - bounds,
            caster->getX() + bounds, caster->getY() + bounds,
            caster, [&](CUnit* target)
        {

            if (!isTargetWorthHitting(target, caster))
                return false;

            if (!scbw::canWeaponTargetUnit(WeaponId::SpawnBroodlings, target, caster))
                return false;

            if (units_dat::BaseProperty[target->id] & UnitProperty::Hero)
                return false;

            if (!isUnderAttack && target->getCurrentLifeInGame() < 100)
                return false;

            if (units_dat::BaseProperty[target->id] & UnitProperty::Worker
                || target->id == UnitId::siege_tank
                || target->id == UnitId::siege_tank_s
                || target->id == UnitId::medic
                || (target->mainOrderId == OrderId::NukeWait
                    || target->mainOrderId == OrderId::NukeTrack))
                return true;

            return false;

        });
    }

} //AI
