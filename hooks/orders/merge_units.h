#pragma once
#include "../../BW/structures/Unit.h"

namespace hooks {
Unit* templar_merge_partner(Unit* unit, Unit** units_list, u32 units_list_length);	// 004BFA80
void orders_completing_archon_summon(Unit* unit);									// 00493B10
void orders_warping_mind_tyrant(Unit* unit);										// 00494690
void orders_warping_archon(Unit* unit);												// 004948B0

void inject_merge_units_hooks();
} //hooks