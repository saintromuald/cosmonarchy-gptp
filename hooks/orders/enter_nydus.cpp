#include "enter_nydus.h"
#include <SCBW/api.h>

namespace hooks
{
	// Necessary to assign the Enter Nydus order to units that normally can't use it
	bool can_enter_nydus(Unit* unit, Unit* nydus)
	{
		Unit* const nydus_exit = nydus->building.nydus_exit;
		if  (nydus->status & UnitStatus::Completed
		&&	 nydus_exit && nydus_exit->status & UnitStatus::Completed
		&&	(unit->player == nydus->player || scbw::is_allied_to(unit->player, nydus->player))
		&& !(units_dat::base_property[unit->id] & UnitProperty::Flyer)) {
			return true;
		}
		return false;
	}

	// Perform the Nydus Canal teleport effect on an unit that validated the conditions within the orders_enter_nydus function
	void enter_nydus_effect(Unit* unit, Unit* nydus)
	{
		if (!nydus->building.nydus_exit->sprite)
		{
			nydus->building.nydus_exit = 0;
			return;
		}

		Point16 old_pos = { unit->position.x, unit->position.y };
		Point16 aimed_pos = { (nydus->building.nydus_exit)->position.x, (nydus->building.nydus_exit)->position.y };
		Point16 final_pos = {};
		scbw::prepare_unit_move(unit, true);
		scbw::set_unit_position(unit, aimed_pos.x, aimed_pos.y);
		if (scbw::check_unit_collision_pos(unit, &aimed_pos, &final_pos, 0, false, 0))
		{
			helper::play_sound_from_unit(SoundId::Misc_IntoNydus_wav, nydus, 1, 0);
			scbw::set_unit_position(unit, final_pos.x, final_pos.y);
			scbw::refresh_unit_after_move(unit);
			helper::play_sound_from_unit(SoundId::Misc_IntoNydus_wav, nydus->building.nydus_exit, 1, 0);
			if (unit->order_queue_head)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			} else {
				if (unit->unit_ai)
				{
					unit->order_computer(OrderId::ComputerAI);
				} else {
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				}
			}
		} else {
			// collision detected, restore old position
			scbw::set_unit_position(unit, old_pos.x, old_pos.y);
			scbw::refresh_unit_after_move(unit);
		}
	}

	void orders_enter_nydus(Unit* unit)
	{
		Unit* nydus = unit->order_target.unit;
		if  (!nydus
		||  !(nydus->status & UnitStatus::Completed)
		||   !nydus->building.nydus_exit
		|| !((nydus->building.nydus_exit)->status & UnitStatus::Completed)
		||  (!scbw::is_allied_to(unit->player, nydus->player))
		||    units_dat::base_property[unit->id] & UnitProperty::Flyer)
		{
			if (unit->order_queue_head)
			{
				unit->user_action_flags |= 1;
				prepare_for_next_order(unit);
			} else {
				if (unit->unit_ai)
				{
					unit->order_computer(OrderId::ComputerAI);
				} else {
					unit->order_computer(units_dat::return_to_idle_order[unit->id]);
				}
			}
		} else {
			if (unit->main_order_state == 0)
			{
				if (helper::order_to_move_to_target(unit, nydus))
				{
					unit->main_order_state = 1;
				}
			} else if (unit->main_order_state == 1) {
					u8 movable_state = unit->get_movable_state();
					if (movable_state == MovableState::ReachedDestination)
					{
						unit->main_order_state = 2;
					} else if (movable_state != 0) { // (value is probably 2 == unmovable)
						unit->order_to_idle();
					}
				}
			if (unit->main_order_state == 2)
			{
				enter_nydus_effect(unit, nydus);
			}
		}
	}
}