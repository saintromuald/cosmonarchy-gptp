#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{
	void set_rally_unit(Unit* unit, Unit* target);				// 00466B40
	void order_new_unit_to_rally(Unit* unit, Unit* factory);	// 00466F50
	void set_rally_position(Unit* unit, u16 x, u16 y);			// 00466910
	u32 btnscond_rally(u32 unused, u32 player, Unit* unit);		// 00429520

	Order* remove_references(Unit* unit, Unit* dying);			// 0049E590
	Unit* right_click_command(signed int x, signed int y, Unit *unit, signed int fow_unit_id, char queued); // 004560D0

	void inject_rally_hooks();
}
