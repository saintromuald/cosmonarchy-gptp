#include "nuke_orders.h"
#include <SCBW/api.h>

namespace hooks
{
	void orders_NukeTrack(Unit* unit)
	{

		if (unit->mainOrderState != 0)
		{

			if (unit->mainOrderState == 6)
			{

				if (
					unit->connectedUnit == nullptr ||
					unit->connectedUnit->mainOrderState == 5
					)
				{ //64D3F

					Image* current_image = unit->sprite->images.head;

					if (unit->orderQueueHead == nullptr)
						unit->order_simple(units_dat::ReturnToIdleOrder[unit->id], false);

					helper::force_order_done(unit);
					helper::change_unit_buttonset(unit, unit->id);
					scbw::refresh_console();

					while (current_image != nullptr)
					{
						current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
						current_image = current_image->link.next;
					}

					if (unit->building.ghostNukeMissile != nullptr)
					{

						current_image = (unit->building.ghostNukeMissile)->sprite->images.head;

						while (current_image != nullptr)
						{
							current_image->play_iscript_animation(IscriptAnimation::Death);
							current_image = current_image->link.next;
						}

						unit->building.ghostNukeMissile = nullptr;

					}

					//64DD6
					helper::function_0043DB50(unit);

				}

			}

		}
		else
		{ //64DE3

			Thingy* nukeDot;

			unit->sprite->play_iscript_animation(IscriptAnimation::SpecialState1, true);

			nukeDot = helper::create_thingy(
				SpriteId::Nuke_Target_Dot,
				(unit->connectedUnit)->orderTarget.pt.x,
				(unit->connectedUnit)->orderTarget.pt.y,
				unit->player
			);

			unit->building.ghostNukeMissile = (Unit*)nukeDot;

			if (nukeDot != nullptr)
			{
				nukeDot->sprite->elevationLevel = unit->sprite->elevationLevel + 1;
				helper::set_thingy_visibility_flags(nukeDot);
			}

			unit->mainOrderState = 6;
			scbw::decloak_proc(unit);

		}

	}

	;

	void orders_NukeGround(Unit* unit)
	{

		u32 sightRange, distance;

		if (
			unit->nextTargetWaypoint.x != unit->orderTarget.pt.x ||
			unit->nextTargetWaypoint.y != unit->orderTarget.pt.y
			)
		{
			unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
			unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
		}

		sightRange = unit->get_sight_range(false) * 32;

		if (unit->unit_ai != nullptr)
		{

			distance = scbw::get_distance_fast(
				unit->orderTarget.pt.x * 256,
				unit->orderTarget.pt.y * 256,
				unit->halt.x,
				unit->halt.y
			);

			if (distance / 256 <= sightRange * 3)
				helper::AI_orderUnitCloaking(unit);

		}

		//79267
		distance = scbw::get_distance_fast(
			unit->orderTarget.pt.x * 256,
			unit->orderTarget.pt.y * 256,
			unit->halt.x,
			unit->halt.y
		);

		if (distance / 256 > sightRange)
		{

			if (unit->recentOrderTimer == 0)
				helper::set_move_target_xy(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

		}
		else
		{ //792AE

			s32 direction;
			bool bEndThere = false;

			helper::set_resource_target(unit);

			if (
				unit->nextTargetWaypoint.x != unit->position.x ||
				unit->nextTargetWaypoint.y != unit->position.y
				)
			{

				direction =
					unit->currentDirection1 -
					scbw::get_angle(
						unit->sprite->position.x,
						unit->sprite->position.y,
						unit->nextTargetWaypoint.x,
						unit->nextTargetWaypoint.y
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
							current_unit->building.silo.isReady
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
					current_unit->building.silo.isReady = false;
					helper::play_sound_from_unit(SoundId::Terran_GHOST_TGhLas00_wav, unit, 1, 0);
					nuke->order_to(OrderId::NukeLaunch, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
					nuke->connectedUnit = unit;
					helper::setAllOverlayDirectionsGeneric(nuke, 0);
					helper::show_and_enable_unit(nuke);
					unit->connectedUnit = nuke;
					unit->order_to(OrderId::NukeTrack, unit->sprite->position.x, unit->sprite->position.y);
					if (unit->is_frozen())
					{ // 004793C4
						if (units_dat::BaseProperty[unit->id] & UnitProperty::Building)
						{
							unit->currentButtonSet = UnitId::Buttons_CancelNukeStrike;
						}
					}
					else
					{
						unit->currentButtonSet = UnitId::Buttons_CancelNukeStrike;
					}
					scbw::refresh_console();
				}
			}
		}
	}
}