#include "medic_orders.h"
#include <SCBW/api.h>
#include "globals.h"
#include <MathUtils.h>

namespace hooks {
	void orders_MedicHeal2(CUnit* unit) {
		if (!(unit->movementFlags & MovementFlags::Accelerating)) {
			CImage* current_image = unit->sprite->images.head;
			while (current_image != nullptr) {
				current_image->playIscriptAnim(IscriptAnimation::WalkingToIdle);
				current_image = current_image->link.next;
			}
		}
		actUnitReturnToIdle(unit);
	}

	void orders_HealMove(CUnit* unit) {
		bool bStopThere = false;
		CUnit* target = unit->orderTarget.unit;
		CUnit* oldTarget = unit->orderTarget.unit;

		if (target != nullptr) {
			if (AI_OrderHeal(unit, target) != 0) {
				unit->orderComputerCL(OrderId::MedicHeal1);
				unit->orderTarget.unit = target;
				unit->orderTarget.pt.x = target->sprite->position.x;
				unit->orderTarget.pt.y = target->sprite->position.y;
				moveToTarget(unit, target);
				bStopThere = true;
			} else { //63801
				unit->orderTarget.pt.x = target->sprite->position.x;
				unit->orderTarget.pt.y = target->sprite->position.y;
			}

		}

		if (!bStopThere) { //63817

			if (unit->recentOrderTimer == 0) {
				if (!SetMoveTarget_xy(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y))
					bStopThere = true;
			}

			//63842
			if (!bStopThere) {

				if (unit->moveTarget.pt.x == unit->sprite->position.x &&
					unit->moveTarget.pt.y == unit->sprite->position.y
					)
					unit->orderToIdle();
				else
					if (unit->mainOrderTimer == 0) {

						CUnit* newTarget;
						Point16 oldPos;

						unit->mainOrderTimer = 15;

						newTarget = MedicHeal_TargetAcquire(unit);

						if (newTarget != nullptr) {

							oldPos.x = unit->orderTarget.pt.x;
							oldPos.y = unit->orderTarget.pt.y;

							orderInterrupt(
								unit,
								unit->orderTarget.unit,
								unit->orderTarget.pt.x,
								unit->orderTarget.pt.y,
								OrderId::MedicHeal1
							);

							moveToTarget(unit, newTarget);
							unitOrderMoveToTargetUnit(unit, newTarget);

							if (oldTarget != nullptr)
								function_474940(unit, oldTarget, OrderId::HealMove);
							else
								function_4748E0(unit, oldPos.x, oldPos.y, OrderId::HealMove);
						}

					}


			}

		}

	}

	// Idle/AI order probably
	void orders_Medic(CUnit* unit) {

		bool bStopThere = false;

		if (unit->pAI != nullptr && AI_CastSpellBehaviour(unit, NULL) != 0)
			bStopThere = true;
		if (!bStopThere) {

			CUnit* target = MedicHeal_TargetAcquire(unit);

			if (target != nullptr) {

				unit->userActionFlags |= 1;

				if (unit->mainOrderId != OrderId::Die) {

					bool bEndLoop = unit->orderQueueTail == nullptr;

					while (!bEndLoop) {

						if (!orders_dat::CanBeInterrupted[unit->orderQueueTail->orderId] &&
							unit->orderQueueTail->orderId != OrderId::MedicHeal1)
							bEndLoop = true;
						else {
							removeOrderFromUnitQueue(unit, unit->orderQueueTail);
							bEndLoop = (unit->orderQueueTail == nullptr);
						}

					}

					unit->performAnotherOrder(
						OrderId::MedicHeal1,
						0,
						0,
						nullptr,
						UnitId::None
					);

				}

				//63991
				prepareForNextOrder(unit);

				unit->orderTarget.unit = target;
				unit->orderTarget.pt.x = target->sprite->position.x;
				unit->orderTarget.pt.y = target->sprite->position.y;

				moveToTarget(unit, target);

			} else { //639BD

				if (unit->pAI != nullptr) {

					if (*(uint8*)((uint32)unit->pAI + 8) != 1)
						unit->orderComputerCL(OrderId::ComputerAI);
					else
						if (unit->sprite->position.x != *(uint16*)((uint32)unit->pAI + 0x12) ||
							unit->sprite->position.y != *(uint16*)((uint32)unit->pAI + 0x14)
							)
							unit->orderTo(
								OrderId::ComputerReturn,
								*(uint16*)((uint32)unit->pAI + 0x12),
								*(uint16*)((uint32)unit->pAI + 0x14)
							);

				}

			}

		}

	}

	//void orders_MedicHoldPosition(CUnit* unit)
	//in base_orders/stopholdpos_orders.cpp

	uint32 doMedicHeal_Func(CUnit* medic, CUnit* target) { // 00463C40
		auto v3 = medic;
		auto result = medic->canUseTech(TechId::Healing, medic->playerId);
		if (result) {
			auto v4 = target;
			if (target && AI_OrderHeal(v3, target)) {
				if (!target->isBeingHealed)
					goto LABEL_7;
				auto v5 = MedicHeal_TargetAcquire(v3);
				auto v6 = v5;
				if (v5) {
					unitOrderMoveToTargetUnit(v3, v5);
					moveToTarget(v6, v3);
					v4 = v6;
				LABEL_7:
					if (!medic->isTargetWithinMinRange(v4, 30)) {
						return 3;
					}
					target->heal(MathUtils::map((int)medic->hitPoints, 0, (int)medic->getMaxHpRaw(), 0, 200), medic);
					result = 1;
					target->isBeingHealed = 1;
					return result;
				}
			}
			result = 0;
		}
		return result;
	}

	void orders_MedicHeal1(CUnit* unit) {
		CUnit* target = unit->orderTarget.unit;
		uint32 doMedicHeal_result = 0;
		auto a = unit->hitPoints;
		bool reqHeal = false;
		if (target != nullptr && target->hitPoints < units_dat::MaxHitPoints[target->id]) {
			reqHeal = true;
		}
		doMedicHeal_result = doMedicHeal(unit, target);
		if (doMedicHeal_result == 0) {
			setNextWaypoint_Sub4EB290(unit);
			if (unit->orderQueueHead != nullptr) {
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			} else
				if (unit->pAI != nullptr)
					unit->orderComputerCL(OrderId::ComputerAI);
				else
					unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
		} else { //641D3
			if (unit->mainOrderState == 0) {
				if (target != nullptr &&
					function_004EB900(unit, target) != 0 &&
					unit->getMovableState() != 2 //unit not unmovable
					) {
					if (unit->isTargetWithinMinRange(target, 30)) {
						setNextWaypoint_Sub4EB290(unit);
						unit->mainOrderState = 1;
					}
				} else //64216
					unit->orderToIdle();
			} else { //6421E
				if (doMedicHeal_result == 1) {
					if (!(unit->orderSignal & 1)) {
						CImage* current_image = unit->sprite->images.head;
						while (current_image != nullptr) {
							current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
							current_image = current_image->link.next;
						}
						if (unit->orderQueueHead == nullptr ||
							unit->orderQueueHead->orderId != OrderId::MedicHeal2
							)
							orderImmediate(unit, OrderId::MedicHeal2);
					}

					//6425B
					target = unit->orderTarget.unit;

					if (target->position.x != unit->nextTargetWaypoint.x ||
						target->position.y != unit->nextTargetWaypoint.y
						) {
						unit->nextTargetWaypoint.x = target->position.x;
						unit->nextTargetWaypoint.y = target->position.y;
					}
				} else //64283
					unit->mainOrderState = 0;
			}
		}
	}

	bool AI_AcquireHealTarget(CUnit* medic, CUnit* target) {
		if (target == nullptr || target == medic || medic->status & UnitStatus::IsHallucination) {
			return 0;
		}
		int type = target->id;
		int abilityFlags = units_dat::BaseProperty[type];
		int enemyCheck;
		bool result;
		if	(abilityFlags & UnitProperty::Building
			|| target->status & UnitStatus::InAir
			|| !scbw::isAlliedTo(medic->playerId, target->playerId)
			|| target->playerId >= 8
			|| medic->playerId >= 8
			|| target->hitPoints >= units_dat::MaxHitPoints[type]
			|| target->isFrozen()
			|| scbw::get_generic_value(target, ValueId::LazarusNoHeal) == 1) {
			return 0;
		}
		return true;
	}
}