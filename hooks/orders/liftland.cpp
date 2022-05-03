#include "larva_creep_spawn.h"
#include <SCBW/api.h>
#include "globals.h"

namespace hooks
{
	void orders_BuildingLand(Unit* unit)
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
				while (unit->orderQueueTail != nullptr && !bLoopBreak) // 004643B0
				{
					bLoopBreak =
						!orders_dat::CanBeInterrupted[unit->orderQueueTail->orderId] &&
						(unit->orderQueueTail->orderId != OrderId::BuildingLiftoff);
					if (!bLoopBreak) {
						helper::removeOrderFromUnitQueue(unit, unit->orderQueueTail);
					}
				}
				unit->performAnotherOrder(OrderId::BuildingLiftoff, coords.x, coords.y,	nullptr, 0); // 004643D3:
			}
			prepareForNextOrder(unit); // 004643E8
			if (unit->mainOrderId != OrderId::Die)
			{
				while (unit->orderQueueTail != nullptr && unit->orderQueueTail->orderId == OrderId::BuildingLand)
				{
					helper::removeOrderFromUnitQueue(unit, unit->orderQueueTail);
				}
				unit->performAnotherOrder(OrderId::BuildingLand, unitTarget.pt.x, unitTarget.pt.y, unitTarget.unit, UnitId::None); // 0046440A:
			}
			unit->order(units_dat::ReturnToIdleOrder[unit->id], 0, 0, nullptr, UnitId::None, false); // 00464422
		}
		else
			//64450
			if (unit->mainOrderState == 0)
			{
				//plan vertical movement toward ground
				helper::changeMovementTargetMoveTarget2(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y - unit->sprite->images.tail->verticalOffset);
				unit->mainOrderState = 1;
			}
			else
				if (unit->mainOrderState == 1)
				{

					//6448A
					if (unit->getMovableState() != 0)
					{

						s32 x, y;

						x = units_dat::BuildingDimensions[unit->id].x;
						y = units_dat::BuildingDimensions[unit->id].y;

						if (x < 0) x += 1; if (y < 0) y += 1;

						x /= 2; y /= 2;

						x = unit->orderTarget.pt.x - x;
						y = unit->orderTarget.pt.y - y;

						if (x < 0) x += 31; if (y < 0) y += 31;

						x /= 32; y /= 32;

						//check for units occupying the landing zone before
						//the final vertical movement started?
						u32 return_00473FB0 =
							helper::canBuildOnTileLocation(unit, unit->playerId, x, y, unit->id, 0, 0, 1, 0);

						if (return_00473FB0 != 0)
						{

							//64504:

							//probably play "landing sequence interrupted"
							helper::playBuildingLandSound(unit);

							if (unit->orderQueueHead != nullptr &&
								unit->orderQueueHead->orderId != OrderId::PlaceAddon
								)
								helper::removeOrderFromUnitQueue(unit, unit->orderQueueHead);

							unit->orderToIdle();


						}
						else
						{

							//6452B:

							//use a AddFogMask() function after some coordinates calculations
							helper::SetBuildingTileFlag(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

							unit->building.landingTimer = 1;

							//plan movement again after latest updates
							helper::changeMovementTargetMoveTarget2(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

							if (unit->sprite->images.tail->paletteType == PaletteType::RLE_SHADOW)
								unit->sprite->images.tail->flags |= CImage_Flags::DontUpdate;

							unit->status |= UnitStatus::CanNotReceiveOrders;

							unit->flingyTopSpeed = 256;
							helper::SetUnitMovementSpeed(unit, 256);

							unit->mainOrderState = 2;

						} //if(return_00473FB0 == 0)

					} //if(unit->getMovableState()) != 0)

				} //if(unit->mainOrderState == 1)
				else
					if (unit->mainOrderState == 2)
					{

						bool bStopThere = false;

						//64587:
						if (!helper::isUnitPositions2Equal(unit))
						{

							s32 someValue =
								scbw::getAngle(
									unit->sprite->position.x,
									unit->sprite->position.y,
									unit->nextTargetWaypoint.x,
									unit->nextTargetWaypoint.y
								);

							bStopThere = ((unit->velocityDirection1 - someValue) != 0);

						}

						//645B7
						if (!bStopThere)
						{
							unit->sprite->playIscriptAnim(IscriptAnimation::Landing);
							unit->mainOrderState = 3;
						}


					} //if(unit->mainOrderState == 2)

					//there's no "else" here, mainOrderState = 2 can continue directly
					//into mainOrderState = 3 if mainOrderState was updated

		if (unit->mainOrderState == 3)
		{

			//645C7:
			//orderSignal is set by IScript animation
			if ((unit->getMovableState() != 0) && (unit->orderSignal & 0x10))
			{

				Unit* addon;

				unit->orderSignal -= 0x10;

				//use a SetFogMask() function after some coordinates preparation
				helper::function_00469EC0(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

				//recalculate position and update effect on
				//surroundings?
				helper::function_0046A560(unit);

				//toggle pathing-related stuff to prepare
				//to no longer use pathing
				helper::function_004634E0(unit);

				unit->sprite->elevationLevel = 4;
				helper::setUnitPathing(unit, 0);
				scbw::setUnitPosition(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);

				//maybe update orders targetting the unit,
				//set repulse for units in surroundings,and 
				//some others things?			
				helper::function_0046A5A0(unit);

				//maybe the landing unit would get excluded from multi-units selection
				//if that could happen in normal Starcraft gameplay?That may be what
				//this function do.
				if (unit->sprite->flags & SpriteFlags::Selected && *clientSelectionCount > 1)
					helper::function_004C3B40(unit);

				//64648

				//change the verticalOffset of images besides shadow to 0
				helper::function_00498150(unit->sprite, 0);

				if (unit->sprite->images.tail->paletteType == PaletteType::RLE_SHADOW)
					unit->sprite->images.tail->flags &= ~CImage_Flags::DontUpdate;

				//probably what display the landing dust
				helper::function_004997E0(unit->sprite);

				scbw::refreshConsole();

				//crush units that managed to get unmovable there
				//(tanks in siege, burrowed zerg)
				helper::function_00464300(unit);

				if (unit->orderQueueHead != nullptr)
				{

					bool bGotMoveOrder;

					if (unit->orderQueueHead->orderId == OrderId::Move ||
						unit->orderQueueHead->orderId == OrderId::Follow
						)
						bGotMoveOrder = true;
					else
					{

						bGotMoveOrder = false;

						//6468F
						Order* order = unit->orderQueueHead;

						if (order->orderId != OrderId::PlaceAddon)
							order = nullptr;

						while (unit->orderQueueTail != order)
							helper::removeOrderFromUnitQueue(unit, unit->orderQueueTail);

						//646AF
						if (unit->orderQueueHead == nullptr)
							unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);

					}

					if (bGotMoveOrder)
						//646C7
						//this cause the landed building to immediately take off again to
						//execute the queued move/follow order
						helper::prepend_order_targeting_nothing(unit, unit->orderQueueHead, OrderId::BuildingLiftoff);

				} //if(unit->orderQueueHead != nullptr)

				//646CE

				//some units iterations, idle orders affectations,
				//and unknown order assignment?
				//maybe affect another selected unit depending on
				//attempt at giving order?
				helper::function_00463640(unit);

				helper::actUnitReturnToIdle(unit);

				addon = helper::connectAddonCheck(unit);
				if (addon != nullptr)
					helper::completeAddon(unit, addon);

				//646EC
				if (*IS_PLACING_BUILDING)
				{

					helper::placebuildingProc();

					//add the last minute occupation info
					//of landing building to update an
					//ongoing attempt to choose a building
					//or landing location?
					if (!helper::canStillPlaceBuilding())
					{

						helper::refreshLayers3And4();

						//refresh various stuff, including
						//buttonset, dialogs...
						helper::endBuildingPlacement();

					}

				}

			} //if( (unit->getMovableState() != 0) && (unit->orderSignal & 0x10) )

		} //if(unit->mainOrderState == 3)

	}  //void orders_BuildingLand(Unit* unit)

	;

	void orders_BuildingLiftoff(Unit* unit)
	{

		if ((unit->status & UnitStatus::GroundedBuilding) &&
			(units_dat::BaseProperty[unit->id] & UnitProperty::FlyingBuilding)
			)
		{

			//649D5
			if (unit->building.addon != nullptr)
				helper::disconnectFromAddOn(unit);

			if (unit->isFactory()) {
				UnitExt* unitExt = globals.GetUnitExt(unit);
				unit->rally.unit = unit;
			}

			//update globally known position and may
			//remove repulse tile if no longer needed
			helper::function_0046A560(unit);

			//disable GroundedBuilding status and
			//prepare to enable pathing?
			helper::function_00463470(unit);

			//64A05
			unit->sprite->elevationLevel = 12;
			helper::setUnitPathing(unit, 1);

			//maybe update orders targetting the unit,
			//set repulse for units in surroundings,and 
			//some others things?
			helper::function_0046A5A0(unit);

			if (unit->sprite->images.tail->paletteType == PaletteType::RLE_SHADOW)
				unit->sprite->images.tail->flags |= CImage_Flags::DontUpdate;

			//equivalent to 64A40
			scbw::refreshConsole();

			if (unit->sprite->position.x != unit->nextTargetWaypoint.x ||
				unit->sprite->position.y - 42 != unit->nextTargetWaypoint.y
				)
			{
				//64A76
				unit->nextTargetWaypoint.x = unit->sprite->position.x;
				unit->nextTargetWaypoint.y = unit->sprite->position.y - 42;
			}

			//64A7E
			//probably update things related to creep since
			//there may be some under or around an infested
			//command center, return value unused
			if (unit->id == UnitId::ZergInfestedCommandCenter)
				helper::function_0047DF90(unit);

			if (*IS_PLACING_BUILDING)
				helper::placebuildingProc();

			unit->mainOrderId = OrderId::Liftoff;

		} //GroundedBuilding && FlyingBuilding
		else
		{

			//64AA2
			if (unit->orderQueueHead == nullptr)
				unit->order(units_dat::ReturnToIdleOrder[unit->id], 0, 0, nullptr, UnitId::None, 0);

			helper::actUnitReturnToIdle(unit);

		}
	}
}