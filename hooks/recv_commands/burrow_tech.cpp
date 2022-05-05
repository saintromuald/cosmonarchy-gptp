#include "burrow_tech.h"
#include <SCBW/api.h>

namespace hooks
{
	void CMDRECV_Unburrow()
	{
		Unit* selected_unit;
		*selectionIndexStart = 0;
		selected_unit = getActivePlayerNextSelection();
		while (selected_unit != nullptr)
		{
			if (units_dat::BaseProperty[selected_unit->id] & UnitProperty::Burrowable
				&& selected_unit->canUseTech(TechId::Burrowing, *ACTIVE_NATION_ID) == 1)
			{
				helper::function_004E97C0(selected_unit);
			}
			selected_unit = getActivePlayerNextSelection();
		}
	}

	void CMDRECV_Burrow(u8 bCommandType)
	{
		Unit* selected_unit;
		*selectionIndexStart = 0;
		selected_unit = getActivePlayerNextSelection();
		while (selected_unit != nullptr)
		{
			if (units_dat::BaseProperty[selected_unit->id] & UnitProperty::Burrowable
				&& selected_unit->canUseTech(TechId::Burrowing, *ACTIVE_NATION_ID) == 1
				&& !(selected_unit->status & UnitStatus::Burrowed)
				&& selected_unit->mainOrderId != OrderId::Burrow)
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
			selected_unit = getActivePlayerNextSelection();
		}
	}

	void Unburrow_Proc(Unit* unit)
	{
		if (unit->mainOrderId == OrderId::Burrow
			|| unit->mainOrderId == OrderId::Burrowed
			|| unit->id == UnitId::ZergLakizilisk)
		{
			if (unit->id == UnitId::ZergLakizilisk)
			{
				unit->status |= UnitStatus::CanNotReceiveOrders;
			}

			unit->userActionFlags |= 1;
			if (unit->mainOrderId != OrderId::Die)
			{
				bool bStopLoop = false;
				while (unit->orderQueueTail != nullptr && !bStopLoop)
				{
					if (!orders_dat::CanBeInterrupted[unit->orderQueueTail->orderId]
						&& unit->orderQueueTail->orderId != OrderId::Unburrow)
					{
						bStopLoop = true;
					}
					else
					{
						helper::remove_order_from_unit_queue(unit->orderQueueTail, unit);
					}
				}
				unit->performAnotherOrder(OrderId::Unburrow, 0, 0, nullptr, UnitId::None);
			}
			// 004E9846
			prepareForNextOrder(unit);
			helper::actUnitReturnToIdle(unit);
		}
	}
}