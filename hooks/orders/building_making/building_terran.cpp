#include "building_terran.h"
#include "building_modifier.h"
#include <SCBW/api.h>

#ifndef FALSE
#define FALSE 0
#endif
#include <globals.h>

#ifndef TRUE
#define TRUE 1
#endif

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
				current_image->playIscriptAnim(IscriptAnimation::SpecialState1);
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
				current_image->playIscriptAnim(IscriptAnimation::SpecialState2);
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
			helper::initBuilding(building, 0);
			Image* current_image = building->sprite->images.head;
			while (current_image != nullptr)
			{
				current_image->playIscriptAnim(IscriptAnimation::AlmostBuilt);
				current_image = current_image->link.next;
			}
			building->mainOrderState = 6;
		}
		else if (building->mainOrderState == 6)
		{
			if (building->hitPoints > 80 * units_dat::MaxHitPoints[building->id] / 100)
			{
				building->orderComputerClear(OrderId::Nothing2);
			}
		}
	}

	// Go to build on an existing building
	void orders_SCVBuild2(Unit* unit)
	{
		Unit* builtUnit = unit->orderTarget.unit;

		BuildingModifier modifier = get_building_modifier(builtUnit);

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
				if (helper::orderToMoveToTarget(unit, builtUnit))
					unit->mainOrderState = 2;
			}
			if (unit->mainOrderState == 2)
			{
				u32 movableState = unit->getMovableState();
				if (movableState == 2)
				{ // unmovable
					unit->orderToIdle();
				}
				else if (movableState == 1)
				{ // reached destination
					scbw::refreshConsole();
					if (builtUnit->connectedUnit != nullptr &&
						builtUnit->connectedUnit != unit &&
						(builtUnit->connectedUnit)->orderTarget.unit == builtUnit)
					{
						unit->orderToIdle(); // another SCV is already on the job
					}
					else
					{ // 00467AF9
						builtUnit->connectedUnit = unit;
						unit->status &= ~UnitStatus::IsNormal;	// remove UnitStatus::IsNormal if present
						unit->sprite->elevationLevel = units_dat::Elevation[unit->id] + 1;
						unit->orderSimple(OrderId::ResetCollision1, false);
						unit->orderSimple(units_dat::ReturnToIdleOrder[unit->id], false);
						if (helper::changeMovementTargetMoveTarget2(unit,
							builtUnit->sprite->position.x,
							builtUnit->sprite->position.y))
						{
							unit->mainOrderState = 3;
						}
					}
				}
			}
			else if (unit->mainOrderState == 3)
			{
				if (unit->getMovableState() != 0)
				{
					unit->mainOrderState = 4;
				}
			}
			else if (unit->mainOrderState == 4)
			{
				s16 ebp_18, ebp_16, ebp_14, ebp_12, x_min, y_min, x_max, y_max;
				s32 ebp_08;
				ebp_18 = builtUnit->sprite->position.x - builtUnit->sprite->width / 2;
				ebp_14 = builtUnit->sprite->position.x + builtUnit->sprite->width / 2 - 1;
				ebp_16 = builtUnit->sprite->position.y - builtUnit->sprite->height / 2;
				ebp_12 = builtUnit->sprite->position.y + builtUnit->sprite->height / 2 - 1;
				ebp_08 = units_dat::BuildingDimensions[builtUnit->id].x;
				if (ebp_08 < 0)
				{
					x_min = builtUnit->sprite->position.x - (ebp_08 - 0xFFFFFFFF) / 2;
				}
				else
				{
					x_min = builtUnit->sprite->position.x - ebp_08 / 2;
				}
				x_max = ebp_08 + x_min - 1;
				if (units_dat::BuildingDimensions[builtUnit->id].y < 0)
				{
					y_min = builtUnit->sprite->position.y - (units_dat::BuildingDimensions[builtUnit->id].y - 0xFFFFFFFF) / 2;
				}
				else
				{
					y_min = builtUnit->sprite->position.y - units_dat::BuildingDimensions[builtUnit->id].y / 2;
				}
				y_max = units_dat::BuildingDimensions[builtUnit->id].y + y_min - 1;
				x_min = x_min + units_dat::UnitBounds[unit->id].left + 1;
				x_max = x_max + (0xFFFF - units_dat::UnitBounds[unit->id].right);
				y_min = y_min + units_dat::UnitBounds[unit->id].top + 1;
				y_max = y_max + (0xFFFF - units_dat::UnitBounds[unit->id].bottom);

				if (x_min < ebp_18)
				{
					x_min = ebp_18;
				}
				if (y_min < ebp_16)
				{
					y_min = ebp_16;
				}
				if (x_max > ebp_14)
				{
					x_max = ebp_14;
				}
				if (y_max > ebp_12)
				{
					y_max = ebp_12;
				}
				unit->orderTarget.pt.x = helper::RandBetween(ebp_18, ebp_14, 9);
				unit->orderTarget.pt.y = helper::RandBetween(ebp_16, ebp_12, 9);
				helper::fixTargetLocation(&unit->orderTarget.pt, unit->id);
				if (helper::unk_terrainCollision2(
					unit,
					unit->orderTarget.pt.x - unit->sprite->position.x,
					unit->orderTarget.pt.y - unit->sprite->position.y) != 0)
				{
					unit->orderTarget.pt.x = helper::RandBetween(x_min, x_max, 9);
					unit->orderTarget.pt.y = helper::RandBetween(y_min, y_max, 9);
					helper::fixTargetLocation(&unit->orderTarget.pt, unit->id);
				}

				if (helper::changeMovementTargetMoveTarget2(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y) != 0)
				{
					unit->mainOrderState = 5;
					jump_to_switch_1 = true;
				}
			} // mainOrderState == 4
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
					helper::setNextWaypoint(unit);
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
					else
					{ // 00467E4A
						u8 rand_value = helper::RandomizeShort(10);
						Image* current_image = unit->sprite->images.head;
						unit->mainOrderTimer = (rand_value & 63) + 30;
						while (current_image != nullptr)
						{
							current_image->playIscriptAnim(IscriptAnimation::AlmostBuilt);
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
						current_image->playIscriptAnim(IscriptAnimation::AlmostBuilt);
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
						current_image->playIscriptAnim(IscriptAnimation::GndAttkToIdle);
						current_image = current_image->link.next;
					}
					unit->mainOrderState = 4;
					jump_to_switch_1 = true;
				}
			}

			if (jump_to_switch_1)
			{
				u32 hpGain = builtUnit->buildRepairHpGain * modifier.multiplier;
				UnitExt* unitExt = globals.GetUnitExt(builtUnit);
				if (*CHEAT_STATE & CheatFlags::OperationCwal)
				{
					hpGain *= 16;
				}
				helper::buildingAddon(builtUnit, hpGain, FALSE);
				if (modifier.period && builtUnit->remainingBuildTime % modifier.period == 0)
				{
					if (builtUnit->remainingBuildTime > modifier.decrease)
					{
						builtUnit->remainingBuildTime -= modifier.decrease;
					}
					else
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
						current_image->playIscriptAnim(IscriptAnimation::WalkingToIdle);
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
								|| builtUnit->id == UnitId::ZergExtractor)
							{
								if (builtUnit->playerId == unit->playerId)
								{
									jump_to_67F3C = true;
								}
							}
						}
						if (!jump_to_67F3C)
						{ // 00467F2C
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
					if (!bEndThere)
					{ // 00467F4C
						if (!(unit->orderQueueHead == nullptr))
						{
							unit->userActionFlags |= 1;
							prepareForNextOrder(unit);
						}
						else if (unit->pAI != nullptr)
						{ // 00467F6F												
							unit->orderComputerClear(OrderId::ComputerAI);
						}
						else
						{
							unit->orderComputerClear(units_dat::ReturnToIdleOrder[unit->id]);
						}
					}
				}
			}
		}
		else
		{ // 00467ED0
			Image* current_image = unit->sprite->images.head;
			bool bEndThere = false;
			while (current_image != nullptr)
			{
				current_image->playIscriptAnim(IscriptAnimation::WalkingToIdle);
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
						|| builtUnit->id == UnitId::ZergExtractor)
					{
						if (builtUnit->playerId == unit->playerId)
						{
							jump_to_67F3C = true;
						}
					}
				}
				if (!jump_to_67F3C)
				{ // 00467F2C
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
			if (!bEndThere)
			{ // 00467F4C
				if (!(unit->orderQueueHead == nullptr))
				{
					unit->userActionFlags |= 1;
					prepareForNextOrder(unit);
				}
				else if (unit->pAI != nullptr)
				{ // 00467F6F
					unit->orderComputerClear(OrderId::ComputerAI);
				}
				else
				{
					unit->orderComputerClear(units_dat::ReturnToIdleOrder[unit->id]);
				}
			}
		}
	}

	//// Go to build a not yet existing building
	void orders_SCVBuild(Unit* unit)
	{
		bool bEndThere = false;
		if (unit->pAI != nullptr)
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
					if (unit->orderQueueHead == nullptr)
					{ // 004680A5
						helper::addOrderTargetingNothing(unit, nullptr, OrderId::ResetCollision1);
						helper::addOrderTargetingNothing(unit, nullptr, units_dat::ReturnToIdleOrder[unit->id]);
					}
					else
					{
						helper::prependOrderTargetingNothing(unit, unit->orderQueueHead, OrderId::ResetCollision1);
					}
					if (unit->buildQueue[unit->buildQueueSlot] == UnitId::TerranReservoir)
					{
						builtUnit = helper::beginGasBuilding(unit, UnitId::TerranReservoir);
					}
					else
					{
						builtUnit = helper::createUnit(
							unit->buildQueue[unit->buildQueueSlot],
							unit->orderTarget.pt.x,
							unit->orderTarget.pt.y,
							unit->playerId);
					}
					unit->buildQueue[unit->buildQueueSlot] = UnitId::None;
					if (builtUnit == nullptr)
					{
						helper::displayLastNetErrorForPlayer(unit->playerId);
						unit->orderComputerClear(units_dat::ReturnToIdleOrder[unit->id]);
						bEndThere = true;
					}
					else
					{ // 00468131
						builtUnit->connectedUnit = unit;
						unit->mainOrderId = OrderId::ConstructingBuilding;
						unit->mainOrderState = 3;
						unit->orderTarget.unit = builtUnit;
						if (*IS_PLACING_BUILDING)
						{
							if (!helper::canStillPlaceBuilding())
							{
								helper::refreshLayers3And4();
								helper::endBuildingPlacement();
							}
						}
						scbw::refreshConsole();
						builtUnit->orderComputerClear(OrderId::BuildSelf1);
						helper::AI_TrainingOverlord(builtUnit, unit);
						bEndThere = true;
					}
				}
				if (!bEndThere)
				{
					unit->orderToIdle();
				}
			}
		}
		if (!bEndThere && unit->mainOrderState == 0)
		{ // 00468188
			bool bMoveTargetResult;
			scbw::refreshConsole();
			if (unit->orderTarget.unit != nullptr)
			{
				unit->orderTarget.pt.x = (unit->orderTarget.unit)->sprite->position.x;
				unit->orderTarget.pt.y = (unit->orderTarget.unit)->sprite->position.y;
			}
			if ((unit->orderTarget.pt.x != unit->nextTargetWaypoint.x ||
				unit->orderTarget.pt.y != unit->nextTargetWaypoint.y)
				&& !(unit->orderTarget.pt.x == 0 && unit->orderTarget.pt.y == 0))
			{
				bMoveTargetResult = helper::SetMoveTarget_xy(unit, unit->orderTarget.pt.x, unit->orderTarget.pt.y);
				unit->nextTargetWaypoint.x = unit->orderTarget.pt.x;
				unit->nextTargetWaypoint.y = unit->orderTarget.pt.y;
				if (bMoveTargetResult)
				{
					unit->mainOrderState = 1;
				}
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
				helper::refreshLayers3And4();
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
				result = helper::refreshLayers3And4();
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