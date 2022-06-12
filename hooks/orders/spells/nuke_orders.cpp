#include "nuke_orders.h"
#include <SCBW/api.h>

namespace hooks
{
	void orders_NukeTrack(Unit* unit)
	{

		if (unit->main_order_state != 0)
		{

			if (unit->main_order_state == 6)
			{

				if (
					unit->connected_unit == nullptr ||
					unit->connected_unit->main_order_state == 5
					)
				{ //64D3F

					Image* current_image = unit->sprite->images.head;

					if (unit->order_queue_head == nullptr)
						unit->order_simple(units_dat::ReturnToIdleOrder[unit->id], false);

					helper::force_order_done(unit);
					helper::change_unit_buttonset(unit, unit->id);
					scbw::refresh_console();

					while (current_image != nullptr)
					{
						current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
						current_image = current_image->link.next;
					}

					if (unit->building.nuke_missile != nullptr)
					{

						current_image = (unit->building.nuke_missile)->sprite->images.head;

						while (current_image != nullptr)
						{
							current_image->play_iscript_animation(IscriptAnimation::Death);
							current_image = current_image->link.next;
						}

						unit->building.nuke_missile = nullptr;

					}

					//64DD6
					helper::ai_return_to_nearest_base_forced(unit);

				}

			}

		}
		else
		{ //64DE3

			Thingy* nukeDot;

			unit->sprite->play_iscript_animation(IscriptAnimation::SpecialState1, true);

			nukeDot = helper::create_thingy(
				SpriteId::Nuke_Target_Dot,
				(unit->connected_unit)->order_target.pt.x,
				(unit->connected_unit)->order_target.pt.y,
				unit->player
			);

			unit->building.nuke_missile = (Unit*)nukeDot;

			if (nukeDot != nullptr)
			{
				nukeDot->sprite->elevation_level = unit->sprite->elevation_level + 1;
				helper::set_thingy_visibility_flags(nukeDot);
			}

			unit->main_order_state = 6;
			scbw::decloak_proc(unit);

		}

	}

	;

	void orders_NukeGround(Unit* unit)
	{

		u32 sightRange, distance;

		if (
			unit->next_target_waypoint.x != unit->order_target.pt.x ||
			unit->next_target_waypoint.y != unit->order_target.pt.y
			)
		{
			unit->next_target_waypoint.x = unit->order_target.pt.x;
			unit->next_target_waypoint.y = unit->order_target.pt.y;
		}

		sightRange = unit->get_sight_range(false) * 32;

		if (unit->unit_ai != nullptr)
		{

			distance = scbw::get_distance_fast(
				unit->order_target.pt.x * 256,
				unit->order_target.pt.y * 256,
				unit->halt.x,
				unit->halt.y
			);

			if (distance / 256 <= sightRange * 3)
				helper::ai_order_unit_cloaking(unit);

		}

		//79267
		distance = scbw::get_distance_fast(
			unit->order_target.pt.x * 256,
			unit->order_target.pt.y * 256,
			unit->halt.x,
			unit->halt.y
		);

		if (distance / 256 > sightRange)
		{

			if (unit->recent_order_timer == 0)
				helper::set_move_target_xy(unit, unit->order_target.pt.x, unit->order_target.pt.y);

		}
		else
		{ //792AE

			s32 direction;
			bool bEndThere = false;

			helper::set_resource_target(unit);

			if (
				unit->next_target_waypoint.x != unit->position.x ||
				unit->next_target_waypoint.y != unit->position.y
				)
			{

				direction =
					unit->current_direction_1 -
					scbw::get_angle(
						unit->sprite->position.x,
						unit->sprite->position.y,
						unit->next_target_waypoint.x,
						unit->next_target_waypoint.y
					);

				if (direction < -1 || direction > 1)
					bEndThere = true;

			}

			//792F9
			if (!bEndThere)
			{

				bool bLoopEnd = false;
				Unit* current_unit = firstPlayerUnit->unit[unit->player];

				while (!bLoopEnd)
				{

					if (current_unit == nullptr)
						bLoopEnd = true;
					else
						if (current_unit->id == UnitId::TerranNuclearSilo &&
							current_unit->building.silo.is_ready
							)
							bLoopEnd = true;
						else
							current_unit = current_unit->player_link.next;

				}
				if (current_unit == nullptr)
				{
					unit->order_to_idle();
				}
				else
				{ // 00479333
					Unit* nuke = current_unit->building.silo.nuke;
					current_unit->building.silo.nuke = nullptr;
					current_unit->building.silo.is_ready = false;
					helper::play_sound_from_unit(SoundId::Terran_GHOST_TGhLas00_wav, unit, 1, 0);
					nuke->order_to(OrderId::NukeLaunch, unit->order_target.pt.x, unit->order_target.pt.y);
					nuke->connected_unit = unit;
					helper::set_all_overlay_directions_generic(nuke, 0);
					helper::show_and_enable_unit(nuke);
					unit->connected_unit = nuke;
					unit->order_to(OrderId::NukeTrack, unit->sprite->position.x, unit->sprite->position.y);
					if (unit->is_frozen())
					{ // 004793C4
						if (units_dat::BaseProperty[unit->id] & UnitProperty::Building)
						{
							unit->current_button_set = UnitId::Buttons_CancelNukeStrike;
						}
					}
					else
					{
						unit->current_button_set = UnitId::Buttons_CancelNukeStrike;
					}
					scbw::refresh_console();
				}
			}
		}
	}
}