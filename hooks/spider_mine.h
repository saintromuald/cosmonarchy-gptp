#pragma once
#include "../BW/structures/Unit.h"

namespace hooks {
	Unit* find_best_mine_target(Unit* mine);		// 00441270
	void orders_terran_mine(Unit* unit);			// 00463DF0
	void orders_place_mine(Unit* unit);				// 00464FD0
	void burrow_generic(Unit* unit);				// 004E9A30

	void inject_spider_mine_hooks();
}