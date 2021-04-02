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
    
    void applyEnsnare(CUnit* a1);													// 004F45E0
    int32_t updateCorrosiveSpores(CUnit* target);                                   // 004F4480

    bool GorgonCondition(CUnit* attacker, CUnit* target);

    void injectWeaponDamageHook();

} //hooks
