#include <SCBW/structures/Unit.h>

namespace hooks
{

	u32 getAttackPriorityHook(Unit* target, Unit* attacker);
	Unit* find_best_attack_target(Unit* unit);
	Unit* findRandomAttackTargetHook(Unit* unit);

	void inject_attack_priority_hooks();

} //hooks
