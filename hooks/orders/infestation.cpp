#include "infestation.h"
#include <BW/api.h>
#include "globals.h"

namespace hooks
{
	// replace 0x00402750 canInfest
	bool unit_can_infest(Unit* unit)
	{
		return false;
	}

	// replace 0x00402210 CC_CanBeInfested
	bool unit_can_be_infested(Unit* unit)
	{
		u32 unitHpTest;
		u32 unitMaxHp;
		bool returnValue;
		returnValue = false;
		if (returnValue)
		{
			unitMaxHp = unit->get_max_hp_ingame();
			unitHpTest = unit->get_current_hp_ingame();
			// if HP/maxHP higher or equal than 50%, not damaged enough
			if (((100 * unitHpTest) / unitMaxHp) >= 50) {
				returnValue = false;
			}
		}
		return returnValue;
	}

	// Based on and replace orders_InfestMine1 (0x004EA4C0)
	void unit_morph_into_infested_order(Unit* unitInfested)
	{
		bool bErrorReturnToIdle;
		bool bStopFunction = false;
		u16 infestedUnitNewId;
		u16 infestation_overlay_id;
		Unit* unitInfesting = unitInfested->order_target.unit;
		bErrorReturnToIdle = (unitInfesting == nullptr || !unit_can_be_infested(unitInfested));

		if (!bErrorReturnToIdle) {
			if (unitInfested->main_order_state == 0) {
				//init timer
				unitInfested->remaining_build_time = 3;
				unitInfested->main_order_state = 2;
				bStopFunction = true;
			}

			if (!bStopFunction) {
				if (unitInfested->remaining_build_time != 0) {
					// advancing timer
					unitInfested->remaining_build_time--;
					bStopFunction = true;
				} else {
					// will become 0xffff, this is used by internal functions
					unitInfested->remaining_build_time--;

					// disconnect terran buildings from their add-on
					if (unitInfested->building.addon != nullptr)
					{
						helper::disconnect_addon(unitInfested);
					}

					// this is the same as calling 0047B2E0 unitIsFactoryUnit like original code but faster
					if (unitInfested->is_factory())
					{
						UnitExt* unitInfestedExt = get_unit_ext(unitInfested);
						unitInfested->rally.unit = unitInfested;
					}

					// original code was *(u16*)(0x006648AC + 2*unitInfested->id);
					infestedUnitNewId = units_dat::infested_unit_partial[unitInfested->id - UnitId::TerranMinistry];

					// update number of kills/casualties for endgame score
					helper::increment_unit_death_scores(unitInfested, unitInfesting->player);

					// effect unknown, don't seem to affect score screen at endgame
					helper::increment_unit_scores(unitInfested, -1);
					if (unitInfested->status & UnitStatus::Completed)
					{
						helper::increment_unit_scores_ex(unitInfested, 0, -1);
					}

					// the unit receive the new id (infested id)
					unitInfested->id = infestedUnitNewId;

					// effect unknown, don't seem to affect score screen at endgame
					helper::increment_unit_scores(unitInfested, 1);
					if (unitInfested->status & UnitStatus::Completed) {
						helper::increment_unit_scores_ex(unitInfested, 0, 1);
					}

					// I don't really know, advanced internal mechanics, just have to work with it
					helper::give_unit(unitInfested, unitInfesting->player, 1);
					helper::give_sprite(unitInfested, unitInfesting->player);

					// Related to infestation process, don't use unitInfested->set_secondary_order
					// because here it is supposed to perform all actions even if 
					// unitInfested->secondary_order_id == OrderId::Nothing2 beforehand
					unitInfested->secondary_order_pos.y = 0;
					unitInfested->secondary_order_pos.x = 0;
					unitInfested->current_build_unit = nullptr;
					unitInfested->secondary_order_state = 0;
					unitInfested->secondary_order_id = OrderId::Nothing2;

					// refund what the unit was producing
					helper::refund_all_queue_slots(unitInfested);

					// clear the orders queue of the unit (hardcoding function @ 0x004744D0)
					// original code used the actual function instead of hardcoding
					while (unitInfested->order_queue_head != nullptr)
					{
						helper::remove_order_from_unit_queue(unitInfested->order_queue_head, unitInfested);
					}

					if (unitInfested->status & UnitStatus::GroundedBuilding) {
						Image* current_image = unitInfested->sprite->images.head;
						while (current_image != nullptr)
						{
							// script 0x14 in asm code
							current_image->play_iscript_animation(IscriptAnimation::WorkingToIdle);
							// load next image of the list (or nullptr)
							current_image = current_image->link.next;
						}
					}

					// Just use image_id::InfestedCommandCenterOverlay instead, if you're modding this file
					// and you're going for the default infestation overlay for any target. -GPTP
					infestation_overlay_id = get_construction_graphic(unitInfested);

					if (infestation_overlay_id != ImageId::None) {
						unitInfested->sprite->create_overlay(infestation_overlay_id);
					}
					unitInfested->set_hitpoints(units_dat::max_hitpoints[infestedUnitNewId]);

					// give the proper button set to the unit
					helper::change_unit_buttonset(unitInfested, infestedUnitNewId);
					bw::refresh_console();
				}
			}
		}
		if (!bStopFunction)
		{
			helper::force_order_done(unitInfested);
		}
	}

	// Based on and replace 004EA290 orders_InfestMine4
	void unit_infest_order(Unit* unitInfesting)
	{
		Unit* unitInfested = unitInfesting->order_target.unit;
		bool bBuildTimeReachedZero = false;
		bool bStopFunction = false;
		bool bReturnToIdle = false;
		Point16 actualpos;

		if (unitInfesting->main_order_state == 3)
		{
			//infesting in progress, dealing with timer			
			if (unitInfesting->remaining_build_time != 0)
			{
				unitInfesting->remaining_build_time--;
				bStopFunction = true;
			}
			else
			{
				bBuildTimeReachedZero = true;
			}
		}
		else if (unitInfesting->main_order_state != 0) // if ==0, wait for EA33F to start infestation logic
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
			if (bw::check_unit_collision_pos(unitInfesting, &unitInfesting->sprite->position, &actualpos))
			{
				bw::set_unit_position(unitInfesting, actualpos.x, actualpos.y);
				helper::make_to_hold_position(unitInfesting);
				helper::show_and_enable_unit(unitInfesting);

				if (unitInfesting->order_queue_head == nullptr)
				{
					unitInfesting->order_simple(units_dat::return_to_idle_order[unitInfesting->id], false);
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
				&& unit_can_be_infested(unitInfested)
				&& unitInfested->main_order_id != OrderId::InfestMine1)
			{
				helper::hide_and_disable_unit(unitInfesting);								// hide the infesting unit
				unitInfested->order_to(OrderId::InfestMine1, unitInfesting);				// order the target to be infested, giving it the infesting unit as order target
				unitInfesting->main_order_state = 3;
				unitInfesting->remaining_build_time = 60;									// timer before reappearing
				bStopFunction = true;
			}

			if (!bStopFunction)
			{ // EA377
				// "move to invalid target" logic...I think
				helper::set_resource_target(unitInfesting);
				if (unitInfesting->move_target.pt.x != unitInfesting->next_target_waypoint.x
					|| unitInfesting->move_target.pt.y != unitInfesting->next_target_waypoint.y)
				{
					unitInfesting->next_target_waypoint.x = unitInfesting->move_target.pt.x;
					unitInfesting->next_target_waypoint.y = unitInfesting->move_target.pt.y;
				}
				//EA39E
				if (unitInfesting->order_queue_head != nullptr)
				{
					helper::force_order_done(unitInfesting);
				}
				else
				{
					unitInfesting->order(units_dat::return_to_idle_order[unitInfesting->id], 0, 0, nullptr, UnitId::None, false);
					helper::force_order_done(unitInfesting);
				}
			}
		}
	}
}