#include <SCBW/structures/Unit.h>

namespace hooks
{
	u32 get_attack_priority(Unit* target, Unit* attacker);
	Unit* find_best_attack_target(Unit* unit);
	Unit* find_random_attack_target(Unit* unit);

	void inject_attack_priority_hooks();
}