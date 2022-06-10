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

	s32 shield_gain = 1280, energySpent = 640;

	if (maxShields - target->shields < shield_gain)
	{

		shield_gain = maxShields - target->shields;

		if (shield_gain < 0)
			shield_gain++;

		energySpent = shield_gain / 2;

	}

	if (energySpent > battery->energy)
	{
		energySpent = battery->energy;
		shield_gain = energySpent * 2;
	}

	target->heal_shields(shield_gain, battery);

	if (!scbw::is_cheat_enabled(CheatFlags::TheGathering))
		battery->spend_unit_energy(energySpent);

}

;

//-------- Actual hook functions --------//

namespace hooks
{

	// Decides whether the @p target can recharge shields from the @p battery.
	b32 unitCanRechargeShieldsHook(Unit* target, Unit* battery) {
		//Check target conditions
		if (!scbw::is_allied_to(target->player, battery->player)			 // Is not owned by the player (NEW: or the player's allies)
		|| !(units_dat::ShieldsEnabled[target->id])						     // Does not have shields
		|| !(target->status & UnitStatus::Completed)						 // Is incomplete?
	//	|| !(target->status & UnitStatus::IsBuilding)						 // Is building (like archon build self)?
	//	|| target->status & UnitStatus::GroundedBuilding					 // Is a building
	//	|| target->get_race() != RaceId::Protoss							     // Check target race
		|| target->shields >= (units_dat::MaxShieldPoints[target->id] * 256) // Already has max shields
		|| !(battery->status & UnitStatus::Completed) || !battery->energy
		|| battery->is_frozen() || helper::isHangarUnit(target)) {
			return 0;
		}

		// Separate check for AI-controlled units
		if (target->unit_ai) {
			if (target->main_order_id == OrderId::RechargeShields1 || target->main_order_id == OrderId::Pickup4) {
				return 0;
			}

			if (target->order_queue_head && target->order_queue_head->order_id == OrderId::RechargeShields1) {
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

		Unit* battery = unit->order_target.unit;

		//Skip if the Shield Battery does not exist, has no energy, or is disabled
		//is_frozen code was hardcoded rather than function call in original code
		if (battery == nullptr || battery->energy == 0 || battery->is_frozen())
		{

			if (unit->order_queue_head == nullptr)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			}
			else
			{
				if (unit->unit_ai != nullptr)
					unit->order_computer(OrderId::ComputerAI);
				else
					unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
			}

		}
		else
			if (unit->main_order_state <= 3)
			{

				bool bStopThere = false;

				if (unit->main_order_state == 0)
				{

					if (helper::order_to_move_to_target(unit, battery))
						unit->main_order_state = 1;

					bStopThere = true;

				}
				else
					if (unit->main_order_state == 1)
					{

						u32 movableState = unit->get_movable_state();

						if (movableState == 0)
						{ //not reached destination
							if (!unit->is_target_within_min_range(battery, BATTERY_RANGE))
								bStopThere = true;
						}
						else
							if (movableState == 1)
							{ //unit reached destination
								helper::make_to_hold_position(unit);
								unit->main_order_state = 2;
							}
							else
							{ //probably movableState == 2 == unit unmovable
								unit->order_to_idle();
								bStopThere = true;
							}

					}

				if (!bStopThere && unit->main_order_state == 2)
				{

					u8 overlaysCounter;

					if (battery->secondary_order_id != OrderId::ShieldBattery)
						battery->set_secondary_order(OrderId::ShieldBattery);

					overlaysCounter = helper::has_overlay(unit);
					unit->sprite->create_overlay(ImageId::RechargeShields_Small + overlaysCounter, 0, 0, 0);

					//the order is correct, see OrderId.h description
					helper::insertFirstOrder(unit, OrderId::Harvest5);

					unit->main_order_state = 3;

				}

				if (!bStopThere && unit->main_order_state == 3)
				{

					if (battery->order_target.unit == nullptr)
						battery->order_target.unit = unit;

					//do 1 recharge action? (increase unit shield, decrease battery energy)
					rechargeShieldsProc(unit, battery);

					if (
						unit->shields >= units_dat::MaxShieldPoints[unit->id] * 256 ||
						battery->energy == 0
						)
					{

						if (battery->order_target.unit == unit)
							battery->order_target.unit = nullptr;

						unit->order_to_idle();

					}

				} //if(!bStopThere && unit->main_order_state == 3)

			} //if(unit->main_order_state <= 3)
	}
}