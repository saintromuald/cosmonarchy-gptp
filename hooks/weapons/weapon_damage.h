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

bool GorgonCondition(CUnit* attacker, CUnit* target);
void injectWeaponDamageHook();

} //hooks
