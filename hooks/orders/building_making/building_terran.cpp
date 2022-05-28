#include "building_terran.h"
#include "building_modifier.h"
#include <SCBW/api.h>
#include <globals.h>

namespace hooks
{
	void orders_TerranBuildSelf(Unit* building)
	{
		if (building->mainOrderState == 0)
		{
			if (building->hitPoints > 20 * units_dat::MaxHitPoints[building->id] / 100)
				building->mainOrderState = 1;
		}
		else if (building->mainOrderState == 1)
		{
			Image* current_image = building->sprite->images.head;
			while (current_image != nullptr)
			{
				current_image->play_iscript_animation(IscriptAnimation::SpecialState1);
				current_image = current_image->link.next;
			}
			building->mainOrderState = 2;
		}
		else if (building->mainOrderState == 2)
		{
			if (building->hitPoints > 40 * units_dat::MaxHitPoints[building->id] / 100)
				building->mainOrderState = 3;
		}
		else if (building->mainOrderState == 3)
		{
			Image* current_image = building->sprite->images.head;
			while (current_image != nullptr)
			{
				current_image->play_iscript_animation(IscriptAnimation::SpecialState2);
				current_image = current_image->link.next;
			}
			building->mainOrderState = 4;
		}
		else if (building->mainOrderState == 4)
		{
			if (building->hitPoints > 60 * units_dat::MaxHitPoints[building->id] / 100)
			{
				building->mainOrderState = 5;
			}
		}
		else if (building->mainOrderState == 5)
		{
			helper::init_building(building, 0);
			Image* current_image = building->sprite->images.head;
			while (current_image != nullptr)
			{
				current_image->play_iscript_animation(IscriptAnimation::AlmostBuilt);
				current_image = current_image->link.next;
			}
			building->mainOrderState = 6;
		}
		else if (building->mainOrderState == 6)
		{
			if (building->hitPoints > 80 * units_dat::MaxHitPoints[building->id] / 100)
			{
				building->order_computer(OrderId::Nothing2);
			}
		}
	}

	// Go to build on an existing building
	void orders_SCVBuild2(Unit* unit)
	{
		Unit* builtUnit = unit->orderTarget.unit;

		BuildingModifier modifier = get_building_modifier(builtUnit, unit);

		if (builtUnit != nullptr && !(builtUnit->status & UnitStatus::Completed))
		{
			bool jump_to_switch_1 = false;
			if (unit->mainOrderState > 8 ||
				unit->mainOrderState == 1)
			{
				jump_to_switch_1 = true;
			}
			if (unit->mainOrderState == 0)
			{
				if (helper::order_to_move_to_target(unit, builtUnit))
					unit->mainOrderState = 2;
			}
			if (unit->mainOrderState == 2)
			{
				u32 movableState = unit->getMovableState();
				if (movableState == MovableState::UnmovableAtDestination)
				{
					unit->orderToIdle();
				}
				else if (movableState == MovableState::ReachedDestination)
				{
					scbw::refresh_console();
					if (builtUnit->connectedUnit != nullptr &&
						builtUnit->connectedUnit != unit &&
						(builtUnit->connectedUnit)->orderTarget.unit == builtUnit)
					{
						unit->orderToIdle(); // another SCV is already on the job
					}
					else // 00467AF9
					{
						builtUnit->connectedUnit = unit;
						unit->status &= ~UnitStatus::IsNormal;	//remove UnitStatus::IsNormal if present
						unit->sprite->elevationLevel = units_dat::Elevation[unit->id] + 1;
						unit->orderSimple(OrderId::ResetCollision1, false);
						unit->orderSimple(units_dat::ReturnToIdleOrder[unit->id], false);
						// DF: Removed an unecessary delay for resuming construction progress. There might be a small chance for the SCV to remain outside of the building's bounds if
						//    the randomized orderTarget is very near the same spot the SCV resumed construction, but I think the helper::TerrainCollision_? condition in State 4 should handle it.
						//if (helper::ChangeMovementTargetMoveTarget2(unit, builtUnit->sprite->position.x, builtUnit->sprite->position.y))
						unit->mainOrderState = 3;
					}
				}
			}
			else if (unit->mainOrderState == 3)
			{
				if (unit->getMovableState() != MovableState::NotReachedDestination)
				{
					unit->mainOrderState = 4;
				}
			}
			else if (unit->mainOrderState == 4)
			{
				s16 x_min, x_max, y_min, y_max;
				x_min = builtUnit->sprite->position.x - units_dat::UnitBounds[builtUnit->id].left + units_dat::UnitBounds[unit->id].left;
				x_max = builtUnit->sprite->position.x + units_dat::UnitBounds[builtUnit->id].right - units_dat::UnitBounds[unit->id].right;
				y_min = builtUnit->sprite->position.y - units_dat::UnitBounds[builtUnit->id].top + units_dat::UnitBounds[unit->id].top;
				y_max = builtUnit->sprite->position.y + units_dat::UnitBounds[builtUnit->id].bottom - units_dat::UnitBounds[unit->id].bottom;
				if (x_min > x_max) // DF: If worker is wider than building, don't make him shuffle around horizontally. Might be safer to compare units_dat bounds, but this should be true for like 99% of cases.
				{
					x_min = builtUnit->sprite->position.x;
					x_max = builtUnit->sprite->position.y;
				}
				if (y_min > y_max) // DF: If worker is taller than building, don't make him shuffle around vertically.
				{
					y_min = 0;
					y_max = 0;
				}

				unit->orderTarget.pt.x = helper::rand_between(x_min, x_max, 9); // DF: Can't tell what effect the third value has on the randomization.
				unit->orderTarget.pt.y = helper::rand_between(y_min, y_max, 9);
				helper::fix_target_location(&unit->orderTarget.pt,unit->id);
				if (helper::unk_terrain_collision_2(unit, unit->orderTarget.pt.x - unit->sprite->position.x, unit->orderTarget.pt.y - unit->sprite->position.y) != 0)
				{
					unit->orderTarget.pt.x = helper::rand_between(x_min, x_max, 9);
					unit->orderTarget.pt.y = helper::rand_between(y_min, y_max, 9);
					helper::fix_target_location(&unit->orderTarget.pt, unit->id);
				}

				if (helper::changeMovementTargetMoveTarget2(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y) != 0)
				{
					unit->mainOrderState = 5;
					jump_to_switch_1 = true;
				}
			}
			else if (unit->mainOrderState == 5)
			{
				unit->mainOrderState = 6;
				jump_to_switch_1 = true;
			}
			else if (unit->mainOrderState == 6)
			{
				if (!helper::isDistanceGreaterThanHaltDistance(
					unit,
					unit->orderTarget.pt.x,
					unit->orderTarget.pt.y,
					20))
				{
					jump_to_switch_1 = true;
				}
				else
				{
					s32 angle;
					helper::set_resource_target(unit);
					helper::setSecondaryWaypoint(unit, (unit->orderTarget.unit)->sprite->position.x, (unit->orderTarget.unit)->sprite->position.y);
					angle = scbw::getAngle(unit->sprite->position.x, unit->sprite->position.y, unit->nextTargetWaypoint.x, unit->nextTargetWaypoint.y);
					unit->orderTarget.pt.x = (angleDistance[angle].x * 20) / 256 + unit->sprite->position.x;
					unit->orderTarget.pt.y = (angleDistance[angle].y * 20) / 256 + unit->sprite->position.y;
					unit->mainOrderState = 7;
					jump_to_switch_1 = true;
				}
			}
			else if (unit->mainOrderState == 7)
			{
				if (!helper::isUnitPositions2Equal(unit))
				{
					s32 angle = scbw::getAngle(
						unit->sprite->position.x,
						unit->sprite->position.y,
						unit->nextTargetWaypoint.x,
						unit->nextTargetWaypoint.y
					);
					angle = unit->currentDirection1 - angle;
					if (angle < 0)
					{
						angle += 256;
					}
					if (angle > weapons_dat::AttackAngle[unit->getGroundWeapon()])
					{
						jump_to_switch_1 = true;
					}
					else // 00467E4A
					{
						u8 rand_value = helper::RandomizeShort(10);
						Image* current_image = unit->sprite->images.head;
						unit->mainOrderTimer = (rand_value & 63) + 30;
						while (current_image != nullptr)
						{
							current_image->play_iscript_animation(IscriptAnimation::AlmostBuilt);
							current_image = current_image->link.next;
						}
						unit->mainOrderState = 8;
						jump_to_switch_1 = true;
					}
				}
				else
				{
					u8 rand_value = helper::RandomizeShort(10);
					Image* current_image = unit->sprite->images.head;
					unit->mainOrderTimer = (rand_value & 63) + 30;
					while (current_image != nullptr)
					{
						current_image->play_iscript_animation(IscriptAnimation::AlmostBuilt);
						current_image = current_image->link.next;
					}
					unit->mainOrderState = 8;
					jump_to_switch_1 = true;
				}
			}
			else if (unit->mainOrderState == 8)
			{
				if (unit->mainOrderTimer != 0)
				{
					jump_to_switch_1 = true;
				}
				else
				{
					Image* current_image = unit->sprite->images.head;
					while (current_image != nullptr)
					{
						current_image->play_iscript_animation(IscriptAnimation::GndAttkToIdle);
						current_image = current_image->link.next;
					}
					unit->mainOrderState = 4;
					jump_to_switch_1 = true;
				}
			}

			if (jump_to_switch_1)
			{
				u32 hpGain = builtUnit->buildRepairHpGain * modifier.multiplier;
				UnitExt* unitExt = globals.get_unit_ext(builtUnit);
				if (*CHEAT_STATE & CheatFlags::OperationCwal)
				{
					hpGain *= 16;
				}
				helper::ProgressBuild(builtUnit, hpGain, false);
				if (modifier.decrease)
				{
					builtUnit->remainingBuildTime -= modifier.decrease;
					if (builtUnit->remainingBuildTime < 0)
					{
						builtUnit->remainingBuildTime = 0;
					}
				}
				if (builtUnit->status & UnitStatus::Completed)
				{
					Image* current_image = unit->sprite->images.head;
					bool bEndThere = false;
					while (current_image != nullptr)
					{
						current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
						current_image = current_image->link.next;
					}
					if (builtUnit != nullptr)
					{
						bool jump_to_67F3C = false;
						builtUnit->connectedUnit = nullptr;
						helper::playBuildingCompleteSound(unit);

						if (builtUnit->status & UnitStatus::Completed)
						{
							if (builtUnit->id == UnitId::TerranReservoir
								|| builtUnit->id == UnitId::ProtossAquifer
								|| builtUnit->id == UnitId::ZergExcisant)
							{
								if (builtUnit->playerId == unit->playerId)
								{
									jump_to_67F3C = true;
								}
							}
						}
						if (!jump_to_67F3C) // 00467F2C
						{
							if (builtUnit->id >= UnitId::ResourceMineralField &&
								builtUnit->id <= UnitId::ResourceMineralFieldType3)
							{
								jump_to_67F3C = true;
							}
						}
						if (jump_to_67F3C)
						{
							unit->orderTo(OrderId::Harvest1, builtUnit);
							bEndThere = true;
						}
					}
					if (!bEndThere) // 00467F4C
					{
						if (!(unit->orderQueueHead == nullptr))
						{
							unit->userActionFlags |= 1;
							prepareForNextOrder(unit);
						}
						else if (unit->unit_ai != nullptr) // 00467F6F
						{
							unit->order_computer(OrderId::ComputerAI);
						}
						else
						{
							unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
						}
					}
				}
			}
		}
		else // 00467ED0
		{
			Image* current_image = unit->sprite->images.head;
			bool bEndThere = false;
			while (current_image != nullptr)
			{
				current_image->play_iscript_animation(IscriptAnimation::WalkingToIdle);
				current_image = current_image->link.next;
			}
			if (builtUnit != nullptr)
			{
				bool jump_to_67F3C = false;
				builtUnit->connectedUnit = nullptr;
				helper::playBuildingCompleteSound(builtUnit);
				if (builtUnit->status & UnitStatus::Completed)
				{
					if (builtUnit->id == UnitId::TerranReservoir
						|| builtUnit->id == UnitId::ProtossAquifer
						|| builtUnit->id == UnitId::ZergExcisant)
					{
						if (builtUnit->playerId == unit->playerId)
						{
							jump_to_67F3C = true;
						}
					}
				}
				if (!jump_to_67F3C) // 00467F2C
				{
					if (builtUnit->id >= UnitId::ResourceMineralField &&
						builtUnit->id <= UnitId::ResourceMineralFieldType3)
					{
						jump_to_67F3C = true;
					}
				}
				if (jump_to_67F3C)
				{
					unit->orderTo(OrderId::Harvest1, builtUnit);
					bEndThere = true;
				}
			}
			if (!bEndThere) // 00467F4C
			{
				if (!(unit->orderQueueHead == nullptr))
				{
					unit->userActionFlags |= 1;
					prepareForNextOrder(unit);
				}
				else if (unit->unit_ai != nullptr) // 00467F6F
				{
					unit->order_computer(OrderId::ComputerAI);
				}
				else
				{
					unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
	}

	// Go to build a not yet existing building
	void orders_SCVBuild(Unit* unit)
	{
		if (unit->unit_ai != nullptr)
		{
			unit->status |= UnitStatus::IsGathering;
		}
		if (unit->mainOrderState == 1)
		{
			if (unit->sprite->position.x == unit->moveTarget.pt.x &&
				unit->sprite->position.y == unit->moveTarget.pt.y)
			{
				if (!(unit->status & UnitStatus::Unmovable)
					&& helper::isDistanceGreaterThanHaltDistance(
						unit,
						unit->orderTarget.pt.x,
						unit->orderTarget.pt.y,
						128)
					&& helper::beginBuilding(unit))
				{
					Unit* builtUnit;
					unit->status &= ~UnitStatus::IsNormal;
					unit->sprite->elevationLevel = units_dat::Elevation[unit->id] + 1;
					if (unit->orderQueueHead == nullptr) // 004680A5
					{
						helper::addOrderTargetingNothing(unit, nullptr, OrderId::ResetCollision1);
						helper::addOrderTargetingNothing(unit, nullptr, units_dat::ReturnToIdleOrder[unit->id]);
					}
					else
					{
						helper::prepend_order_targeting_nothing(unit, unit->orderQueueHead, OrderId::ResetCollision1);
					}
					if (unit->buildQueue[unit->buildQueueSlot] == UnitId::TerranReservoir)
					{
						builtUnit = helper::beginGasBuilding(unit, UnitId::TerranReservoir);
					}
					else
					{
						builtUnit = helper::create_unit(
							unit->buildQueue[unit->buildQueueSlot],
							unit->orderTarget.pt.x,
							unit->orderTarget.pt.y,
							unit->playerId);
					}
					u32 unit_id = unit->buildQueue[unit->buildQueueSlot];
					unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
					if (builtUnit == nullptr)
					{
						helper::refundUnitTrainCost(unit_id, unit->playerId);
						helper::display_last_network_error_for_player(unit->playerId);
						unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
					}
					else // 00468131
					{
						builtUnit->connectedUnit = unit;
						unit->mainOrderId = OrderId::ConstructingBuilding;
						unit->mainOrderState = 3;
						unit->orderTarget.unit = builtUnit;
						if (*IS_PLACING_BUILDING)
						{
							if (!helper::canStillPlaceBuilding())
							{
								helper::refresh_layers_3_and_4();
								helper::end_building_placement();
							}
						}
						scbw::refresh_console();
						builtUnit->order_computer(OrderId::BuildSelf1);
						helper::AI_TrainingOverlord(builtUnit, unit);
					}
				}
				else
				{
					unit->orderToIdle();
				}
			}
		}
		else if (unit->mainOrderState == 0) // 00468188
		{
			bool bMoveTargetResult;
			scbw::refresh_console();
			if (unit->orderTarget.unit != nullptr)
			{
				unit->orderTarget.pt.x = (unit->orderTarget.unit)->sprite->position.x;
				unit->orderTarget.pt.y = (unit->orderTarget.unit)->sprite->position.y;
			}
			bMoveTargetResult = helper::SetMoveTarget_xy(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
			if ((unit->orderTarget.pt.x != unit->nextTargetWaypoint.x ||
				unit->orderTarget.pt.y != unit->nextTargetWaypoint.y)
				&& !(unit->orderTarget.pt.x == 0 && unit->orderTarget.pt.y == 0))
			{
				unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
				unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
			}
			if (bMoveTargetResult)
			{
				unit->mainOrderState = 1;
			}
		}
	}

	char refreshScreen(s32 ecx0) // 0048DDC0
	{
		auto v6 = ecx0;
		auto result = *IS_PLACING_BUILDING;
		if (*IS_PLACING_BUILDING)
		{
			auto y = mouse->y;
			auto x = mouse->x;
			auto mouse_pos = Point16{(u16)mouse->x, (u16)mouse->y};
			if (helper::initPlacementBoxPos(&mouse_pos))
			{
				*placeBuildingMessageId = helper::updatePlacementStateForPlacementBox_Func();
				helper::refresh_layers_3_and_4();
				Point16 buildPoint;
				buildPoint.x = x;
				buildPoint.y = y;
				helper::unk_buildingPlacementRelated(1, &buildPoint);
				if (*isPlacingAddon)
				{
					auto v4 = *placeBuildingUnitId - UnitId::TerranCommandCenter;
					Point16 addon_point;
					addon_point.x = units_dat::AddonPlacement[v4].x;
					addon_point.y = units_dat::AddonPlacement[v4].y;
					scbw::adjustAddonCoordinates(*placeBuildingUnitId, 0, &addon_point.x, &addon_point.y);
					helper::unk_buildingPlacementRelated(0, &addon_point);
				}
				result = helper::refresh_layers_3_and_4();
			}
			else
			{
				helper::unk_buildingPlacementRelatedMouse(0, 1);
				result = helper::unk_buildingPlacementRelatedMouse(0, 0);
			}
		}
		return result;
	}
}