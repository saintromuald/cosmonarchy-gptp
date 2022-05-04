#include "harvest.h"
#include "../SCBW/api.h"

namespace hooks
{
	const s32 RESOURCE_HARVESTED = 8;
	// Harvests minerals/gas from the @p resource and returns the amount that a worker should carry.
	// Identical to sub_469650
	bool isGasContainer(u16 unitId)
	{
		if (unitId == UnitId::ProtossAquifer)
		{
			return true;
		}
		if (unitId == UnitId::TerranReservoir)
		{
			return true;
		}
		if (unitId == UnitId::ZergExtractor)
		{
			return true;
		}
		return false;
	}

	u8 harvestResourceFrom(Unit* resource, bool isMineral)
	{
		u8 return_value;
		if (resource->building.resource.resourceAmount < RESOURCE_HARVESTED)
		{
			return_value = std::max((s32)resource->building.resource.resourceAmount, 1);
			resource->building.resource.resourceAmount = 0;
		}
		else
		{
			resource->building.resource.resourceAmount -= RESOURCE_HARVESTED;
			if (isMineral)
			{
				if (resource->building.resource.resourceAmount > 0)
				{
					helper::updateMineralPatchImage(resource);	// equivalent to setResourceCount @ 00468830 ?
				}
				else
				{
					resource->building.resource.resourceAmount = 2;
				}
			}
			else if (resource->building.resource.resourceAmount < RESOURCE_HARVESTED
					&& isGasContainer(resource->id))
			{
				scbw::showErrorMessageWithSfx(resource->playerId, 875, 20); // Gas depleted message and sound
			}
			return_value = RESOURCE_HARVESTED;
		}
		return return_value;
	}

	void transferResourceToWorkerHook(Unit* worker, Unit* resource) // 004696D0
	{
		u32 chunkImageId;
		bool isMineral = false;
		bool bEndThere = false;

		// TODO: Add depleted mineral chunk to worker
		if (resource->id >= UnitId::ResourceMineralField &&
			resource->id <= UnitId::ResourceMineralFieldType3)
		{
			chunkImageId = ImageId::MineralChunkType1;
			isMineral = true;
		}
		else if (resource->id == UnitId::ProtossAquifer)
		{
			chunkImageId = ImageId::ProtossGasOrbType1;
		}
		else if (resource->id == UnitId::ZergExtractor)
		{
			chunkImageId = ImageId::ZergGasSacType1;
		}
		else if (resource->id == UnitId::TerranReservoir)
		{
			chunkImageId = ImageId::TerranGasTankType1;
		}
		else if (resource->id == UnitId::ResourceVespeneGeyser)
		{
			chunkImageId = ImageId::ZergGasSacType1;
		}
		else
		{
			bEndThere = true;
		}

		if (!bEndThere)
		{
			u8 resourceAmount = harvestResourceFrom(resource, isMineral);
			if (resourceAmount < RESOURCE_HARVESTED)
			{
				chunkImageId += 1;	// Use depleted (smaller) chunk image
			}
			if (resourceAmount > 0)
			{
				if (worker->resourceType & 3)
				{ // Is carrying a mineral / gas
				// Identical to function @ 0x004F3900
				// Remove powerup shadows (mineral chunk shadow, psi emitter shadow, etc.)
					worker->sprite->removeOverlay(ImageId::MineralChunkShadow, ImageId::PsiEmitterShadow_Carried);
					worker->sprite->removeOverlay(ImageId::Flag, ImageId::TerranGasTankType2); // Remove powerup graphics
					worker->sprite->removeOverlay(ImageId::Uraj, ImageId::Khalis); // Remove Uraj / Khalis
					worker->resourceType = 0;
				}

				// AI cheat
				switch (scbw::getUpgradeLevel(worker->playerId, UpgradeId::MiningEfficiency))
				{
					case 0:
					case 99:
					{
					} break;
					case 1:
					{
						// 125% - Protoss
						resourceAmount += (resourceAmount / 4);
					} break;
					case 2:
					{
						// 133% - Terran
						resourceAmount += (resourceAmount / 3);
					} break;
					case 3:
					{
						// 150% - Zerg
						resourceAmount += (resourceAmount / 2);
					} break;
					case 4:
					{
						// 175%
						resourceAmount += ((resourceAmount / 4) + (resourceAmount / 4) + (resourceAmount / 4));
					} break;
					case 5:
					{
						// 200%
						resourceAmount *= 2;
					} break;
					case 6:
					{
						// 225%
						resourceAmount *= (2 + (1 / 4));
					} break;
					default:
					{
						// 250%
						resourceAmount *= (2 + (1 / 2));
					}
				}
				helper::setResourceAmountCarried(worker, resourceAmount, chunkImageId, isMineral);
			}
		}
	}

	// Hooked as part of the vespene rework. -Pr0nogo
	bool unit_is_geyser_ex(Unit* node, Unit* worker) // 00468930
	{
		switch (node->id) {
			case UnitId::ResourceVespeneGeyser: {
				if (!(node->sprite->flags & SpriteFlags::Hidden)) {
					return true;
				}
			} break;
			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExtractor: {
				if (node->status & UnitStatus::Completed && node->playerId == worker->playerId) {
					return true;
				}
			} break;
		}

		return false;
	}

	// Hooked as part of the vespene rework. -Pr0nogo
	void order_enter_exit_gas(Unit* unit) // 00469980
	{
		Unit* node = unit->orderTarget.unit;
		if (unit->sprite->flags & SpriteFlags::Hidden || node
			&& unit_is_geyser_ex(node, unit)
			&& (unit->resourceType == 0 || unit->resourceType & 3))
		{
			helper::set_gathering_flags(unit);
			if (unit->mainOrderState)
			{
				if (unit->mainOrderState == 5 && !unit->mainOrderTimer)
				{
					s32 worker_direction = unit->currentDirection1;
					Point16 pos = {};
					if (helper::get_gas_exit_point(unit, node, &pos, &worker_direction))
					{
						if (!node || node->id != UnitId::ResourceVespeneGeyser) {
							helper::turnUnit(unit, worker_direction);
							helper::move_unit(unit, pos.x, pos.y);
							helper::show_and_enable_unit(unit);
						}
						if (node)
						{
							hooks::transferResourceToWorkerHook(unit, node);
						}
						if (unit->worker.resourceCarryAmount)
						{
							helper::finished_mining(unit, unit->worker.harvestTarget);
						}
						helper::delete_order(unit, OrderId::ResetCollision2);
						u8 resource_return_order = unit->resourceType & 1 ? OrderId::ReturnGas : units_dat::ReturnToIdleOrder[unit->id];
						if (!unit->orderQueueHead)
						{
							helper::append_order_targeting_ground(resource_return_order, unit, {});
						}
						else if (unit->orderQueueHead->orderId == OrderId::Harvest4)
						{
							// Semi fixes a bug with simultenous vespene geyser mining freezing the unit
							unit->orderTo(resource_return_order);
						}
						helper::prepend_order_targeting_nothing(unit, unit->orderQueueHead, OrderId::ResetCollision2);
						helper::actUnitReturnToIdle(unit);
					}
					else
					{
						unit->mainOrderTimer = 18;
					}
				}
			}
			else
			{
				u32 timer = 37;
				if (node && node->id == UnitId::ResourceVespeneGeyser) {
					timer *= 1.5;
					unit->playIscriptAnim(IscriptAnimation::AlmostBuilt);
				} else {
					helper::hide_unit_partial(unit, true);
				}

				unit->mainOrderState = 5;
				unit->mainOrderTimer = timer;
			}
		}
		else
		{
			helper::setResourceTarget(unit);
			s32 target_pos_x = unit->moveTarget.pt.x;
			s32 target_pos_y = unit->moveTarget.pt.y;
			if (target_pos_x != unit->nextTargetWaypoint.x || target_pos_y != unit->nextTargetWaypoint.y)
			{
				unit->nextTargetWaypoint.x = target_pos_x;
				unit->nextTargetWaypoint.y = target_pos_y;
			}
			helper::actUnitReturnToIdle(unit);
			if (unit->worker.resourceCarryAmount)
			{
				helper::finished_mining(unit, unit->worker.harvestTarget);
			}
		}
	}

	void MoveToHarvestGas(Unit* unit) { // 00469000
		Unit* node = unit->orderTarget.unit;

		bool valid_structure = false;
		if (node) {
			if (node->id == UnitId::ResourceVespeneGeyser) {
				valid_structure = true;
			} else if ((node->id == UnitId::TerranReservoir || node->id == UnitId::ProtossAquifer || node->id == UnitId::ZergExtractor)
				&& node->playerId == unit->playerId) {
				valid_structure = true;
			}
		}

		if (node
			&& node->status & UnitStatus::Completed
			&& valid_structure
			&& (unit->resourceType == 0 || unit->resourceType & 3))
		{
			if (node->id == UnitId::ResourceVespeneGeyser && unit->mainOrderState == 2) {
				unit->mainOrderState = 0;
			}
			if (!unit->mainOrderState) {
				helper::Order_BeginHarvest(node, unit);
			}
		}
		else
		{
			helper::setResourceTarget(unit);
			u32 harvest_gas_x = unit->orderTarget.pt.x;
			u32 harvest_gas_y = unit->orderTarget.pt.y;
			if (harvest_gas_x != unit->nextTargetWaypoint.x || harvest_gas_y != unit->nextTargetWaypoint.y)
			{
				unit->nextTargetWaypoint.x = harvest_gas_x;
				unit->nextTargetWaypoint.y = harvest_gas_y;
			}
			if (unit->orderQueueHead)
			{
				unit->userActionFlags |= 1u;
				helper::orders_Nothing2(unit);
			}
			else if (unit->pAI)
			{
				unit->orderComputer(OrderId::ComputerAI);
			}
			else
			{
				unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
	}
}