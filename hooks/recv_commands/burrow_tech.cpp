#include "burrow_tech.h"
#include <SCBW/api.h>

namespace hooks
{
	void CMDRECV_Unburrow()
	{
		Unit* selected_unit;
		*unit_iterator_pos = 0;
		selected_unit = get_active_player_next_selection();
		while (selected_unit != nullptr)
		{
			if (units_dat::BaseProperty[selected_unit->id] & UnitProperty::Burrowable
				&& selected_unit->canUseTech(TechId::Burrowing, *ACTIVE_NATION_ID) == 1)
			{
				helper::function_004E97C0(selected_unit);
			}
			selected_unit = get_active_player_next_selection();
		}
	}

	void CMDRECV_Burrow(u8 bCommandType)
	{
		Unit* selected_unit;
		*unit_iterator_pos = 0;
		selected_unit = get_active_player_next_selection();
		while (selected_unit != nullptr)
		{
			if (units_dat::BaseProperty[selected_unit->id] & UnitProperty::Burrowable
				&& selected_unit->canUseTech(TechId::Burrowing, *ACTIVE_NATION_ID) == 1
				&& !(selected_unit->status & UnitStatus::Burrowed)
				&& selected_unit->main_order_id != OrderId::Burrow)
			{

				if (bCommandType != 0)
				{
					bCommandType = 1;
				}
				else
				{
					bCommandType = 0;
				}

				helper::TargetedOrderCopy(selected_unit, UnitId::None, *(u32*)(0x006D5C24), *(u32*)(0x006D5C20), OrderId::Burrow, 0, bCommandType, 1);
			}
			selected_unit = get_active_player_next_selection();
		}
	}

	void Unburrow_Proc(Unit* unit)
	{
		if (unit->main_order_id == OrderId::Burrow
			|| unit->main_order_id == OrderId::Burrowed
			|| unit->id == UnitId::ZergLakizilisk)
		{
			if (unit->id == UnitId::ZergLakizilisk)
			{
				unit->status |= UnitStatus::CanNotReceiveOrders;
			}

			unit->user_action_flags |= 1;
			if (unit->main_order_id != OrderId::Die)
			{
				bool bStopLoop = false;
				while (unit->order_queue_tail != nullptr && !bStopLoop)
				{
					if (!orders_dat::CanBeInterrupted[unit->order_queue_tail->orderId]
						&& unit->order_queue_tail->orderId != OrderId::Unburrow)
					{
						bStopLoop = true;
					}
					else
					{
						helper::remove_order_from_unit_queue(unit->order_queue_tail, unit);
					}
				}
				unit->perform_another_order(OrderId::Unburrow, 0, 0, nullptr, UnitId::None);
			}
			// 004E9846
			prepare_for_next_order(unit);
			helper::force_order_done(unit);
		}
	}
}