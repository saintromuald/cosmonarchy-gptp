#pragma once
#include "../../SCBW/structures/CUnit.h"

void augurSacrifice(CUnit* target);
namespace hooks {

    void weaponDamageHook(int32     damage,
                          CUnit*  target,
                          uint8      weaponId,
                          CUnit*  attacker,
                          uint8      attackingPlayerId,
                          int8      direction,
                          uint8      dmgDivisor);
    
    void applyEnsnare(CUnit* a1);													// 004F45E0
    int32_t updateCorrosiveSpores(CUnit* target);                                   // 004F4480

    bool GorgonCondition(CUnit* attacker, CUnit* target);

    void injectWeaponDamageHook();

} //hooks
