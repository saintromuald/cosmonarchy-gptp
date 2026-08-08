#include <BW/structures/Unit.h>

namespace hooks {
u32 get_attack_priority(Unit* attacker, Unit* target);	// 00442160
Unit* find_best_attack_target(Unit* unit);				// 00443080
Unit* find_random_attack_target(Unit* unit);			// 00442FC0

void inject_attack_priority_hooks();
}