#include "larva_creep_spawn.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	void orders_BuildingLand(Unit* unit) // 00464360
	{
		Target unitTarget;
		unitTarget.unit = unit->orderTarget.unit;
		unitTarget.pt = unit->orderTarget.pt;
		Point16 coords;
		coords.x = 0;
		coords.y = 0;

		if (unit->status & UnitStatus::GroundedBuilding)
		{
			unit->userActionFlags |= 1;
			if (unit->mainOrderId != OrderId::Die)
			{
				bool bLoopBreak = false;
				while (unit->orderQueueTail != nullptr && !bLoopBreak)
				{
					bLoopBreak =
						!orders_dat::CanBeInterrupted[unit->orderQueueTail->orderId] &&
						(unit->orderQueueTail->orderId != OrderId::BuildingLiftoff);
					if (!bLoopBreak) {
						helper::remove_order_from_unit_queue(unit->orderQueueTail, unit);
					}
				}
				unit->performAnotherOrder(OrderId::BuildingLiftoff, coords.x, coords.y,	nullptr, 0);
			}
			prepareForNextOrder(unit);
			if (unit->mainOrderId != OrderId::Die)
			{
				while (unit->orderQueueTail != nullptr && unit->orderQueueTail->orderId == OrderId::BuildingLand)
				{
					helper::remove_order_from_unit_queue(unit->orderQueueTail, unit);
				}
				unit->performAnotherOrder(OrderId::BuildingLand, unitTarget.pt.x, unitTarget.pt.y, unitTarget.unit, UnitId::None);
			}
			unit->order(units_dat::ReturnToIdleOrder[unit->id], 0, 0, nullptr, UnitId::None, false);
		}
		else if (unit->mainOrderState == 0)
		{
			// Plan vertical movement toward ground
			helper::changeMovementTargetMoveTarget2(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y - unit->sprite->images.tail->verticalOffset);
			unit->mainOrderState = 1;
		}
		else if (unit->mainOrderState == 1)
		{
			if (unit->getMovableState() != 0)
			{
				s32 x = units_dat::BuildingDimensions[unit->id].x;
				s32 y = units_dat::BuildingDimensions[unit->id].y;
				if (x < 0)
				{
					x += 1; if (y < 0) y += 1;
				}

				x /= 2; y /= 2;
				x = unit->orderTarget.pt.x - x;
				y = unit->orderTarget.pt.y - y;
				if (x < 0)
				{
					x += 31; if (y < 0) y += 31;
				}
				x /= 32; y /= 32;

				// Check for units occupying the landing zone before the final vertical movement started?
				u32 return_00473FB0 = helper::canBuildOnTileLocation(unit, unit->playerId, x, y, unit->id, 0, 0, 1, 0);

				if (return_00473FB0 != 0)
				{
					// Probably play "landing sequence interrupted"
					helper::playBuildingLandSound(unit);

					if (unit->orderQueueHead != nullptr
					&&	unit->orderQueueHead->orderId != OrderId::PlaceAddon)
					{
						helper::remove_order_from_unit_queue(unit->orderQueueHead, unit);
					}
					unit->orderToIdle();
				}
				else
				{
					// Use a AddFogMask() function after some coordinates calculations
					helper::SetBuildingTileFlag(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

					unit->building.landingTimer = 1;

					// Plan movement again after latest updates
					helper::changeMovementTargetMoveTarget2(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

					if (unit->sprite->images.tail->paletteType == PaletteType::RLE_SHADOW)
					{
						unit->sprite->images.tail->flags |= ImageFlags::DontUpdate;
					}

					unit->status |= UnitStatus::CanNotReceiveOrders;

					unit->flingyTopSpeed = 256;
					helper::SetUnitMovementSpeed(unit, 256);

					unit->mainOrderState = 2;
				}
			}
		}
		else if (unit->mainOrderState == 2)
		{
			bool bStopThere = false;
			if (!helper::isUnitPositions2Equal(unit))
			{
				bStopThere = ((unit->velocityDirection1 - scbw::getAngle(unit->sprite->position.x, unit->sprite->position.y, unit->nextTargetWaypoint.x, unit->nextTargetWaypoint.y)) != 0);
			}
			if (!bStopThere)
			{
				unit->sprite->play_iscript_animation(IscriptAnimation::Landing);
				unit->mainOrderState = 3;
			}
		}

		// There's no "else" here, mainOrderState = 2 can continue directly into mainOrderState = 3 if mainOrderState was updated
		if (unit->mainOrderState == 3)
		{
			if ((unit->getMovableState() != 0) && (unit->orderSignal & 0x10))
			{
				unit->orderSignal -= 0x10;

				if (unit->orderTarget.pt.x >= 0 && unit->orderTarget.pt.x < mapTileSize->width * 32 && 
					unit->orderTarget.pt.y >= 0 && unit->orderTarget.pt.y < mapTileSize->height * 32) { // New sanity check; Attempt on fixing clear tile flags
					helper::clear_building_from_tile_flag(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
				}

				helper::end_collision_tracking(unit); // Recalculate position and update effect on surroundings?
				helper::toggle_pathing_related(unit); // Toggle pathing-related stuff to prepare to no longer use pathing

				unit->sprite->elevationLevel = 4;
				helper::setUnitPathing(unit, 0);
				scbw::set_unit_position(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

				helper::start_collision_tracking(unit); // Maybe update orders targeting the unit, set repulse for units in surroundings, and some other things?

				// Maybe the landing unit would get excluded from multi-units selection if that could happen in normal Starcraft gameplay? That may be what this function does.
				if (unit->sprite->flags & SpriteFlags::Selected && *clientSelectionCount > 1)
				{
					helper::function_004C3B40(unit);
				}

				helper::set_shadow_y(unit->sprite, 0);
				if (unit->sprite->images.tail->paletteType == PaletteType::RLE_SHADOW)
				{
					unit->sprite->images.tail->flags &= ~ImageFlags::DontUpdate;
				}
				helper::create_landing_dust(unit->sprite);
				scbw::refresh_console();
				helper::crush_immobile_units(unit);

				if (unit->orderQueueHead != nullptr)
				{
					bool bGotMoveOrder;
					if (unit->orderQueueHead->orderId == OrderId::Move
					||	unit->orderQueueHead->orderId == OrderId::Follow)
					{
						bGotMoveOrder = true;
					}
					else
					{
						bGotMoveOrder = false;
						Order* order = unit->orderQueueHead;
						if (order->orderId != OrderId::PlaceAddon)
						{
							order = nullptr;
						}

						while (unit->orderQueueTail != order)
						{
							helper::remove_order_from_unit_queue(unit->orderQueueTail, unit);
						}
						if (unit->orderQueueHead == nullptr)
						{
							unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
						}
					}

					if (bGotMoveOrder)
					{
						// This causes the landed building to immediately take off again to execute the queued move/follow order
						helper::prepend_order_targeting_nothing(unit, unit->orderQueueHead, OrderId::BuildingLiftoff);
					}						
				}
				// Some units iterations, idle orders affectations, and unknown order assignment?
				// Maybe affect another selected unit depending on attempt at giving order?
				helper::flying_building_lift_if_still_blocked(unit);
				helper::force_order_done(unit);

				UnitExt* unitExt = globals.get_unit_ext(unit);
				if (unitExt->values[ValueBlissfulEmbraceOriginalPlayer]) {
					scbw::spawn_custom_creep(unit);
				}

				Unit* addon = helper::connectAddonCheck(unit);
				if (addon != nullptr)
				{
					helper::completeAddon(unit, addon);
				}
				if (*IS_PLACING_BUILDING)
				{
					helper::placebuildingProc();

					// Add the last minute occupation info of landing building to update an ongoing attempt to choose a building or landing location?
					if (!helper::canStillPlaceBuilding())
					{
						helper::refresh_layers_3_and_4();
						helper::end_building_placement();
					}
				}
			}
		}
	}

	void orders_BuildingLiftoff(Unit* unit) // 004649B0
	{
		if ((unit->status & UnitStatus::GroundedBuilding)
		&& (units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding))
		{
			if (unit->building.addon != nullptr)
			{
				helper::disconnectFromAddOn(unit);
			}
			if (unit->isFactory()) {
				UnitExt* unitExt = globals.get_unit_ext(unit);
				unit->rally.unit = unit;
			}

			helper::end_collision_tracking(unit); // Update globally known position and may remove repulse tile if no longer needed
			helper::modify_building_grounded_status(unit); // Disable GroundedBuilding status and prepare to enable pathing?

			unit->sprite->elevationLevel = 12;
			helper::setUnitPathing(unit, 1);
			helper::start_collision_tracking(unit); // Maybe update orders targetting the unit, set repulse for units in surroundings,and  some others things?

			if (unit->sprite->images.tail->paletteType == PaletteType::RLE_SHADOW)
			{
				unit->sprite->images.tail->flags |= ImageFlags::DontUpdate;
			}

			scbw::refresh_console();
			if (unit->sprite->position.x != unit->nextTargetWaypoint.x
			||	unit->sprite->position.y - 42 != unit->nextTargetWaypoint.y)
			{
				unit->nextTargetWaypoint.x = unit->sprite->position.x;
				unit->nextTargetWaypoint.y = unit->sprite->position.y - 42;
			}

			if (unit->id == UnitId::ZergInfestedCommandCenter)
			{
				helper::update_creep_disappearance_unit(unit);
			}
			scbw::update_custom_creep_disappearance(unit);

			if (*IS_PLACING_BUILDING)
			{
				helper::placebuildingProc();
			}
			unit->mainOrderId = OrderId::Liftoff;
		}
		else
		{
			if (unit->orderQueueHead == nullptr)
			{
				unit->order(units_dat::ReturnToIdleOrder[unit->id], 0, 0, nullptr, UnitId::None, 0);
			}
			helper::force_order_done(unit);
		}
	}
}