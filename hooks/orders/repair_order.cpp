#include "repair_order.h"
#include <SCBW/api.h>

namespace hooks {
	void orders_Repair1(CUnit* unit) {
		bool jump_to_676C4 = false; // handle various early invalid cases
		bool bEndThere = false;
		CUnit* target = unit->orderTarget.unit;
		if (target == nullptr) {
			jump_to_676C4 = true;
		}
		else if (target->hitPoints >= units_dat::MaxHitPoints[target->id]) {
			jump_to_676C4 = true;
		}
		else if (target->stasisTimer != 0) {
			jump_to_676C4 = true;
		}
		else if (unit->pAI != nullptr && target->status & UnitStatus::IsBuilding && target->orderTarget.unit != unit) {
			jump_to_676C4 = true;
		}
		else if (target->status & UnitStatus::InTransport) {
			jump_to_676C4 = true;
		}
		else if (!(target->status & UnitStatus::Completed)) {
			jump_to_676C4 = true;
		}

		if (!jump_to_676C4) {
			unit->sprite->playIscriptAnim(IscriptAnimation::WalkingToIdle, true);
			unit->orderToIdle();
			bEndThere = true;
		}

		if (!bEndThere) {
			bool jump_to_default_switch;
			if (!jump_to_676C4) {
				jump_to_default_switch = ((unit->mainOrderState > 8) || (unit->mainOrderState >= 2 && unit->mainOrderState <= 5));
				if (!jump_to_default_switch && unit->mainOrderState == 0) {
					int16 unkRepairInfo = 0;
					uint32 repairCostMinerals = 0, repairCostGas = 0;

					helper::getRepairInfo(target, &repairCostMinerals, &repairCostGas, &unkRepairInfo);
					helper::verifyResources(repairCostMinerals, repairCostGas, unit->playerId);

					unit->worker.repairResourceLossTimer = 0;
					unit->mainOrderState = 1; // continue directly into this case
				}

				if (!jump_to_default_switch && unit->mainOrderState == 1) {
					if (!(target->status & UnitStatus::Completed)) {
						unit->orderTo(OrderId::ConstructingBuilding, target);
						if (*IS_PLACING_BUILDING) {
							if (!helper::canStillPlaceBuilding()) {
								helper::refreshLayers3And4();
								helper::endBuildingPlacement();
							}
						}
						bEndThere = true;
					} else {
						if (helper::orderToMoveToTarget(unit, target)) {
							helper::unitOrderMoveToTargetUnit(unit, target);
							unit->mainOrderState = 6; // don't continue directly into this case
						}
						bEndThere = true;
					}
				}

				if (!bEndThere && !jump_to_default_switch && unit->mainOrderState == 6) {
					if (unit->getMovableState() == 0) { // not reached destination
						helper::function_004EB900(unit, target); // may cause unit to move or become idle
						bEndThere = true;
					} else { // reached destination or unmovable
						if (!(unit->isTargetWithinMinRange(target, 5))) {
							unit->mainOrderState = 1; // don't continue directly into this case
							bEndThere = true;
						} else {
							helper::function_00494BB0(unit, target->sprite->position.x, target->sprite->position.y);
							helper::unitOrderMoveToTargetUnit(unit, target);
							unit->mainOrderState = 7; // don't continue directly into this case
							jump_to_default_switch = true;
						}
					}
				}

				if (!bEndThere && !jump_to_default_switch && unit->mainOrderState == 7) {
					if (!(unit->isTargetWithinMinRange(target, 5))) {
						unit->mainOrderState = 1;
						bEndThere = true;
					} else {
						helper::function_00494BB0(unit, target->sprite->position.x, target->sprite->position.y);
						helper::unitOrderMoveToTargetUnit(unit, target);
						if (!helper::isUnitPositions2Equal(unit)) {
							int32 angleReturned, currentAngle;
							uint8 weaponId;
							angleReturned =
								scbw::getAngle(
									unit->sprite->position.x,
									unit->sprite->position.y,
									unit->nextTargetWaypoint.x,
									unit->nextTargetWaypoint.y
								);
							currentAngle = unit->currentDirection1 - angleReturned;
							if (currentAngle < 0) {
								currentAngle += 256;
							}
							if (currentAngle > 128) {
								currentAngle = 256 - currentAngle;
							}
							weaponId = unit->getGroundWeapon();
							if (currentAngle > weapons_dat::AttackAngle[weaponId]) {
								jump_to_default_switch = true;
							}
							else {
								unit->sprite->playIscriptAnim(IscriptAnimation::AlmostBuilt, true);
								unit->mainOrderState = 8; // don't continue directly into this case
								jump_to_default_switch = true;
							}
						}
						else {
							unit->sprite->playIscriptAnim(IscriptAnimation::AlmostBuilt, true);
							unit->mainOrderState = 8; // don't continue directly into this case
							jump_to_default_switch = true;
						}
					}
				}

				if (!bEndThere && !jump_to_default_switch && unit->mainOrderState == 8) {
					helper::function_00494BB0(unit, target->sprite->position.x, target->sprite->position.y);
					helper::unitOrderMoveToTargetUnit(unit, target);
					if (unit->isTargetWithinMinRange(target, 5) && target->mainOrderId != OrderId::Liftoff) {
						jump_to_default_switch = true;
					}
					else {
						CImage* current_image = unit->sprite->images.head;
						while (current_image != nullptr) {
							current_image->playIscriptAnim(IscriptAnimation::GndAttkToIdle);
							current_image = current_image->link.next;
						}
						unit->mainOrderState = 1; // don't continue directly into this case
						jump_to_default_switch = true;
					}
				}

				if (!bEndThere) { // jump_to_default_switch CASE
					bool bDoRepair = false;
					if (unit->worker.repairResourceLossTimer == 0) {
						unit->worker.repairResourceLossTimer--;
						bDoRepair = helper::ord_repair_subtract(unit, target);
						if (!bDoRepair) {
							CImage* current_image = unit->sprite->images.head;
							while (current_image != nullptr) {
								current_image->playIscriptAnim(IscriptAnimation::WalkingToIdle);
								current_image = current_image->link.next;
							}
							unit->orderToIdle();
							bEndThere = true;
						}
					} else {
						unit->worker.repairResourceLossTimer--;
						bDoRepair = true;
					}
					if (bDoRepair) { // //67692
						uint32 hpGain = target->buildRepairHpGain;
						if (*CHEAT_STATE & CheatFlags::OperationCwal) {
							hpGain *= 16;
						}
						// Talos - Advanced repair speed
						/*
						if (unit->id == UnitId::TerranTalos) {
							hpGain *= 1.5;
						}
						*/
						target->heal(hpGain, unit);
						if (target->hitPoints < units_dat::MaxHitPoints[target->id]) {
							bEndThere = true;
						}
						else {
							jump_to_676C4 = true;
						}
					}
				}
			}

			if (!bEndThere) { //jump_to_676C4 CASE
				CImage* current_image = unit->sprite->images.head;
				while (current_image != nullptr) {
					current_image->playIscriptAnim(IscriptAnimation::WalkingToIdle);
					current_image = current_image->link.next;
				}
				if (unit->orderQueueHead != nullptr) {
					unit->userActionFlags |= 1;
					prepareForNextOrder(unit);
				} else { //676FE
					uint8 orderId;
					if (unit->pAI == nullptr) {
						orderId = units_dat::ReturnToIdleOrder[unit->id];
					}
					else {
						orderId = OrderId::ComputerAI;
					}
					unit->orderComputerCL(orderId);
				}
				//6771B
				if (target != nullptr && !(target->status & UnitStatus::InTransport)) {
					target->connectedUnit = nullptr;
				}
			}
		}
	}
}