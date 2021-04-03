#include <SCBW/structures/CUnit.h>

namespace hooks {

uint32 getAttackPriorityHook(CUnit* target, CUnit* attacker);
CUnit* findBestAttackTargetHook(CUnit* unit);
CUnit* findRandomAttackTargetHook(CUnit* unit);

void injectAttackPriorityHooks();

} //hooks
