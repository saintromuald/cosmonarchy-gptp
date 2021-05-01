#pragma once
#include "CMDRECV_Morph.h"
#include <SCBW/api.h>

namespace hooks {
	void CMDRECV_BuildingMorph(uint16 wUnitType) {

		CUnit* building;
		*selectionIndexStart = 0;
		building = getActivePlayerNextSelection();
		if (
			building != nullptr &&
			getActivePlayerNextSelection() == nullptr &&
			building->id >= UnitId::ZergInfestedCommandCenter &&
			building->id <= 227 &&
			wUnitType >= UnitId::ZergInfestedCommandCenter &&
			wUnitType <= 227 &&
			building->canMakeUnit(wUnitType, *ACTIVE_NATION_ID) == 1 &&
			helper::HasMoneyCanMake(building, wUnitType) && //start construction if successful
			building->mainOrderId != OrderId::Morph2
			) {
			building->orderComputerCL(OrderId::Morph2);
			scbw::refreshConsole();
		}
	}

	void CMDRECV_UnitMorph(uint16 wUnitType) {
		CUnit* current_unit;
		*selectionIndexStart = 0;
		current_unit = getActivePlayerNextSelection();
		while (current_unit != nullptr) {

			bool bAllowMorph;
			/*
			if(
				current_unit->id == UnitId::ZergHydralisk &&
				wUnitType == UnitId::ZergLakizilisk
			)
				bAllowMorph = current_unit->canUseTech(TechId::LurkerAspect,*ACTIVE_NATION_ID) == 1;
			else*/
			if ((current_unit->id, true)
				) {

				bAllowMorph = current_unit->canMakeUnit(wUnitType, *ACTIVE_NATION_ID) == 1;
			}
			else {
				bAllowMorph = false;
			}
			if (
				bAllowMorph &&
				current_unit->mainOrderId != OrderId::Morph1 &&
				helper::HasMoneyCanMake(current_unit, wUnitType) // start construction if successful
				) {
				current_unit->userActionFlags |= 1;
				if (current_unit->mainOrderId != OrderId::Die) {
					bool bKeepLooping = true;
					while (bKeepLooping) {
						if (current_unit->orderQueueTail == nullptr)
							bKeepLooping = false;
						else
							if (orders_dat::CanBeInterrupted[current_unit->orderQueueTail->orderId] ||
								current_unit->orderQueueTail->orderId == OrderId::Morph1
								)
								helper::removeOrderFromUnitQueue(current_unit, current_unit->orderQueueTail);
							else
								bKeepLooping = false;
					}
					current_unit->performAnotherOrder(OrderId::Morph1, 0, 0, nullptr, UnitId::None, 0);
				}
				prepareForNextOrder(current_unit);
			}
			current_unit = getActivePlayerNextSelection();
		}
		scbw::refreshConsole();
	}
}