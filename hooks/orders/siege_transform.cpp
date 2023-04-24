#include "siege_transform.h"
#include <SCBW/api.h>
#include <cassert>

namespace hooks
{
	void orders_tank_mode(Unit* unit) // 00464AE0
	{
		bool stop_there = false;
		Unit* subunit = unit->subunit;
		if (unit->main_order_state == 0)
		{
			if (scbw::unit_id_is_siege_mode(unit->id))
			{
				unit->main_order_state = 2;
				scbw::decloak_proc(unit);
				if (subunit) {
					if (!(unit->subunit->status & UnitStatus::NoBrkCodeStart))
					{
						unit->subunit->order_to(OrderId::Nothing3, unit);
						helper::set_unit_retreat_point(unit->subunit, units_dat::start_direction[unit->subunit->id]);
						helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState1);
						unit->main_order_state = 2;
						scbw::decloak_proc(unit);
					}
				}
			} else {
				stop_there = true;
				unit->order_to_idle();
			}
		}

		if (!stop_there && unit->main_order_state == 2)
		{
			if ((subunit && helper::get_closer_to_destination(unit->subunit)) || helper::get_closer_to_destination(unit))
			{
				Image* current_image = unit->sprite->images.head;
				while (current_image)
				{
					current_image->play_iscript_animation(IscriptAnimation::SpecialState2);
					current_image = current_image->link.next;
				}
				if (subunit) {
					helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState2);
				}
				unit->order_signal &= ~0x01; // if bit 1 is set, unset it
				unit->main_order_state = 3;
			}
		}

		if (!stop_there && unit->main_order_state == 3)
		{
			if (unit->order_signal & 1)
			{
				u16 new_id = unit->id;
				unit->order_signal -= 1;
				switch (unit->id)
				{
					case UnitId::TerranMatadorCinderMode:
					{
						new_id = UnitId::TerranMatadorTankMode;
					} break;
					case UnitId::TerranPhalanxSiegeMode:
					{
						new_id = UnitId::TerranPhalanxTankMode;
					} break;
				}
				helper::replace_unit_with_id(unit, new_id);
				helper::order_return_to_idle(unit);
				if (subunit) {
					helper::order_return_to_idle(unit->subunit);
				}
			}
		}
	}
	
	void orders_siege_mode(Unit* unit) // 00464BD0
	{
		bool stop_there = false;
		if (unit->main_order_state == 0)
		{
			if (scbw::unit_id_is_tank_mode(unit->id))
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
				stop_there = true;
			}
		}

		Unit* subunit = unit->subunit;
		if (!stop_there && unit->main_order_state == 1)
		{
			if (!(unit->movement_flags & MovementFlags::Accelerating))
			{
				if (subunit) {
					if (!(subunit->movement_flags & MovementFlags::Accelerating))
					{
						if (!(subunit->status & UnitStatus::NoBrkCodeStart))
						{
							helper::set_unit_retreat_point(unit, units_dat::start_direction[unit->id]);
							subunit->order_to(OrderId::Nothing3, unit);
							helper::set_unit_retreat_point(subunit, units_dat::start_direction[subunit->id]);
							helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState1);
						}
					}
				} else {
					helper::set_unit_retreat_point(unit, units_dat::start_direction[unit->id]);
				}
				unit->main_order_state = 2;
				if (helper::check_for_close_unit(unit))
				{
					stop_there = true;
				}
			}
		}

		if (!stop_there && unit->main_order_state == 2)
		{
			if (helper::get_closer_to_destination(unit))
			{
				u16 new_id = unit->id;
				switch (unit->id)
				{
					case UnitId::TerranMatadorTankMode:
					{
						new_id = UnitId::TerranMatadorCinderMode;
					} break;
					case UnitId::TerranPhalanxTankMode:
					{
						new_id = UnitId::None;
						if (helper::get_closer_to_destination(subunit))
						{
							new_id = UnitId::TerranPhalanxSiegeMode;
						}
					} break;
				}
				if (new_id != UnitId::None) {
					helper::replace_unit_with_id(unit, new_id);
					unit->main_order_state = 3;
					scbw::decloak_proc(unit);
				}
			}
		}

		if (!stop_there && unit->main_order_state == 3)
		{
			if (unit->order_signal & 1)
			{
				unit->order_signal -= 1;
				if (unit->order_queue_head)
				{
					if (!(unit->order_queue_head->order_id == OrderId::WatchTarget))
					{
						unit->order_computer(units_dat::return_to_idle_order[unit->id]);
						helper::force_order_done(unit);
						if (subunit) {
							helper::order_return_to_idle(subunit);
						}
						stop_there = true;
					}
				}
				if (!stop_there)
				{
					helper::order_return_to_idle(unit);
					if (subunit) {
						helper::order_return_to_idle(subunit);
					}
				}
			}
		}
	}
}