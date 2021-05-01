#include "burrow_tech.h"
#include <SCBW/api.h>

namespace hooks {
	void CMDRECV_Unburrow() {
		CUnit* selected_unit;
		*selectionIndexStart = 0;
		selected_unit = getActivePlayerNextSelection();
		while (selected_unit != nullptr) {
			if (
				units_dat::BaseProperty[selected_unit->id] & UnitProperty::Burrowable &&
				selected_unit->canUseTech(TechId::Burrowing, *ACTIVE_NATION_ID) == 1
				)
				function_004E97C0(selected_unit);
			selected_unit = getActivePlayerNextSelection();
		}
	}

	void CMDRECV_Burrow(uint8 bCommandType) {
		CUnit* selected_unit;
		*selectionIndexStart = 0;
		selected_unit = getActivePlayerNextSelection();
		while (selected_unit != nullptr) {
			if (
				units_dat::BaseProperty[selected_unit->id] & UnitProperty::Burrowable &&
				selected_unit->canUseTech(TechId::Burrowing, *ACTIVE_NATION_ID) == 1 &&
				!(selected_unit->status & UnitStatus::Burrowed) &&
				selected_unit->mainOrderId != OrderId::Burrow
				) {

				if (bCommandType != 0)
					bCommandType = 1;
				else
					bCommandType = 0;

				function_004754F0(
					selected_unit,
					UnitId::None,
					*(uint32*)(0x006D5C24),
					*(uint32*)(0x006D5C20),
					OrderId::Burrow,
					0,
					bCommandType,
					1
				);
			}
			selected_unit = getActivePlayerNextSelection();
		}
	}	

	void Unburrow_Proc(CUnit* unit) {

		if (unit->mainOrderId == OrderId::Burrow ||
			unit->mainOrderId == OrderId::Burrowed ||
			unit->id == UnitId::ZergLakizilisk
			) {

			if (unit->id == UnitId::ZergLakizilisk)
				unit->status |= UnitStatus::CanNotReceiveOrders;

			unit->userActionFlags |= 1;

			if (unit->mainOrderId != OrderId::Die) {

				bool bStopLoop = false;

				while (unit->orderQueueTail != nullptr && !bStopLoop) {
					if (
						!orders_dat::CanBeInterrupted[unit->orderQueueTail->orderId] &&
						unit->orderQueueTail->orderId != OrderId::Unburrow
						)
						bStopLoop = true;
					else
						removeOrderFromUnitQueue(unit, unit->orderQueueTail);
				}
				unit->performAnotherOrder(OrderId::Unburrow, 0, 0, nullptr, UnitId::None);
			}
			//E9846
			prepareForNextOrder(unit);
			actUnitReturnToIdle(unit);
		}
	}
}