#include "enter_nydus.h"
#include <SCBW/api.h>

namespace hooks
{
	// Necessary to assign enterNydusCanal order to units that normally can't use it (without changing this here,
	// changing identical tests in order function is useless since the unit won't even use that function)
	bool canEnterNydusCanal(Unit* unit, Unit* nydusCanal)
	{
		Unit* const nydus_exit = nydusCanal->building.nydus_exit;
		return (nydusCanal->status & UnitStatus::Completed)
			&& nydus_exit != nullptr
			&& (nydus_exit->status & UnitStatus::Completed)
			&& (unit->player == nydusCanal->player
				|| scbw::is_allied_to(unit->player, nydusCanal->player))
			&& !(units_dat::BaseProperty[unit->id] & UnitProperty::Flyer);
	}

	// Perform the Nydus Canal teleport effect on an unit that validated the conditions within the orders_EnterNydusCanal function
	void enterNydusCanal_Effect(Unit* unit, Unit* nydusCanal)
	{
		Point16 oldPos, aimedPos, finalPos;
		oldPos.x = unit->sprite->position.x;
		oldPos.y = unit->sprite->position.y;
		if (nydusCanal->building.nydus_exit->sprite == nullptr)
		{
			//scbw::print("null sprite of nydus exit!");
			nydusCanal->building.nydus_exit = nullptr;
			return;
		}
		aimedPos.x = (nydusCanal->building.nydus_exit)->sprite->position.x;
		aimedPos.y = (nydusCanal->building.nydus_exit)->sprite->position.y;
		scbw::prepare_unit_move(unit, true);
		scbw::set_unit_position(unit, aimedPos.x, aimedPos.y);
		if (scbw::check_unit_collision_pos(unit, &aimedPos, &finalPos, nullptr, false, 0))
		{ // 004EA201
			helper::play_sound_from_unit(SoundId::Misc_IntoNydus_wav, nydusCanal, 1, 0);
			scbw::set_unit_position(unit, finalPos.x, finalPos.y);
			scbw::refresh_unit_after_move(unit);
			helper::play_sound_from_unit(SoundId::Misc_IntoNydus_wav, nydusCanal->building.nydus_exit, 1, 0);
			if (unit->order_queue_head != nullptr)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
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
			scbw::refresh_unit_after_move(unit);
		}
	}

	void orders_EnterNydusCanal(Unit* unit)
	{
		Unit* nydusCanal;
		nydusCanal = unit->order_target.unit;
		// same tests as canEnterNydusCanal but left hardcoded like the original code
		if (nydusCanal == nullptr
			|| !(nydusCanal->status & UnitStatus::Completed)
			|| nydusCanal->building.nydus_exit == nullptr
			|| !((nydusCanal->building.nydus_exit)->status & UnitStatus::Completed)
			|| (!scbw::is_allied_to(unit->player, nydusCanal->player))
			|| units_dat::BaseProperty[unit->id] & UnitProperty::Flyer)
		{ // 004EA478
			if (unit->order_queue_head != nullptr)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
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
			if (unit->main_order_state == 0)
			{	// 004EA464
				if (helper::order_to_move_to_target(unit, nydusCanal))
				{
					unit->main_order_state = 1;
				}
			}
			else
				if (unit->main_order_state == 1)
				{ // 004EA43B
					u8 movableState = unit->get_movable_state();
					if (movableState == 1)	// reached destination
					{
						unit->main_order_state = 2;
					}
					else
						if (movableState != 0)	// (value is probably 2 == unmovable)
						{
							unit->order_to_idle();
						}
				}
			if (unit->main_order_state == 2) // 004EA458
			{
				enterNydusCanal_Effect(unit, nydusCanal);
			}
		}
	}
}