#include "siege_transform.h"
#include <SCBW/api.h>
#include <cassert>

namespace hooks
{
	void orders_tank_mode(Unit* unit) // 00464AE0
	{
		u16 unit_id = unit->id;
		if (!unit->main_order_state)
		{
			if (!unit_id_is_siege_mode(unit_id)) {
				unit->order_to_idle();
				return;
			}

			scbw::decloak_proc(unit);
			Unit* subunit = unit->subunit;
			if (unit->status & UnitStatus::NoBrkCodeStart || (subunit && subunit->status & UnitStatus::NoBrkCodeStart)) {
				return;
			}

			if (subunit) {
				subunit->order_to(OrderId::Nothing3, unit);
				helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState1);
				scbw::decloak_proc(subunit);
			}
			for (Image* image = unit->sprite->images.head; image; image = image->link.next) {
				image->play_iscript_animation(IscriptAnimation::SpecialState2);
			}
			if (subunit) {
				helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState2);
			}
			unit->order_signal &= ~1;
			++unit->main_order_state;
		}

		if (unit->main_order_state && unit->order_signal & 1) {
			unit->order_signal &= ~1;

			switch (unit_id)
			{
				case UnitId::TerranShamanVisionaryMode:
				{
					unit_id = UnitId::TerranShamanSupportMode;
				} break;
				case UnitId::TerranMatadorCinderMode:
				{
					unit_id = UnitId::TerranMatadorTankMode;
				} break;
				case UnitId::TerranPhalanxSiegeMode:
				{
					unit_id = UnitId::TerranPhalanxTankMode;
				} break;
			}

			scbw::transform_unit(unit, unit_id);
			helper::order_return_to_idle(unit);
			if (unit->subunit) {
				helper::order_return_to_idle(unit->subunit);
			}
		}
	}
	
	void orders_siege_mode(Unit* unit) // 00464BD0
	{
		u16 unit_id = unit->id;
		switch (unit->main_order_state)
		{
			case 0:
			{
				if (!unit_id_is_tank_mode(unit_id)) {
					unit->order_to_idle();
					return;
				}
				if (unit->movement_flags & MovementFlags::Accelerating) {
					helper::make_to_hold_position(unit);
				}
				++unit->main_order_state;
			}
			case 1:
			{
				if (unit->movement_flags & MovementFlags::Accelerating) {
					return;
				}
				Unit* subunit = unit->subunit;
				if (subunit && (subunit->movement_flags & MovementFlags::Accelerating || subunit->status & UnitStatus::NoBrkCodeStart))
				{
					return;
				}
				if (subunit) {
					subunit->order_to(OrderId::Nothing3, unit);
					helper::play_subunit_iscript(unit, IscriptAnimation::SpecialState1);
				}
				++unit->main_order_state;
				if (helper::check_for_close_unit(unit)) {
					return;
				}
			}
			case 2:
			{
				switch (unit_id)
				{
					case UnitId::TerranShamanSupportMode:
					{
						unit_id = UnitId::TerranShamanVisionaryMode;
						break;
					}
					case UnitId::TerranMatadorTankMode:
					{
						unit_id = UnitId::TerranMatadorCinderMode;
						break;
					}
					case UnitId::TerranPhalanxTankMode:
					{
						unit_id = UnitId::TerranPhalanxSiegeMode;
					} break;
				}
				scbw::transform_unit(unit, unit_id);
				++unit->main_order_state;
			}
			case 3:
			{
				if (unit->order_signal & 1)
				{
					Order* order_queue_head = unit->order_queue_head;
					unit->order_signal &= ~1;
					if (order_queue_head && !(order_queue_head->order_id == OrderId::WatchTarget)) {
						unit->order_computer(units_dat::return_to_idle_order[unit_id]);
						helper::force_order_done(unit);
					} else {
						helper::order_return_to_idle(unit);
					}

					if (unit->subunit) {
						helper::order_return_to_idle(unit->subunit);
					}
				}
			}
			default:
			{
				return;
			}
		}
	}
}