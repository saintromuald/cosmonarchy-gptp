#include "armor_bonus.h"
#include <SCBW/api.h>
#include "globals.h"
#include <algorithm>
#include <cmath>

namespace hooks
{
	u8 get_armor_bonus(Unit* unit) // 00453FC0
	{
		s32 armor_modifier = scbw::get_upgrade_level(unit->player, units_dat::armor_upgrade[unit->id]);
		UnitExt* unit_ext = get_unit_ext(unit);
		switch (unit->id) {
			case UnitId::TerranMatadorCinderMode:
			{
				armor_modifier += 1;
			} break;
			case UnitId::TerranPhalanxTankMode:
			{
				armor_modifier += 4;
			} break;
		}

		// Negatives
		armor_modifier -= unit_ext->values[ValueApollyoidAdhesiveStacks];
		armor_modifier -= unit_ext->values[ValueControlledOpposition];

		return (u8)std::max(armor_modifier, 0);
	}
}