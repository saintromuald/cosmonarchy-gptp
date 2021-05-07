#include <SCBW/structures/Unit.h>

namespace hooks
{

	u32 getAttackPriorityHook(Unit* target, Unit* attacker);
	Unit* findBestAttackTargetHook(Unit* unit);
	Unit* findRandomAttackTargetHook(Unit* unit);

	void injectAttackPriorityHooks();

} //hooks
