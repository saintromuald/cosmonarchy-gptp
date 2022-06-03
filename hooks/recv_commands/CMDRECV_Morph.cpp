#pragma once
#include "CMDRECV_Morph.h"
#include <SCBW/api.h>

namespace hooks
{
	void CMDRECV_BuildingMorph(u16 wUnitType)
	{

		Unit* building;
		*selectionIndexStart = 0;
		building = getActivePlayerNextSelection();
		if (
			building != nullptr &&
			getActivePlayerNextSelection() == nullptr &&
			building->id >= UnitId::ZergInfestedCommandCenter &&
			building->id <= 227 &&
			wUnitType >= UnitId::ZergInfestedCommandCenter &&
			wUnitType <= 227 &&
			building->can_make_unit(wUnitType, *ACTIVE_NATION_ID) == 1 &&
			helper::has_money_can_make(building, wUnitType) && //start construction if successful
			building->main_order_id != OrderId::Morph2
			)
		{
			building->order_computer(OrderId::Morph2);
			scbw::refresh_console();
		}
	}

	void CMDRECV_UnitMorph(u16 wUnitType)
	{
		Unit* current_unit;
		*selectionIndexStart = 0;
		current_unit = getActivePlayerNextSelection();
		while (current_unit != nullptr)
		{

			bool bAllowMorph;
			/*
			if(
				current_unit->id == UnitId::ZergHydralisk &&
				wUnitType == UnitId::ZergLakizilisk
			)
				bAllowMorph = current_unit->canUseTech(TechId::LurkerAspect,*ACTIVE_NATION_ID) == 1;
			else*/
			if ((current_unit->id, true)
				)
			{

				bAllowMorph = current_unit->can_make_unit(wUnitType, *ACTIVE_NATION_ID) == 1;
			}
			else
			{
				bAllowMorph = false;
			}
			if (
				bAllowMorph &&
				current_unit->main_order_id != OrderId::Morph1 &&
				helper::has_money_can_make(current_unit, wUnitType) // start construction if successful
				)
			{
				current_unit->userActionFlags |= 1;
				if (current_unit->main_order_id != OrderId::Die)
				{
					bool bKeepLooping = true;
					while (bKeepLooping)
					{
						if (current_unit->orderQueueTail == nullptr)
							bKeepLooping = false;
						else
							if (orders_dat::CanBeInterrupted[current_unit->orderQueueTail->orderId] ||
								current_unit->orderQueueTail->orderId == OrderId::Morph1
								)
								helper::remove_order_from_unit_queue(current_unit->orderQueueTail, current_unit);
							else
								bKeepLooping = false;
					}
					current_unit->perform_another_order(OrderId::Morph1, 0, 0, nullptr, UnitId::None, 0);
				}
				prepareForNextOrder(current_unit);
			}
			current_unit = getActivePlayerNextSelection();
		}
		scbw::refresh_console();
	}
}