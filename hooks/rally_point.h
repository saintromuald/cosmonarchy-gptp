#pragma once
#include "../SCBW/structures/Unit.h"

namespace hooks
{
	void setRallyUnit(Unit* unit, Unit* target);				// 00466B40
	void orderNewUnitToRallyHook(Unit* unit, Unit* factory);	// 00466F50
	void setRallyPosition(Unit* unit, u16 x, u16 y);			// 00466910
	u32 BTNSCOND_Rally(u32 unused, u32 player, Unit* unit);		// 00429520

	Order* RemoveReferences(Unit* unit, Unit* dying); // 0049E590
	Unit* RightClickCommand(signed int x, signed int y, Unit *unit, signed int fow_unit_id, char queued); // 004560D0

	void injectRallyHooks();
}
