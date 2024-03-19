#include "recharge_shields.h"
#include "../BW/bwdata.h"
#include "../BW/enumerations.h"
#include "../BW/api.h"

// This function is called every frame when a unit recharges shields.
// Not hooked
void recharge_shields_proc(Unit* target, Unit* battery) // 004934B0
{
	const s32 max_shields = units_dat::max_shield_points[target->id] * 256;
	s32 shield_gain = 1280;
	s32 energy_spent = 640;
	if (max_shields - target->shields < shield_gain) {
		shield_gain = max_shields - target->shields;
		if (shield_gain < 0) {
			shield_gain++;
		}
		energy_spent = shield_gain / 2;
	}

	if (energy_spent > battery->energy) {
		energy_spent = battery->energy;
		shield_gain = energy_spent * 2;
	}
	target->heal_shields(shield_gain, battery);
	battery->spend_unit_energy(energy_spent);
}

//-------- Actual hook functions --------//

namespace hooks
{
	// Decides whether the @p target can recharge shields from the @p battery.
	b32 unit_can_recharge_shields(Unit* target, Unit* battery) // 00493520
	{
		if (!bw::is_allied_to(target->player, battery->player)					// Is not owned by the player (NEW: or the player's allies)
		|| !(units_dat::shields_enabled[target->id])							// Does not have shields
		|| !(target->status & UnitStatus::Completed)							// Is incomplete?
		|| target->shields >= (units_dat::max_shield_points[target->id] * 256) // Already has max shields
		|| !(battery->status & UnitStatus::Completed) || !battery->energy
		|| battery->is_frozen() || helper::is_hangar_unit(target))
		{
			return 0;
		}

		// Separate check for AI-controlled units
		if (target->unit_ai) {
			if (target->main_order_id == OrderId::RechargeShields1 || target->main_order_id == OrderId::TransportLoad) {
				return 0;
			}
			if (target->order_queue_head && target->order_queue_head->order_id == OrderId::RechargeShields1) {
				return 1;
			}
			return 0;
		}
		return 1;
	}

	void order_recharge_shields(Unit* unit) // 00493DD0
	{
		const s32 BATTERY_RANGE = 128;
		Unit* battery = unit->order_target.unit;
		if (!battery || battery->energy == 0 || battery->is_frozen()) { // is_frozen code was hardcoded rather than function call in original code
			if (unit->order_queue_head) {
				unit->user_action_flags |= UserActionFlags_IscriptGoToIdle;
				prepare_for_next_order(unit);
			} else {
				if (unit->unit_ai) {
					unit->order_computer(OrderId::ComputerAI);
				} else {
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				}
			}
		} else if (unit->main_order_state <= 3) {
			bool bStopThere = false;
			if (unit->main_order_state == 0) {
				if (helper::order_to_move_to_target(unit, battery))
					unit->main_order_state = 1;
				bStopThere = true;
			} else if (unit->main_order_state == 1) {
					u32 movable_state = unit->get_movable_state();
					if (movable_state == MovableState_NotReachedDestination) {
						if (!unit->is_target_within_min_range(battery, BATTERY_RANGE)) {
							bStopThere = true;
						}
					} else if (movable_state == MovableState_ReachedDestination) {
						helper::make_to_hold_position(unit);
						unit->main_order_state = 2;
					} else {
						unit->order_to_idle();
						bStopThere = true;
					}
				}
			if (!bStopThere && unit->main_order_state == 2) {
				u8 overlay_counter;
				if (battery->secondary_order_id != OrderId::ShieldBattery) {
					battery->set_secondary_order(OrderId::ShieldBattery);
				}
				overlay_counter = helper::has_overlay(unit);
				unit->sprite->create_overlay(ImageId::RechargeShields_Small + overlay_counter, 0, 0, 0);
				helper::insert_first_order(unit, OrderId::Harvest5); // the order is correct, see OrderId.h description
				unit->main_order_state = 3;
			}
			if (!bStopThere && unit->main_order_state == 3) {
				if (!battery->order_target.unit) {
					battery->order_target.unit = unit;
				}
				// do 1 recharge action? (increase unit shield, decrease battery energy)
				recharge_shields_proc(unit, battery);

				if (unit->shields >= units_dat::max_shield_points[unit->id] * 256 || battery->energy == 0) {
					if (battery->order_target.unit == unit) {
						battery->order_target.unit = 0;
					}
					unit->order_to_idle();
				}
			}
		}
	}
}