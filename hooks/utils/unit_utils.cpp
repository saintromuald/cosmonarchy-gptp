#include "unit_utils.h"
#include <BW/api.h>
#include <storm/storm.h>

// Those functions are less likely to be useful (especially if modified alone against as a part of a larger modification),
// or may cause more trouble if modified the wrong way, so they are put here a bit on the side.

namespace hooks
{	
bool unit_is_standard_and_movable(Unit* unit) { // 0047B770
	if (units_dat::base_property[unit->id] & UnitProperty::NeutralAccessories
	||	unit->status & UnitStatus::Frozen
	||	unit_id_is_trap(unit->id)
	||  unit_id_is_spell(unit->id)
	||	unit->id == UnitId::Special_PitDoor
	||	unit->id == UnitId::Special_RightPitDoor
	||	unit->id == UnitId::Special_UpperLevelDoor
	||	unit->id == UnitId::Special_RightUpperLevelDoor)
	{
		return false;
	}
	return true;
}
		
Unit* get_active_player_first_selection() // 0049A850
{
	Unit* unit;
	if (*g_unit_iterator_pos >= 12)
	{
		unit = nullptr;
	}
	else
	{
		s32 player = g_active_player_id;
		u8 index = *g_unit_iterator_pos;
		bool bStopLoop = false;
		while (!bStopLoop)
		{
			unit = players_selections->unit[player][index];
			if (unit == nullptr)
			{
				bStopLoop = true;
			}
			else if (unit->sprite != nullptr)
			{
				if (unit->main_order_id != OrderId::Die)
				{
					bStopLoop = true;
				}
				else if (unit->main_order_state != 1)
				{
					bStopLoop = true;
				}
			}

			if (!bStopLoop)
			{
				helper::remove_from_selections(unit);
			}
		}
		*g_unit_iterator_pos = *g_unit_iterator_pos + 1;
	}
	return unit;
}
}
