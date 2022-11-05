#include "siege_transform.h"
#include <SCBW/api.h>
#include <cassert>

namespace hooks
{
	void orders_tank_mode(Unit* unit) // 00464AE0
	{
		bool bStopThere = false;
		if (unit->main_order_state == 0)
		{
			if (unit->id == UnitId::TerranPhalanxSiegeMode)
			{
				if (!(unit->subunit->status & UnitStatus::NoBrkCodeStart))
				{
					unit->subunit->order_to(OrderId::Nothing3, unit);
					helper::set_unit_retreat_point(unit->subunit, units_dat::start_direction[unit->subunit->id]);
					helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState1);
					unit->main_order_state = 2;
					scbw::decloak_proc(unit);
				}
			}
			else
			{
				bStopThere = true;
				unit->order_to_idle();
			}
		}

		if (!bStopThere && unit->main_order_state == 2)
		{
			if (helper::get_closer_to_destination(unit->subunit))
			{
				Image* current_image = unit->sprite->images.head;
				while (current_image != nullptr)
				{
					current_image->play_iscript_animation(IscriptAnimation::SpecialState2);
					current_image = current_image->link.next;
				}
				helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState2);
				unit->order_signal &= ~0x01; // if bit 1 is set, unset it
				unit->main_order_state = 3;
			}
		}

		if (!bStopThere && unit->main_order_state == 3)
		{
			if (unit->order_signal & 1)
			{
				u16 unitNewId;
				unit->order_signal -= 1;
				if (unit->id == UnitId::TerranPhalanxSiegeMode)
				{
					unitNewId = UnitId::TerranPhalanxTankMode;
				}
				else
				{
					unitNewId = unit->id;
				}
				helper::replace_unit_with_id(unit, unitNewId);
				helper::order_return_to_idle(unit);
				helper::order_return_to_idle(unit->subunit);
			}
		}
	}
	
	void orders_siege_mode(Unit* unit) // 00464BD0
	{
		bool bStopThere = false;
		if (unit->main_order_state == 0)
		{
			if (unit->id == UnitId::TerranPhalanxTankMode)
			{
				if (unit->movement_flags & MovementFlags::Accelerating)
				{
					helper::make_to_hold_position(unit);
				}
				unit->main_order_state = 1;
			}
			else
			{
				unit->order_to_idle();
				bStopThere = true;
			}
		}

		if (!bStopThere && unit->main_order_state == 1)
		{
			if (!(unit->movement_flags & MovementFlags::Accelerating))
			{
				if (!(unit->subunit->movement_flags & MovementFlags::Accelerating))
				{
					if (!(unit->subunit->status & UnitStatus::NoBrkCodeStart))
					{
						helper::set_unit_retreat_point(unit, units_dat::start_direction[unit->id]);
						unit->subunit->order_to(OrderId::Nothing3, unit);
						helper::set_unit_retreat_point(unit->subunit, units_dat::start_direction[unit->subunit->id]);
						helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState1);
						unit->main_order_state = 2;
						if (helper::check_for_close_unit(unit))
						{
							bStopThere = true;
						}
					}
				}
			}
		}

		if (!bStopThere && unit->main_order_state == 2)
		{
			if (helper::get_closer_to_destination(unit))
			{
				if (helper::get_closer_to_destination(unit->subunit))
				{
					u16 new_unit_id = unit->id;
					// identical to @00463310
					if (unit->id == UnitId::TerranPhalanxTankMode)
					{
						new_unit_id = UnitId::TerranPhalanxSiegeMode;
					}
					else
					{
						new_unit_id = unit->id;
					}
					helper::replace_unit_with_id(unit, new_unit_id);
					unit->main_order_state = 3;
					scbw::decloak_proc(unit);
				}
			}
		}

		if (!bStopThere && unit->main_order_state == 3)
		{
			if (unit->order_signal & 1)
			{
				unit->order_signal -= 1;
				if (unit->order_queue_head != nullptr)
				{
					if (!(unit->order_queue_head->order_id == OrderId::WatchTarget))
					{
						unit->order_computer(units_dat::return_to_idle_order[unit->id]);
						helper::force_order_done(unit);
						helper::order_return_to_idle(unit->subunit);
						bStopThere = true;
					}
				}
				if (!bStopThere)
				{
					helper::order_return_to_idle(unit);
					helper::order_return_to_idle(unit->subunit);
				}
			}
		}
	}
}