#include "recharge_shields.h"
#include "../SCBW/scbwdata.h"
#include "../SCBW/enumerations.h"
#include "../SCBW/api.h"

//This function is called every frame when a unit recharges shields.
//Equivalent to 004934B0  sub_4934B0
//Not hooked
void rechargeShieldsProc(Unit* target, Unit* battery)
{

	const s32 maxShields = units_dat::MaxShieldPoints[target->id] * 256;

	s32 shieldGain = 1280, energySpent = 640;

	if (maxShields - target->shields < shieldGain)
	{

		shieldGain = maxShields - target->shields;

		if (shieldGain < 0)
			shieldGain++;

		energySpent = shieldGain / 2;

	}

	if (energySpent > battery->energy)
	{
		energySpent = battery->energy;
		shieldGain = energySpent * 2;
	}

	target->healShields(shieldGain, battery);

	if (!scbw::isCheatEnabled(CheatFlags::TheGathering))
		battery->spendUnitEnergy(energySpent);

}

;

//-------- Actual hook functions --------//

namespace hooks
{

	// Decides whether the @p target can recharge shields from the @p battery.
	b32 unitCanRechargeShieldsHook(Unit* target, Unit* battery) {
		//Check target conditions
		if (!scbw::isAlliedTo(target->playerId, battery->playerId)			 // Is not owned by the player (NEW: or the player's allies)
		|| !(units_dat::ShieldsEnabled[target->id])						     // Does not have shields
		|| !(target->status & UnitStatus::Completed)						 // Is incomplete?
	//	|| !(target->status & UnitStatus::IsBuilding)						 // Is building (like archon build self)?
	//	|| target->status & UnitStatus::GroundedBuilding					 // Is a building
	//	|| target->getRace() != RaceId::Protoss							     // Check target race
		|| target->shields >= (units_dat::MaxShieldPoints[target->id] * 256) // Already has max shields
		|| !(battery->status & UnitStatus::Completed) || !battery->energy
		|| battery->isFrozen() || helper::isHangarUnit(target)) {
			return 0;
		}

		// Separate check for AI-controlled units
		if (target->pAI) {
			if (target->mainOrderId == OrderId::RechargeShields1 || target->mainOrderId == OrderId::Pickup4) {
				return 0;
			}

			if (target->orderQueueHead && target->orderQueueHead->orderId == OrderId::RechargeShields1) {
				return 1;
			}

			return 0;
		}

		return 1;
	}

	// The order process run by a unit when recharging shields
	// Identical to orders_RechargeShields1 @ 00493DD0
	void orderRechargeShieldsHook(Unit* unit)
	{

		const s32 BATTERY_RANGE = 0x80; //(128)

		Unit* battery = unit->orderTarget.unit;

		//Skip if the Shield Battery does not exist, has no energy, or is disabled
		//isFrozen code was hardcoded rather than function call in original code
		if (battery == nullptr || battery->energy == 0 || battery->isFrozen())
		{

			if (unit->orderQueueHead == nullptr)
			{
				unit->userActionFlags |= 1;
				prepareForNextOrder(unit);
			}
			else
			{
				if (unit->pAI != nullptr)
					unit->orderComputer(OrderId::ComputerAI);
				else
					unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
			}

		}
		else
			if (unit->mainOrderState <= 3)
			{

				bool bStopThere = false;

				if (unit->mainOrderState == 0)
				{

					if (helper::orderToMoveToTarget(unit, battery))
						unit->mainOrderState = 1;

					bStopThere = true;

				}
				else
					if (unit->mainOrderState == 1)
					{

						u32 movableState = unit->getMovableState();

						if (movableState == 0)
						{ //not reached destination
							if (!unit->isTargetWithinMinRange(battery, BATTERY_RANGE))
								bStopThere = true;
						}
						else
							if (movableState == 1)
							{ //unit reached destination
								helper::make_to_hold_position(unit);
								unit->mainOrderState = 2;
							}
							else
							{ //probably movableState == 2 == unit unmovable
								unit->orderToIdle();
								bStopThere = true;
							}

					}

				if (!bStopThere && unit->mainOrderState == 2)
				{

					u8 overlaysCounter;

					if (battery->secondaryOrderId != OrderId::ShieldBattery)
						battery->setSecondaryOrder(OrderId::ShieldBattery);

					overlaysCounter = helper::hasOverlay(unit);
					unit->sprite->createOverlay(ImageId::RechargeShields_Small + overlaysCounter, 0, 0, 0);

					//the order is correct, see OrderId.h description
					helper::insertFirstOrder(unit, OrderId::Harvest5);

					unit->mainOrderState = 3;

				}

				if (!bStopThere && unit->mainOrderState == 3)
				{

					if (battery->orderTarget.unit == nullptr)
						battery->orderTarget.unit = unit;

					//do 1 recharge action? (increase unit shield, decrease battery energy)
					rechargeShieldsProc(unit, battery);

					if (
						unit->shields >= units_dat::MaxShieldPoints[unit->id] * 256 ||
						battery->energy == 0
						)
					{

						if (battery->orderTarget.unit == unit)
							battery->orderTarget.unit = nullptr;

						unit->orderToIdle();

					}

				} //if(!bStopThere && unit->mainOrderState == 3)

			} //if(unit->mainOrderState <= 3)
	}
}