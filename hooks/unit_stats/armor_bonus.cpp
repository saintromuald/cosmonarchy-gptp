#include "armor_bonus.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	u8 get_armor_bonus(Unit* unit)
	{
		u8 bonus_armor = 0;

		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueSublimeShepherd])
		{
			bonus_armor += unit_ext->values[ValueSublimeShepherd];
		}
		if (unit->id == UnitId::ZergLakizilisk && unit->status & UnitStatus::Burrowed)
		{
			bonus_armor += 2;
		}
		return bonus_armor + scbw::get_upgrade_level(unit->player, units_dat::armor_upgrade[unit->id]);
	}
}