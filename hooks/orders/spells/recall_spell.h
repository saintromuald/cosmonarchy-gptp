#pragma once
#include "../../../BW/structures/Unit.h"

namespace hooks {
	void spells_recall_action_on_valid_target_for_recall(Unit* target, Unit* caster);	// 004942D0
	void spells_recall_action_on_target_in_range(Unit* target, Unit* caster);			// 00494400
	void orders_recall(Unit* unit);														// 00494470

	void inject_recall_spell_hooks();
}