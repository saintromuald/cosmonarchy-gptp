#pragma once
#include "../../SCBW/structures/CUnit.h"

void augurSacrifice(CUnit* target);
namespace hooks {

    void weaponDamageHook(s32     damage,
                          CUnit*  target,
                          u8      weaponId,
                          CUnit*  attacker,
                          u8      attackingPlayerId,
                          s8      direction,
                          u8      dmgDivisor);
    // Hooked to allow for modifying Ensnare duration - Pr0nogo
    void applyEnsnare(CUnit* a1);													// 004F45E0
    bool GorgonCondition(CUnit* attacker, CUnit* target);
    void injectWeaponDamageHook();

} //hooks
