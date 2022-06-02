#include "armor_bonus.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	u8 getArmorBonusHook(Unit* unit)
	{
		u8 armorUpg = 0;

		UnitExt* unitExt = globals.get_unit_ext(unit);
		if (unitExt->values[ValueSublimeShepherd])
		{
			armorUpg += unitExt->values[ValueSublimeShepherd];
		}

		return armorUpg + scbw::get_upgrade_level(unit->player, units_dat::ArmorUpgrade[unit->id]);
	}

}
