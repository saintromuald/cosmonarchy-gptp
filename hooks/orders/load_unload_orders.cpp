#include "load_unload_orders.h"
#include <SCBW/api.h>
#include <logger.h>
#include "globals.h"
#include "hooks\limits\ccmu.h"
#define WRITE_TO_LOG(x)GPTP::logger<<x<<std::endl

namespace hooks
{
	// unit is a transport (overlord, shuttle, dropship)
	// this is the initial pickup order
	void orders_Pickup1(Unit* unit)
	{
		Unit* unitToLoad = unit->orderTarget.unit;
		if (unitToLoad != nullptr
			&& scbw::canBeEnteredBy(unit, unitToLoad))
		{
			helper::prependOrderTargetingUnit(unit, unitToLoad, OrderId::Pickup2);
			prepareForNextOrder(unit);
		}
		else if (unit->pAI != nullptr)
		{
			Point16 pos = {0,0};
			unit->userActionFlags |= 1;
			if (unit->mainOrderId != OrderId::Die)
			{
				bool bStopLoop = (unit->orderQueueTail == nullptr);
				while (!bStopLoop)
				{
					u8 orderId = (u8)unit->orderQueueTail->orderId;
					if (orders_dat::CanBeInterrupted[orderId] == 0
						&& orderId != OrderId::ComputerAI)
					{
						bStopLoop = true;
					}

					if (!bStopLoop)
					{
						helper::remove_order_from_unit_queue(unit->orderQueueTail, unit);
						bStopLoop = (unit->orderQueueTail == nullptr);
					}
				}
				unit->performAnotherOrder(OrderId::ComputerAI, 0, 0, nullptr, UnitId::None);
			}
			prepareForNextOrder(unit);
		}
	}

	// unit is a non-mobile transport (bunker)
	void orders_Pickup3_0(Unit* unit)
	{
		Unit* unitToLoad = unit->orderTarget.unit;
		if (unitToLoad == nullptr)
		{
			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{
				if (unit->pAI != nullptr)
				{
					unit->orderComputer(OrderId::ComputerAI);
				}
				else
				{
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
		else
		{ // 004E73F4
			if (scbw::canBeEnteredBy(unit, unitToLoad))
			{
				unitToLoad->orderTo(OrderId::EnterTransport, unit);
			}
			unit->orderToIdle();
		}
	}

	// unit is a transport (overlord,shuttle,dropship)
	// this is used after orders_Pickup1
	void orders_Pickup2(Unit* unit)
	{
		Unit* unitToLoad = unit->orderTarget.unit;
		if (unitToLoad == nullptr)
		{
			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{
				if (unit->pAI != nullptr)
				{
					unit->orderComputer(OrderId::ComputerAI);
				}
				else
				{
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
		else
		{ // 004E761F
			if (unitToLoad->pAI == nullptr
				|| unitToLoad->mainOrderId == OrderId::Pickup4)
			{ // 004E762F
				if (unit->mainOrderState == 0)
				{
					if (scbw::canBeEnteredBy(unit, unitToLoad))
					{
						if (unit->mainOrderTimer == 0)
						{
							unit->mainOrderTimer = 7;
							if (!unit->isTargetWithinMinRange(unitToLoad, 1))
							{
								helper::orderToMoveToTarget(unit, unitToLoad);
							}
							else
							{
								helper::makeToHoldPosition(unit);
								if (unitToLoad->mainOrderId != OrderId::EnterTransport
									&& unitToLoad->mainOrderId != OrderId::Pickup4)
								{
									unitToLoad->orderTo(OrderId::EnterTransport, unit);
								}
								if (unit->pAI != nullptr)
								{
									unit->mainOrderState = 1;
								}
								else
								{
									helper::prependOrderTargetingUnit(unit, unitToLoad, OrderId::Pickup1);
									prepareForNextOrder(unit);
								}
							}
						}
					}
					else
					{
						unit->orderToIdle();
					}
				}
			}
			else // 004E7642
			{
				unit->orderToIdle();
			}
		}
	}

	// unit is a transport
	// a big part of the code could not be tested since different ground height test never occured during testing
	void orders_MoveUnload(Unit* unit)
	{
		u32 distance;
		if (unit->mainOrderState == 0 &&
			unit->orderTarget.unit != nullptr)
		{
			unit->mainOrderState = 1;
			unit->orderTarget.pt.x = (unit->orderTarget.unit)->sprite->position.x;
			unit->orderTarget.pt.y = (unit->orderTarget.unit)->sprite->position.y;
			unit->orderTarget.unit = nullptr;
		}

		// 004E7739
		distance = scbw::getDistanceFast(unit->orderTarget.pt.x * 256, unit->orderTarget.pt.y * 256, unit->halt.x, unit->halt.y);
		distance &= 0xFFFFFF00;
		if (distance <= 0x1000)
		{
			if (unit->orderQueueHead != nullptr)
			{
				helper::unknownSpecialOrder(unit, unit->orderQueueHead, OrderId::Unload, 0, 0, nullptr, UnitId::None);
				prepareForNextOrder(unit);
			}
			else
			{
				unit->performAnotherOrder(OrderId::Unload, 0, 0, nullptr, UnitId::None);
				prepareForNextOrder(unit);
			}
		}
		else
		{ // 004E77B4
			helper::SetMoveTarget_xy(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
			if (unit->orderTarget.pt.x != unit->nextTargetWaypoint.x
				|| unit->orderTarget.pt.y != unit->nextTargetWaypoint.y)
			{
				unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
				unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
			}
			if (unit->pAI != nullptr)
			{
				u32 groundHeightAtPxPos1 = scbw::getGroundHeightAtPos(unit->sprite->position.x, unit->sprite->position.y);
				if (groundHeightAtPxPos1 != scbw::getGroundHeightAtPos(unit->orderTarget.pt.x, unit->orderTarget.pt.y))
				{
					distance = scbw::getDistanceFast(unit->orderTarget.pt.x * 256, unit->orderTarget.pt.y * 256, unit->halt.x, unit->halt.y);
					distance &= 0xFFFFFF00;
					if (distance <= 0xCA000
						&& helper::Ai_AreOnConnectedRegions(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y) != 0
						&& helper::getFirstLoadedWorker(unit) == 0)
					{
						s32 index = 0;
						bool bStopLoop = false;
						bool bStopAll = false;
						bool bIsDistanceGreaterThanHaltDistance = helper::isDistanceGreaterThanHaltDistance(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y, 512);
						Point16 pos = {0,0}; // unsure about this start value
						while (!bStopLoop)
						{
							Unit* unitToUnload = helper::getLoadedUnitFromIndex(unit, index);
							if (unitToUnload != nullptr)
							{
								if (unitToUnload->id == UnitId::ProtossAccantor && bIsDistanceGreaterThanHaltDistance)
								{
									bStopLoop = true;
								}
								else if (helper::canUnload(unit, unitToUnload, &pos) == 0)
								{
									bStopAll = true;
									bStopLoop = true;
								}
							}
							// 004E78BC
							if (!bStopAll)
							{
								index++;
								bStopLoop = (index < 8);
							}
						}
						if (!bStopAll)
						{
							helper::orderImmediate(unit, OrderId::Unload);
							prepareForNextOrder(unit);
						}
					}
				}
			}
		}
	}

	// unit is a unit to pick up by transport
	// COULD NOT BE TESTED (tried bunker and all and nothing called this function)
	void orders_Pickup4_0(Unit* unit)
	{
		scbw::print("hello");
		Unit* unitVar = unit;
		bool bIsNeutral = unit->playerId == 11;
		char unitStatus = unit->status;
		if (bIsNeutral)
		{
			scbw::print("isNeutral");
			u8 idleOrder;
			if (unitStatus & 0x40)
			{
				idleOrder = OrderId::Nothing2;
			}
			else
			{
				idleOrder = units_dat::ReturnToIdleOrder[(u16)unitVar->id];
			}
			unitVar->userActionFlags |= 1u;
			helper::order(1, unitVar, idleOrder, 0, 0, 228);
			helper::orders_nothing_2(unitVar);
			return;
		}
		if (!(unitStatus & 0x40) && !helper::AI_UpdateAttackTarget(unitVar, 0, 0, 1))
		{
			scbw::print("orderTarget");
			Unit* orderTarget = unitVar->orderTarget.unit;
			Unit** orderTargetOffset = &unitVar->orderTarget.unit;
			if (orderTarget)
			{
				if (!helper::unitIsActiveTransport(orderTarget) || orderTarget->orderTarget.unit != unitVar)
				{
					scbw::print("goto label16");
					goto LABEL_16;
				}
			}
			else
			{
				if (!helper::Ai_FindTransport(unitVar, &unitVar->orderTarget.unit))
				{
					scbw::print("toIdle / return");
					helper::toIdle(unitVar);
					return;
				}
				orderTarget = *orderTargetOffset;
				if (!*orderTargetOffset)
				{
					scbw::print("return");
					return;
				}
			}
			if (!canEnterTransport(orderTarget, unitVar))
			{
			LABEL_16:
				scbw::print("can't enter transport");
				*orderTargetOffset = 0;
				return;
			}
			if (helper::isTargetWithinMinRange(unitVar, 1u, orderTarget))
			{
				scbw::print("target within min range");
				helper::loadUnitProc(orderTarget, unitVar);
				helper::issueOrderTargetingGround(orderTarget, OrderId::MoveUnload, unitVar->orderTarget.pt.x, unitVar->orderTarget.pt.y);
				if (orderTarget->pAI)
				{
					scbw::print("has AI");
					helper::_doOrder(orderTarget, OrderId::Move, orderTarget->sprite->position.x, orderTarget->sprite->position.y, 0);
				}
			}
			else
			{
				scbw::print("target not within min range");
				helper::prependOrderTargetingUnit(unitVar, orderTarget, 0x5Eu);
				helper::orders_nothing_2(orderTarget);
				if (helper::Ai_AreOnConnectedRegions(unitVar, orderTarget->sprite->position.x, orderTarget->sprite->position.y))
				{
					scbw::print("set waypoint");
					helper::setSecondaryWaypoint(unitVar, orderTarget->sprite->position.x, orderTarget->sprite->position.y);
					helper::moveTowards(orderTarget, unitVar);
				}
			}
		}

		/*
		if (unit->playerId == 11)
		{
			unit->userActionFlags |= 1;
			if (unit->status & UnitStatus::InTransport)
			{
				unit->order(OrderId::Nothing2, 0, 0, nullptr, UnitId::None, 1);
			}
			else
			{
				unit->order(units_dat::ReturnToIdleOrder[unit->id], 0, 0, nullptr, UnitId::None, 1);
			}
			prepareForNextOrder(unit);
		}
		else if (!(unit->status & UnitStatus::InTransport)
				&& helper::Ai_UpdateAttackTarget(unit, 1, 0, 0) == 0)
		{ // 004E7BEB
			Unit* target = unit->orderTarget.unit;
			bool jump_to_E7C2B = false;
			bool jump_to_E7C37 = false;
			bool jump_to_E7C44 = false;
			bool bEndThere = false;
			if (target == nullptr)
			{
				if (helper::Ai_FindTransport(unit, &target) == 0)
				{
					unit->orderToIdle();
					bEndThere = true;
				}
				else
				if (target != nullptr)
				{
					jump_to_E7C2B = true;
				}
			}
			if (!bEndThere)
			{
				if (!jump_to_E7C2B)
				{ // 004E7C1B
					if (target != nullptr
						&& (!helper::unitIsActiveTransport(target) || target->orderTarget.unit != unit))
					{
						jump_to_E7C37 = true;
					}
					else
					{
						jump_to_E7C2B = true;
					}
				}
				if (jump_to_E7C2B)
				{
					if (scbw::canBeEnteredBy(target, unit))
					{
						jump_to_E7C44 = true;
					}
					else
					{
						jump_to_E7C37 = true;
					}
				}
				if (jump_to_E7C37)
				{
					unit->orderTarget.unit = nullptr;
				}
				if (jump_to_E7C44)
				{
					if (target != nullptr)
					{
						if (unit->isTargetWithinMinRange(target, 1))
						{ // 004E7C52
							helper::loadUnitProc(target, unit);
							target->orderTo(OrderId::MoveUnload, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
							if (target->pAI != nullptr)
							{
								helper::_doOrder(target, OrderId::Move, target->sprite->position.x, target->sprite->position.y, false);
							}
						}
						else
						{ // 004E7C94
							helper::prependOrderTargetingUnit(target, unit, OrderId::Pickup2);
							prepareForNextOrder(target);
							if (helper::Ai_AreOnConnectedRegions(unit, target->sprite->position.x, target->sprite->position.y) != 0)
							{
								helper::setSecondaryWaypoint(unit, target->sprite->position.x, target->sprite->position.y);
								helper::moveTowards(unit, target);
							}
						}
					}
				}
			}
		}
		*/
	}

	// unit is a unit aiming to enter a transport
	void orders_EnterTransport(Unit* unit)
	{
		Unit* target = unit->orderTarget.unit;
		if (target != nullptr
			&& !(target->status & UnitStatus::IsHallucination)
			&& units_dat::SpaceProvided[target->id] != 0
			&& scbw::canBeEnteredBy(target, unit))
		{ // 004E7D51

			if (unit->mainOrderState == 0
				&& target->mainOrderId != OrderId::Pickup2)
			{
				if (target->status & UnitStatus::IsBuilding)
				{
					helper::prependOrderTargetingUnit(target, unit, OrderId::Pickup2);
					prepareForNextOrder(target);
				}
				unit->mainOrderState = 1;
			}
			if (target->sprite->position.x != unit->nextTargetWaypoint.x ||
				target->sprite->position.y != unit->nextTargetWaypoint.y)
			{
				unit->nextTargetWaypoint.x = target->sprite->position.x;
				unit->nextTargetWaypoint.y = target->sprite->position.y;
			}
			helper::moveTowards(unit, target);
			if (unit->isTargetWithinMinRange(target, 1))
			{
				helper::loadUnitProc(target, unit);
			}
		}
		else
		{
			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else if (unit->pAI != nullptr)
			{
				unit->orderComputer(OrderId::ComputerAI);
			}
			else
			{
				unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
	}

	void orders_Unload(Unit* unit)
	{
		static Unit* unitTable_0059CB58 = (Unit*)(0x0059CB58);
		static Unit* unitTable_0059CB64 = (Unit*)(0x0059CB64);
		if (globals.extended_unit_table == 1)
		{
			const u32 index_start = reinterpret_cast<u32>(&new_unit_table.units[0]) - 336;
			unitTable_0059CB58 = (Unit*)index_start;
			unitTable_0059CB64 = (Unit*)(index_start + 12);
		}
		if (unit->status & UnitStatus::GroundedBuilding
			|| unit->mainOrderTimer == 0)
		{
			s32 counter = 0;
			u32 result_E7F70 = 0;
			bool bEndLoop = false;
			bool bFoundUnitToUnload = false;
			u32* loadedUnitOffset;
			bool jump_to_E8191 = false;
			bool jump_to_E8195 = false;
			bool jump_to_E81AD = false;
			while (!bEndLoop)
			{
				loadedUnitOffset = (u32*)&unit->loadedUnit[counter];
				if (*loadedUnitOffset != 0
					&& unitTable_0059CB64[unit->loadedUnit[counter].index].link.prev != nullptr) // sprite check
				{
					Unit* unitToUnload = &(unitTable_0059CB58[unit->loadedUnit[counter].index]);
					if (unitToUnload->mainOrderId != 0 || unitToUnload->mainOrderState != 1)
					{
						if (unit->loadedUnit[counter].unitId == unitToUnload->targetOrderSpecial)
						{
							bFoundUnitToUnload = true;
							result_E7F70 = helper::unloadUnitProc(unitToUnload);
							if (!(unit->status & UnitStatus::GroundedBuilding))
							{
								bEndLoop = true;
							}
						}
					}
				}

				if (!bEndLoop)
				{
					counter++;
					if (counter >= 8)
					{
						bEndLoop = true;
					}
				}
			}

			if (!bFoundUnitToUnload)
			{
				jump_to_E8191 = true;
			}

			if (!jump_to_E8191)
			{
				if (result_E7F70 != 0)
				{
					jump_to_E8195 = true;
				}
				else if (unit->pAI == nullptr
						|| unit->status & UnitStatus::GroundedBuilding)
				{
					jump_to_E81AD = true;
				}
				else
				{
					helper::function_0043DF30(unit);
				}
			}
			if (jump_to_E8191)
			{
				if (result_E7F70 == 0)
				{
					jump_to_E81AD = true;
				}
				else
				{
					jump_to_E8195 = true;
				}
			}
			if (jump_to_E8195)
			{
				if (unit->status & UnitStatus::GroundedBuilding
					|| !unit->hasLoadedUnit())
					jump_to_E81AD = true;
			}
			if (jump_to_E81AD)
			{
				if (unit->orderQueueHead != nullptr)
				{
					unit->userActionFlags |= 1;
					prepareForNextOrder(unit);
				}
				else if (unit->pAI != nullptr)
				{ // 004E81DE
					unit->userActionFlags |= 1;
					if (unit->mainOrderId == OrderId::Die)
					{
						prepareForNextOrder(unit);
					}
					else
					{
						bEndLoop = (unit->orderQueueTail == nullptr);
						while (!bEndLoop)
						{
							if (orders_dat::CanBeInterrupted[(unit->orderQueueTail)->orderId]
								|| unit->orderQueueTail->orderId == OrderId::ComputerAI)
							{
								helper::remove_order_from_unit_queue(unit->orderQueueTail, unit);
							}
							else
							{
								bEndLoop = true;
							}
							if (!bEndLoop)
							{
								bEndLoop = (unit->orderQueueTail == nullptr);
							}
						}
						unit->performAnotherOrder(OrderId::ComputerAI, 0, 0, nullptr, UnitId::None);
						prepareForNextOrder(unit);
					}
				}
				else
				{ // 004E823A
					unit->userActionFlags |= 1;
					unit->order(units_dat::ReturnToIdleOrder[unit->id], 0, 0, nullptr, UnitId::None, true);
					prepareForNextOrder(unit);
				}
			}
		}
	}

	bool canEnterTransport(Unit* transport, Unit* unit)
	{ // 004E6E00
		if (!(transport->status & UnitStatus::Completed))
		{
			return 0;
		}
		if (transport->status & UnitStatus::DoodadStatesThing)
		{
			return 0;
		}
		if (transport->isFrozen())
		{
			return 0;
		}
		if (unit->status & UnitStatus::Burrowed) // originally also checked for playerIds being the same
		{
			return 0;
		}
		if (!(scbw::isAlliedTo(transport->playerId, unit->playerId)))
		{
			return 0;
		}
		if (scbw::transportIsFull(transport))
		{
			return 0;
		}
		u32 spaceProvided = units_dat::SpaceProvided[transport->id];
		u32 index = 0;
		s32 spaceRequired = 0;
		Unit* loadedUnit;
		if (spaceProvided)
		{
			do
			{
				loadedUnit = helper::getLoadedUnitFromIndex(transport, index);
				if (loadedUnit)
				{
					spaceRequired += units_dat::SpaceRequired[loadedUnit->id];
				}
				++index;
			} while (index < spaceProvided);
			unit = unit;
		}
		return spaceProvided - spaceRequired >= units_dat::SpaceRequired[unit->id];
	}
}