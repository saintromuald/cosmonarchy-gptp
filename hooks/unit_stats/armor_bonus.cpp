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
		switch (unit->id) {
			case UnitId::TerranMatadorCinderMode:
			{
				armor_modifier += 2;
			} break;
			case UnitId::TerranPhalanxTankMode:
			{
				armor_modifier += 4;
			} break;
			case UnitId::ProtossZealot:
			{
				if (unit->shields > 256) {
					armor_modifier += 1;
				}
			} break;
		}
		return armor_modifier;
	}

	s32 get_armor_internal(Unit* unit)
	{
		s32 armor = units_dat::armor_amount[unit->id] + get_armor_bonus(unit);
		UnitExt* unit_ext = get_unit_ext(unit);

		// Override base armor
		if (unit_ext->values[ValueTriggerArmorBase]) {
			armor = unit_ext->values[ValueTriggerArmorBase];
		}

		// Armor rends
		armor -= unit_ext->values[ValueApollyoidAdhesiveStacks];
		armor -= unit_ext->values[ValueAutocratControlledOppositionStacks];
		armor -= unit_ext->values[ValueObservanceRend];
		armor -= unit_ext->values[ValueCorrosiveTouchStacks];
		armor -= unit_ext->values[ValueWendigoShatteredWindsStacks];

		if (armor < 0) {
			armor = 0;
		}
		return armor;
	}
}