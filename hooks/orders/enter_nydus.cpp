#include "enter_nydus.h"
#include <SCBW/api.h>

namespace hooks
{
	// Necessary to assign enterNydusCanal order to units that normally can't use it (without changing this here,
	// changing identical tests in order function is useless since the unit won't even use that function)
	bool canEnterNydusCanal(Unit* unit, Unit* nydusCanal)
	{
		Unit* const nydusExit = nydusCanal->building.nydusExit;
		return (nydusCanal->status & UnitStatus::Completed)
			&& nydusExit != nullptr
			&& (nydusExit->status & UnitStatus::Completed)
			&& (unit->player == nydusCanal->player
				|| scbw::isAlliedTo(unit->player, nydusCanal->player))
			&& !(units_dat::BaseProperty[unit->id] & UnitProperty::Flyer);
	}

	// Perform the Nydus Canal teleport effect on an unit that validated the conditions within the orders_EnterNydusCanal function
	void enterNydusCanal_Effect(Unit* unit, Unit* nydusCanal)
	{
		Point16 oldPos, aimedPos, finalPos;
		oldPos.x = unit->sprite->position.x;
		oldPos.y = unit->sprite->position.y;
		if (nydusCanal->building.nydusExit->sprite == nullptr)
		{
			//scbw::print("null sprite of nydus exit!");
			nydusCanal->building.nydusExit = nullptr;
			return;
		}
		aimedPos.x = (nydusCanal->building.nydusExit)->sprite->position.x;
		aimedPos.y = (nydusCanal->building.nydusExit)->sprite->position.y;
		scbw::prepareUnitMove(unit, true);
		scbw::set_unit_position(unit, aimedPos.x, aimedPos.y);
		if (scbw::checkUnitCollisionPos(unit, &aimedPos, &finalPos, nullptr, false, 0))
		{ // 004EA201
			helper::playSoundFromSource(SoundId::Misc_IntoNydus_wav, nydusCanal, 1, 0);
			scbw::set_unit_position(unit, finalPos.x, finalPos.y);
			scbw::refreshUnitAfterMove(unit);
			helper::playSoundFromSource(SoundId::Misc_IntoNydus_wav, nydusCanal->building.nydusExit, 1, 0);
			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{ // 004EA253

				if (unit->unit_ai != nullptr)
				{
					unit->order_computer(OrderId::ComputerAI);
				}
				else
				{
					unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
		else
		{	// collision detected, restore old position
			scbw::set_unit_position(unit, oldPos.x, oldPos.y);
			scbw::refreshUnitAfterMove(unit);
		}
	}

	void orders_EnterNydusCanal(Unit* unit)
	{
		Unit* nydusCanal;
		nydusCanal = unit->orderTarget.unit;
		// same tests as canEnterNydusCanal but left hardcoded like the original code
		if (nydusCanal == nullptr
			|| !(nydusCanal->status & UnitStatus::Completed)
			|| nydusCanal->building.nydusExit == nullptr
			|| !((nydusCanal->building.nydusExit)->status & UnitStatus::Completed)
			|| (!scbw::isAlliedTo(unit->player, nydusCanal->player))
			|| units_dat::BaseProperty[unit->id] & UnitProperty::Flyer)
		{ // 004EA478
			if (unit->orderQueueHead != nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{ // 004EA497
				if (unit->unit_ai != nullptr)
				{
					unit->order_computer(OrderId::ComputerAI);
				}
				else
				{
					unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
		else
		{ // 004EA428
			if (unit->mainOrderState == 0)
			{	// 004EA464
				if (helper::order_to_move_to_target(unit, nydusCanal))
				{
					unit->mainOrderState = 1;
				}
			}
			else
				if (unit->mainOrderState == 1)
				{ // 004EA43B
					u8 movableState = unit->getMovableState();
					if (movableState == 1)	// reached destination
					{
						unit->mainOrderState = 2;
					}
					else
						if (movableState != 0)	// (value is probably 2 == unmovable)
						{
							unit->orderToIdle();
						}
				}
			if (unit->mainOrderState == 2) // 004EA458
			{
				enterNydusCanal_Effect(unit, nydusCanal);
			}
		}
	}
}