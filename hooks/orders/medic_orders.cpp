#include "medic_orders.h"
#include <SCBW/api.h>
#include "globals.h"
#include <MathUtils.h>

namespace hooks
{
	void orders_MedicHeal2(Unit* unit)
	{
		if (!(unit->movementFlags & MovementFlags::Accelerating))
		{
			Image* current_image = unit->sprite->images.head;
			while (current_image != nullptr)
			{
				current_image->playIscriptAnim(IscriptAnimation::WalkingToIdle);
				current_image = current_image->link.next;
			}
		}
		helper::actUnitReturnToIdle(unit);
	}

	void orders_HealMove(Unit* unit)
	{
		bool bStopThere = false;
		Unit* target = unit->orderTarget.unit;
		Unit* oldTarget = unit->orderTarget.unit;

		if (target != nullptr)
		{
			if (helper::AI_OrderHeal(unit, target) != 0)
			{
				unit->orderComputerCL(OrderId::MedicHeal1);
				unit->orderTarget.unit = target;
				unit->orderTarget.pt.x = target->sprite->position.x;
				unit->orderTarget.pt.y = target->sprite->position.y;
				helper::moveToTarget(target, unit);
				bStopThere = true;
			}
			else
			{ // 00463801
				unit->orderTarget.pt.x = target->sprite->position.x;
				unit->orderTarget.pt.y = target->sprite->position.y;
			}
		}

		if (!bStopThere)
		{ // 00463817
			if (unit->recentOrderTimer == 0)
			{
				if (!helper::SetMoveTarget_xy(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y))
				{
					bStopThere = true;
				}
			}

			// 00463842
			if (!bStopThere)
			{
				if (unit->moveTarget.pt.x == unit->sprite->position.x && unit->moveTarget.pt.y == unit->sprite->position.y)
				{
					unit->orderToIdle();
				}
				else
					if (unit->mainOrderTimer == 0)
					{
						Unit* newTarget;
						Point16 oldPos;
						unit->mainOrderTimer = 15;
						newTarget = helper::MedicHeal_TargetAcquire(unit);
						if (newTarget != nullptr)
						{
							oldPos.x = unit->orderTarget.pt.x;
							oldPos.y = unit->orderTarget.pt.y;
							helper::orderInterrupt(unit, unit->orderTarget.unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y, OrderId::MedicHeal1);

							helper::moveToTarget(newTarget, unit);
							helper::unitOrderMoveToTargetUnit(unit, newTarget);
							if (oldTarget != nullptr)
							{
								helper::function_474940(unit, oldTarget, OrderId::HealMove);
							}
							else
							{
								helper::function_4748E0(unit, oldPos.x, oldPos.y, OrderId::HealMove);
							}
						}
					}
			}
		}
	}

	// Idle/AI order probably
	void orders_Medic(Unit* unit)
	{
		bool bStopThere = false;
		if (unit->pAI != nullptr && helper::AI_CastSpellBehaviour(unit, NULL) != 0)
		{
			bStopThere = true;
		}
		if (!bStopThere)
		{
			Unit* target = helper::MedicHeal_TargetAcquire(unit);
			if (target != nullptr)
			{
				unit->userActionFlags |= 1;
				if (unit->mainOrderId != OrderId::Die)
				{
					bool bEndLoop = unit->orderQueueTail == nullptr;
					while (!bEndLoop)
					{

						if (!orders_dat::CanBeInterrupted[unit->orderQueueTail->orderId]
							&& unit->orderQueueTail->orderId != OrderId::MedicHeal1)
						{
							bEndLoop = true;
						}
						else
						{
							helper::removeOrderFromUnitQueue(unit, unit->orderQueueTail);
							bEndLoop = (unit->orderQueueTail == nullptr);
						}
					}
					unit->performAnotherOrder(OrderId::MedicHeal1, 0, 0, nullptr, UnitId::None);
				}
				// 00463991
				prepareForNextOrder(unit);
				unit->orderTarget.unit = target;
				unit->orderTarget.pt.x = target->sprite->position.x;
				unit->orderTarget.pt.y = target->sprite->position.y;
				helper::moveToTarget(target, unit);
			}
			else
			{ // 004639BD
				if (unit->pAI != nullptr)
				{
					if (*(u8*)((u32)unit->pAI + 8) != 1)
					{
						unit->orderComputerCL(OrderId::ComputerAI);
					}
					else
						if (unit->sprite->position.x != *(u16*)((u32)unit->pAI + 0x12)
							|| unit->sprite->position.y != *(u16*)((u32)unit->pAI + 0x14))
						{
							unit->orderTo(OrderId::ComputerReturn, *(u16*)((u32)unit->pAI + 0x12), *(u16*)((u32)unit->pAI + 0x14));
						}
				}
			}
		}
	}

	// void orders_MedicHoldPosition(Unit* unit)
	// in base_orders/stopholdpos_orders.cpp

	u32 doMedicHeal_Func(Unit* cleric, Unit* target) // 00463C40
	{
		if (cleric->canUseTech(TechId::Healing, cleric->playerId))
		{
			if (target && helper::AI_OrderHeal(cleric, target))
			{
				Unit* acquired = helper::MedicHeal_TargetAcquire(cleric);
				if (acquired)
				{
					helper::unitOrderMoveToTargetUnit(cleric, acquired);
					helper::moveToTarget(acquired, cleric);
					if (!cleric->isTargetWithinMinRange(acquired, 30))
					{
						return 3;
					}
					scbw::decloakProc(cleric);
					target->heal(ClericHealValue, cleric);
					return 1;
				}
			}
		}
		return 0;
	}

	void orders_MedicHeal1(Unit* unit)
	{
		Unit* target = unit->orderTarget.unit;
		if (target == nullptr)
		{
			return;
		}
		bool reqHeal = false;
		if (target->hitPoints < units_dat::MaxHitPoints[target->id])
		{
			reqHeal = true;
		}
		u32 doMedicHeal_result = doMedicHeal_Func(unit, target);
		if (doMedicHeal_result == 0)
		{
			helper::setNextWaypoint(unit);
			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else if (unit->pAI != nullptr)
			{
				unit->orderComputerCL(OrderId::ComputerAI);
			}
			else
			{
				unit->orderComputerCL(units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
		else // 004641D3
		{
			if (unit->mainOrderState == 0)
			{
				if (target != nullptr
					&& helper::moveTowards(target, unit) != 0
					&& unit->getMovableState() != 2) // unit not unmovable
				{
					if (unit->isTargetWithinMinRange(target, 30))
					{
						helper::setNextWaypoint(unit);
						unit->mainOrderState = 1;
					}
				}
				else // 00464216
				{
					unit->orderToIdle();
				}
			}
			else // 0046421E
			{
				if (doMedicHeal_result == 1)
				{
					if (!(unit->orderSignal & 1))
					{
						Image* current_image = unit->sprite->images.head;
						while (current_image != nullptr)
						{
							current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
							current_image = current_image->link.next;
						}
						if (unit->orderQueueHead == nullptr
							|| unit->orderQueueHead->orderId != OrderId::MedicHeal2)
						{
							helper::orderImmediate(unit, OrderId::MedicHeal2);
						}
					}

					// 0046425B
					target = unit->orderTarget.unit;
					if (target != nullptr
						&& (target->position.x != unit->nextTargetWaypoint.x || target->position.y != unit->nextTargetWaypoint.y))
					{
						unit->nextTargetWaypoint.x = target->position.x;
						unit->nextTargetWaypoint.y = target->position.y;
					}
				}
				else // 00464283
				{
					unit->mainOrderState = 0;
				}
			}
		}
	}

	bool AI_AcquireHealTarget(Unit* medic, Unit* target)
	{
		if (target == nullptr || target == medic || medic->status & UnitStatus::IsHallucination)
		{
			return 0;
		}
		UnitExt* targetExt = globals.GetUnitExt(target);
		s32 type = target->id;
		s32 abilityFlags = units_dat::BaseProperty[type];
		s32 enemyCheck;
		bool result;
		if (abilityFlags & UnitProperty::Building
			|| target->status & UnitStatus::InAir
			|| !scbw::isAlliedTo(medic->playerId, target->playerId)
			|| target->playerId >= 8
			|| medic->playerId >= 8
			|| target->hitPoints >= units_dat::MaxHitPoints[type]
			|| target->isFrozen()
			|| targetExt->values[ValueLazarusNoHeal] == 1)
		{
			return 0;
		}
		return true;
	}
}