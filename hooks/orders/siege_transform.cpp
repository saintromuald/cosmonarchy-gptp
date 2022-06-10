#include "siege_transform.h"
#include <SCBW/api.h>
#include <cassert>

namespace hooks
{
	void ordersTankMode(Unit* unit) // 00464AE0
	{
		bool bStopThere = false;
		if (unit->main_order_state == 0)
		{
			if (unit->id == UnitId::TerranPhalanxSiegeMode)
			{
				if (!(unit->subunit->status & UnitStatus::NoBrkCodeStart))
				{
					unit->subunit->order_to(OrderId::Nothing3, unit);
					helper::unitSetRetreatPoint(unit->subunit, units_dat::StartDirection[unit->subunit->id]);
					helper::playSubunitIscript(unit, IscriptAnimation::SpecialState1);
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
			if (helper::getCloserToDestination_sub402BE0(unit->subunit))
			{
				Image* current_image = unit->sprite->images.head;
				while (current_image != nullptr)
				{
					current_image->play_iscript_animation(IscriptAnimation::SpecialState2);
					current_image = current_image->link.next;
				}
				helper::playSubunitIscript(unit, IscriptAnimation::SpecialState2);
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
				helper::replace_unit_with_type(unit, unitNewId);
				helper::orderReturnToIdle(unit);
				helper::orderReturnToIdle(unit->subunit);
			}
		}
	}
	
	void ordersSiegeMode(Unit* unit) // 00464BD0
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
						helper::unitSetRetreatPoint(unit, units_dat::StartDirection[unit->id]);
						unit->subunit->order_to(OrderId::Nothing3, unit);
						helper::unitSetRetreatPoint(unit->subunit, units_dat::StartDirection[unit->subunit->id]);
						helper::playSubunitIscript(unit, IscriptAnimation::SpecialState1);
						unit->main_order_state = 2;
						if (helper::checkForCloseUnit(unit))
						{
							bStopThere = true;
						}
					}
				}
			}
		}

		if (!bStopThere && unit->main_order_state == 2)
		{
			if (helper::getCloserToDestination_sub402BE0(unit))
			{
				if (helper::getCloserToDestination_sub402BE0(unit->subunit))
				{
					u16 unitNewId = unit->id;
					// identical to @00463310
					if (unit->id == UnitId::TerranPhalanxTankMode)
					{
						unitNewId = UnitId::TerranPhalanxSiegeMode;
					}
					else
					{
						unitNewId = unit->id;
					}
					helper::replace_unit_with_type(unit, unitNewId);
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
						unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
						helper::force_order_done(unit);
						helper::orderReturnToIdle(unit->subunit);
						bStopThere = true;
					}
				}
				if (!bStopThere)
				{
					helper::orderReturnToIdle(unit);
					helper::orderReturnToIdle(unit->subunit);
				}
			}
		}
	}
}