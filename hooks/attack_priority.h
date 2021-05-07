#include <SCBW/structures/Unit.h>

namespace hooks
{

	u32 getAttackPriorityHook(CUnit* target, CUnit* attacker);
	CUnit* findBestAttackTargetHook(CUnit* unit);
	CUnit* findRandomAttackTargetHook(CUnit* unit);

	void injectAttackPriorityHooks();

} //hooks
