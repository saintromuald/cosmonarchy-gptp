#include "stopholdpos_orders.h"
#include <SCBW/api.h>

namespace hooks
{
	void orders_MedicHoldPosition(Unit* unit)
	{
		u32 switchValue;
		if (unit->mainOrderState == 0)
		{
			//change move target destination to current position
			helper::setNextWaypoint(unit);
			if (
				unit->nextTargetWaypoint.x != unit->moveTarget.pt.x ||
				unit->nextTargetWaypoint.y != unit->moveTarget.pt.y
				)
			{
				unit->nextTargetWaypoint.x = unit->moveTarget.pt.x;
				unit->nextTargetWaypoint.y = unit->moveTarget.pt.y;
			}
			//6407F:
			unit->status |= UnitStatus::HoldingPosition;
			if (unit->subunit != nullptr)
				unit->subunit->status |= UnitStatus::HoldingPosition;
			unit->mainOrderState = 1;
			unit->sprite->playIscriptAnim(IscriptAnimation::WalkingToIdle, true);
		}
		//640AF

		//try healing current target, return a value depending
		//on what happened
		switchValue = helper::doMedicHeal(unit, unit->orderTarget.unit);

		if (switchValue == 3) //640C6
			unit->orderTarget.unit = nullptr;

		if (switchValue == 0 || switchValue == 2 || switchValue == 3) //640CD
			unit->sprite->playIscriptAnim(IscriptAnimation::WalkingToIdle, true);

		//640D9, continued into for values 0,2,3, and jumped into for values
		//above 3, thus only switchValue of 1 don't go through it
		if (switchValue != 1)
		{

			//if timer is 0, seek a target in range
			if (unit->mainOrderTimer == 0)
			{

				unit->mainOrderTimer = 15;
				unit->orderTarget.unit = helper::MedicHeal_TargetAcquire(unit);

				if (unit->orderTarget.unit != nullptr)
				{
					unit->orderTarget.pt.x = (unit->orderTarget.unit)->sprite->position.x;
					unit->orderTarget.pt.y = (unit->orderTarget.unit)->sprite->position.y;
				}

			}

		}
		else
		{ //64109 (switchValue == 1)

			if (!(unit->orderSignal & 1))
			{

				Image* current_image = unit->sprite->images.head;

				while (current_image != nullptr)
				{
					current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
					current_image = current_image->link.next;
				}

				if (
					unit->orderQueueHead == nullptr ||
					unit->orderQueueHead->orderId != OrderId::MedicHeal2
					) 	//6413D
					helper::order_immediate(unit, OrderId::MedicHeal2); //return to idle after heal

			}

			//64146
			if (
				(unit->orderTarget.unit)->sprite->position.x != unit->nextTargetWaypoint.x ||
				(unit->orderTarget.unit)->sprite->position.y != unit->nextTargetWaypoint.y
				)
			{
				unit->nextTargetWaypoint.x = (unit->orderTarget.unit)->sprite->position.x;
				unit->nextTargetWaypoint.y = (unit->orderTarget.unit)->sprite->position.y;
			}

		}

	} //void orders_MedicHoldPosition(Unit* unit)

	;

	void orders_ReaverStop(Unit* unit)
	{

		Unit* outHangarChild = unit->carrier.outHangarChild;
		Point16 pos;

		//change move target destination to current position
		helper::setNextWaypoint(unit);

		if (
			unit->moveTarget.pt.x != unit->nextTargetWaypoint.x ||
			unit->moveTarget.pt.y != unit->nextTargetWaypoint.y
			)
		{
			unit->nextTargetWaypoint.x = unit->moveTarget.pt.x;
			unit->nextTargetWaypoint.y = unit->moveTarget.pt.y;
		}

		//65500
		while (outHangarChild != nullptr)
		{

			outHangarChild->userActionFlags |= 1;

			if (outHangarChild->mainOrderId != OrderId::Die)
			{

				bool bNotRemovableOrder = false;

				while (outHangarChild->orderQueueTail != nullptr && !bNotRemovableOrder)
				{

					if (
						!orders_dat::CanBeInterrupted[outHangarChild->orderQueueTail->orderId] &&
						outHangarChild->orderQueueTail->orderId != OrderId::SelfDestrucing
						)
						bNotRemovableOrder = true;
					else
						helper::remove_order_from_unit_queue(outHangarChild->orderQueueTail, outHangarChild);

				}

				//65539
				outHangarChild->performAnotherOrder(
					OrderId::SelfDestrucing,
					0,
					0,
					nullptr,
					UnitId::None
				);

			}

			//65554
			prepareForNextOrder(outHangarChild);
			outHangarChild = outHangarChild->interceptor.hangar_link.prev;

		} //while(outHangarChild != nullptr)

		//65565

		unit->userActionFlags |= 1;

		if (unit->orderTarget.unit != nullptr)
		{
			pos.x = (unit->orderTarget.unit)->sprite->position.x;
			pos.y = (unit->orderTarget.unit)->sprite->position.y;
		}
		else
		{
			pos.x = 0;
			pos.y = 0;
		}

		//65598
		if (unit->mainOrderId != OrderId::Die)
		{

			bool bNotRemovableOrder = false;

			while (unit->orderQueueTail != nullptr && !bNotRemovableOrder)
			{

				if (
					!orders_dat::CanBeInterrupted[unit->orderQueueTail->orderId] &&
					unit->orderQueueTail->orderId != OrderId::Reaver
					)
					bNotRemovableOrder = true;
				else
					helper::remove_order_from_unit_queue(unit->orderQueueTail, unit);

			}

			//655C3
			unit->performAnotherOrder(
				OrderId::Reaver,
				pos.x,
				pos.y,
				unit->orderTarget.unit,
				UnitId::None
			);

		}

		//655DB
		prepareForNextOrder(unit);

	} //void orders_ReaverStop(Unit* unit)

	;

	void orders_Guard(Unit* unit)
	{

		unit->mainOrderTimer = (u8)helper::RandBetween(0, 15, 29);

		if (playerTable[unit->playerId].type == PlayerType::Computer)
		{

			//since details about pAI structure are unknown, have to use this
			u8* const pAI_subparameter = (u8*)(((s32)(unit->pAI)) + 8);

			unit->mainOrderId = OrderId::ComputerAI;

			if (unit->pAI == nullptr)
			{

				if (units_dat::BaseProperty[unit->id] & UnitProperty::Hero)
					unit->mainOrderId = OrderId::PlayerGuard;

			}
			else //75BD6
				if (*pAI_subparameter == 1)
					unit->mainOrderId = OrderId::GuardPost;

		}
		else
			unit->mainOrderId = OrderId::PlayerGuard;

	} //void orders_Guard(Unit* unit)

	;

	//NOT HOOKED (since most of the action happen in another function)
	//Just here to show that the true code is in function_004774A0, though
	//a few things happen before
	void orders_TurretGuard(Unit* unit)
	{

		if (
			unit->subunit->nextTargetWaypoint.x != unit->nextTargetWaypoint.x ||
			unit->subunit->nextTargetWaypoint.y != unit->nextTargetWaypoint.y
			)
		{
			unit->nextTargetWaypoint.x = unit->subunit->nextTargetWaypoint.x;
			unit->nextTargetWaypoint.y = unit->subunit->nextTargetWaypoint.y;
		}

		function_004774A0(unit);

	} //void orders_TurretGuard(Unit* unit)

	;

	//Guard related function, may be a poorly identified official guard order
	void function_004774A0(Unit* unit)
	{

		//if unit->autoTargetUnit != nullptr, attack this target
		//then disable the unit...I think
		//That's why if it return true, nothing else is done
		//since has already attacked
		if (!helper::function_004770E0(unit))
		{

			if (unit->mainOrderTimer == 0)
			{

				unit->mainOrderTimer = 15;

				if (units_dat::BaseProperty[unit->id] & UnitProperty::Subunit)
				{
					if (
						unit->subunit->nextTargetWaypoint.x != unit->nextTargetWaypoint.x ||
						unit->subunit->nextTargetWaypoint.y != unit->nextTargetWaypoint.y
						)
					{
						unit->nextTargetWaypoint.x = unit->subunit->nextTargetWaypoint.x;
						unit->nextTargetWaypoint.y = unit->subunit->nextTargetWaypoint.y;
					}
				}

				if (unit->getSeekRange() != 0)
				{

					Unit* target = helper::findBestAttackTarget(unit);

					if (target != nullptr)
						//attack target I think, also used by function_004770E0
						helper::order_to_attack_target(unit, target, 0, 1);

				}
			}
		}
	}
}