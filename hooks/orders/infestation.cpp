#include "infestation.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	// replace 0x00402750 canInfest
	bool unitCanInfestHook(Unit* unit)
	{
		return false;
	}

	// replace 0x00402210 CC_CanBeInfested
	bool isInfestableUnitHook(Unit* unit)
	{
		u32 unitHpTest;
		u32 unitMaxHp;
		bool returnValue;
		returnValue = false;
		if (returnValue)
		{
			unitMaxHp = unit->getMaxHpInGame();
			unitHpTest = unit->getCurrentHpInGame();
			// if HP/maxHP higher or equal than 50%, not damaged enough
			if (((100 * unitHpTest) / unitMaxHp) >= 50)
			{
				returnValue = false;
			}
		}
		return returnValue;
	}

	// Based on and replace orders_InfestMine1 (0x004EA4C0)
	void orderMorphIntoInfested(Unit* unitInfested)
	{
		bool bErrorReturnToIdle;
		bool bStopFunction = false;
		u16 infestedUnitNewId;
		u16 infestationOverlayId;
		Unit* unitInfesting = unitInfested->orderTarget.unit;
		bErrorReturnToIdle = (unitInfesting == nullptr || !isInfestableUnitHook(unitInfested));

		if (!bErrorReturnToIdle)
		{
			if (unitInfested->mainOrderState == 0)
			{
				//init timer
				unitInfested->remainingBuildTime = 3;
				unitInfested->mainOrderState = 2;
				bStopFunction = true;
			}

			if (!bStopFunction)
			{
				if (unitInfested->remainingBuildTime != 0)
				{
					// advancing timer
					unitInfested->remainingBuildTime--;
					bStopFunction = true;
				}
				else
				{
					// will become 0xffff, this is used by internal functions
					unitInfested->remainingBuildTime--;

					// disconnect terran buildings from their add-on
					if (unitInfested->building.addon != nullptr)
					{
						helper::disconnectFromAddOn(unitInfested);
					}

					// this is the same as calling 0047B2E0 unitIsFactoryUnit like original code but faster
					if (unitInfested->isFactory())
					{
						UnitExt* unitInfestedExt = globals.GetUnitExt(unitInfested);
						unitInfested->rally.unit = unitInfested;
					}

					// original code was *(u16*)(0x006648AC + 2*unitInfested->id);
					infestedUnitNewId = units_dat::InfestedUnitPartial[unitInfested->id - UnitId::TerranCommandCenter];

					// update number of kills/casualties for endgame score
					helper::incrementUnitDeathScores(unitInfested, unitInfesting->playerId);

					// effect unknown, don't seem to affect score screen at endgame
					helper::incrementUnitScores(unitInfested, -1);
					if (unitInfested->status & UnitStatus::Completed)
					{
						helper::incrementUnitScoresEx(unitInfested, 0, -1);
					}

					// the unit receive the new id (infested id)
					unitInfested->id = infestedUnitNewId;

					// effect unknown, don't seem to affect score screen at endgame
					helper::incrementUnitScores(unitInfested, 1);
					if (unitInfested->status & UnitStatus::Completed)
					{
						helper::incrementUnitScoresEx(unitInfested, 0, 1);
					}

					// I don't really know, advanced internal mechanics, just have to work with it
					helper::GiveUnitHelper(unitInfested, unitInfesting->playerId, 1);
					helper::initializeEmptyUnitsLinkedListRef(unitInfested, unitInfesting->playerId);

					// Related to infestation process, don't use unitInfested->setSecondaryOrder
					// because here it is supposed to perform all actions even if 
					// unitInfested->secondaryOrderId == OrderId::Nothing2 beforehand
					unitInfested->secondaryOrderPos.y = 0;
					unitInfested->secondaryOrderPos.x = 0;
					unitInfested->currentBuildUnit = nullptr;
					unitInfested->secondaryOrderState = 0;
					unitInfested->secondaryOrderId = OrderId::Nothing2;

					// refund what the unit was producing
					helper::refundAllQueueSlots(unitInfested);

					// clear the orders queue of the unit (hardcoding function @ 0x004744D0)
					// original code used the actual function instead of hardcoding
					while (unitInfested->orderQueueHead != nullptr)
					{
						helper::remove_order_from_unit_queue(unitInfested->orderQueueHead, unitInfested);
					}

					if (unitInfested->status & UnitStatus::GroundedBuilding)
					{
						Image* current_image = unitInfested->sprite->images.head;

						while (current_image != nullptr)
						{
							// script 0x14 in asm code
							current_image->playIscriptAnim(IscriptAnimation::WorkingToIdle);

							// load next image of the list (or nullptr)
							current_image = current_image->link.next;
						}
					}

					// Just use ImageId::InfestedCommandCenterOverlay instead
					// if you're modding this file and you're going for the
					// default infestation overlay for any target.
					infestationOverlayId = units_dat::ConstructionGraphic[infestedUnitNewId];

					if (infestationOverlayId < ImageId::None)
					{
						unitInfested->sprite->createOverlay(infestationOverlayId);
					}
					unitInfested->setHp(units_dat::MaxHitPoints[infestedUnitNewId]);

					// give the proper button set to the unit
					helper::changeUnitButtonSet(unitInfested, infestedUnitNewId);
					scbw::refreshConsole();
				}
			}
		}
		if (!bStopFunction)
		{
			helper::force_order_done(unitInfested);
		}
	}

	// Based on and replace 004EA290 orders_InfestMine4
	void orderInfestTarget(Unit* unitInfesting)
	{
		Unit* unitInfested = unitInfesting->orderTarget.unit;
		bool bBuildTimeReachedZero = false;
		bool bStopFunction = false;
		bool bReturnToIdle = false;
		Point16 actualpos;

		if (unitInfesting->mainOrderState == 3)
		{
			//infesting in progress, dealing with timer			
			if (unitInfesting->remainingBuildTime != 0)
			{
				unitInfesting->remainingBuildTime--;
				bStopFunction = true;
			}
			else
			{
				bBuildTimeReachedZero = true;
			}
		}
		else if (unitInfesting->mainOrderState != 0) // if ==0, wait for EA33F to start infestation logic
		{
			bStopFunction = true;
		}

		if (!bStopFunction && bBuildTimeReachedZero)
		{
			// EA2CC
			// Infestation timer ended

			// using unitInfesting->sprite->position because the queen and command center are
			// indeed at the same location, and more importantly, if the infested unit got
			// destroyed, then access to unitInfested->sprite->position would cause a crash
			if (scbw::checkUnitCollisionPos(unitInfesting, &unitInfesting->sprite->position, &actualpos))
			{
				scbw::setUnitPosition(unitInfesting, actualpos.x, actualpos.y);
				helper::makeToHoldPosition(unitInfesting);
				helper::show_and_enable_unit(unitInfesting);

				if (unitInfesting->orderQueueHead == nullptr)
				{
					unitInfesting->orderSimple(units_dat::ReturnToIdleOrder[unitInfesting->id], false);
					bReturnToIdle = true;
				}
				else
				{
					bReturnToIdle = true;
				}
			}
			else
			{
				bStopFunction = true;
			}
		}

		if (bReturnToIdle)
		{
			helper::force_order_done(unitInfesting);
		}
		else if (!bStopFunction)
		{ // EA33F
			// not started the infestation logic yet, doing it here
			if (unitInfested != nullptr
				&& isInfestableUnitHook(unitInfested)
				&& unitInfested->mainOrderId != OrderId::InfestMine1)
			{
				helper::hideAndDisableUnit(unitInfesting);								// hide the infesting unit
				unitInfested->orderTo(OrderId::InfestMine1, unitInfesting);				// order the target to be infested, giving it the infesting unit as order target
				unitInfesting->mainOrderState = 3;
				unitInfesting->remainingBuildTime = 60;									// timer before reappearing
				bStopFunction = true;
			}

			if (!bStopFunction)
			{ // EA377
				// "move to invalid target" logic...I think
				helper::setNextWaypoint(unitInfesting);
				if (unitInfesting->moveTarget.pt.x != unitInfesting->nextTargetWaypoint.x
					|| unitInfesting->moveTarget.pt.y != unitInfesting->nextTargetWaypoint.y)
				{
					unitInfesting->nextTargetWaypoint.x = unitInfesting->moveTarget.pt.x;
					unitInfesting->nextTargetWaypoint.y = unitInfesting->moveTarget.pt.y;
				}
				//EA39E
				if (unitInfesting->orderQueueHead != nullptr)
				{
					helper::force_order_done(unitInfesting);
				}
				else
				{
					unitInfesting->order(units_dat::ReturnToIdleOrder[unitInfesting->id], 0, 0, nullptr, UnitId::None, false);
					helper::force_order_done(unitInfesting);
				}
			}
		}
	}
}