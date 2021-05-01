#include "siege_transform.h"
#include <SCBW/api.h>
#include <cassert>

namespace hooks {
	//00464AE0
	void ordersTankMode(CUnit* unit) {
		bool bStopThere = false;
		if (unit->mainOrderState == 0) {
			if (unit->id == UnitId::TerranPhalanxSiegeMode || unit->id == UnitId::Hero_EdmundDukeSiegeMode) {
				if (!(unit->subunit->status & UnitStatus::NoBrkCodeStart)) {
					unit->subunit->orderTo(OrderId::Nothing3, unit);
					helper::unitSetRetreatPoint(unit->subunit, units_dat::StartDirection[unit->subunit->id]);
					helper::playSubunitIscript(unit, IscriptAnimation::SpecialState1);
					unit->mainOrderState = 2;
				}
			} else {
				bStopThere = true;
				unit->orderToIdle();
			}
		} //unit->mainOrderState == 0

		if (!bStopThere && unit->mainOrderState == 2) {

			if (helper::getCloserToDestination_sub402BE0(unit->subunit)) {

				CImage* current_image = unit->sprite->images.head;

				while (current_image != nullptr) {
					current_image->playIscriptAnim(IscriptAnimation::SpecialState2);
					current_image = current_image->link.next;
				}

				helper::playSubunitIscript(unit, IscriptAnimation::SpecialState2);
				unit->orderSignal &= ~0x01; //if bit 1 is set, unset it
				unit->mainOrderState = 3;

			}

		} //unit->mainOrderState == 2

		if (!bStopThere && unit->mainOrderState == 3) {

			if (unit->orderSignal & 1) {

				uint16 unitNewId;

				unit->orderSignal -= 1;

				if (unit->id == UnitId::Hero_EdmundDukeSiegeMode)
					unitNewId = UnitId::Hero_EdmundDukeTankMode; else
					if (unit->id == UnitId::TerranPhalanxSiegeMode)
						unitNewId = UnitId::TerranPhalanxTankMode;
					else
						unitNewId = unit->id;

				helper::replaceUnitWithType(unit, unitNewId);
				helper::orderReturnToIdle(unit);
				helper::orderReturnToIdle(unit->subunit);

			}

		} //unit->mainOrderState == 3


	} //ordersTankMode

	//00464BD0
	void ordersSiegeMode(CUnit* unit) {

		bool bStopThere = false;

		if (unit->mainOrderState == 0) {

			if (unit->id == UnitId::TerranPhalanxTankMode || unit->id == UnitId::Hero_EdmundDukeTankMode) {

				if (unit->movementFlags & MovementFlags::Accelerating)
					helper::makeToHoldPosition(unit);

				unit->mainOrderState = 1;

			} else {

				unit->orderToIdle();
				bStopThere = true;

			}

		} //unit->mainOrderState == 0

		if (!bStopThere && unit->mainOrderState == 1) {

			if (!(unit->movementFlags & MovementFlags::Accelerating)) {

				if (!(unit->subunit->movementFlags & MovementFlags::Accelerating)) {

					if (!(unit->subunit->status & UnitStatus::NoBrkCodeStart)) {

						helper::unitSetRetreatPoint(unit, units_dat::StartDirection[unit->id]);
						unit->subunit->orderTo(OrderId::Nothing3, unit);
						helper::unitSetRetreatPoint(unit->subunit, units_dat::StartDirection[unit->subunit->id]);
						helper::playSubunitIscript(unit, IscriptAnimation::SpecialState1);
						unit->mainOrderState = 2;

						if (helper::checkForCloseUnit_sub464290(unit))
							bStopThere = true;

					}

				}
			}

		} //unit->mainOrderState == 1

		if (!bStopThere && unit->mainOrderState == 2) {

			if (helper::getCloserToDestination_sub402BE0(unit)) {

				if (helper::getCloserToDestination_sub402BE0(unit->subunit)) {

					uint16 unitNewId = unit->id;

					//identical to @00463310
					if (unit->id == UnitId::TerranPhalanxTankMode)
						unitNewId = UnitId::TerranPhalanxSiegeMode;
					else
						if (unit->id == UnitId::Hero_EdmundDukeTankMode)
							unitNewId = UnitId::Hero_EdmundDukeSiegeMode;
						else
							unitNewId = unit->id;

					helper::replaceUnitWithType(unit, unitNewId);
					unit->mainOrderState = 3;

				}

			}

		} //unit->mainOrderState == 2

		if (!bStopThere && unit->mainOrderState == 3) {

			if (unit->orderSignal & 1) {

				unit->orderSignal -= 1;

				if (unit->orderQueueHead != nullptr) {

					if (!(unit->orderQueueHead->orderId == OrderId::WatchTarget)) {

						unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
						helper::actUnitReturnToIdle(unit);
						helper::orderReturnToIdle(unit->subunit);
						bStopThere = true;

					}

				}

				if (!bStopThere) {
					helper::orderReturnToIdle(unit);
					helper::orderReturnToIdle(unit->subunit);
				}

			}
		} //unit->mainOrderState == 3
	}
}