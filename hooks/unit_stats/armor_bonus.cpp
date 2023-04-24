#include "armor_bonus.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	u8 get_armor_bonus(Unit* unit) {
		u8 bonus_armor = 0;
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit_ext->values[ValueSublimeShepherd]) {
			bonus_armor += unit_ext->values[ValueSublimeShepherd];
		}
		switch (unit->id) {
			case UnitId::TerranMatadorCinderMode:
			{
				bonus_armor += 1;
			} break;
			case UnitId::TerranPhalanxTankMode:
			{
				bonus_armor += 4;
			} break;
		}
		return bonus_armor + scbw::get_upgrade_level(unit->player, units_dat::armor_upgrade[unit->id]);
	}
}