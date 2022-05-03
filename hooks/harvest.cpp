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
		if (resource->id >= UnitId::TerranReservoir && resource->id <= UnitId::ResourceMineralFieldType3)
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
	}

	// Hooked as part of the vespene rework. -Pr0nogo
	bool unit_is_geyser_ex(Unit* resource_node, Unit* worker) // 00468930
	{
		if (resource_node->status & UnitStatus::Completed)
		{
			if (scbw::unit_id_is_gas_structure(resource_node->id) && resource_node->playerId == worker->playerId)
			{
				return true;
			}
		}
		return false;
	}

	// Hooked as part of the vespene rework. -Pr0nogo
	void order_enter_exit_gas(Unit* unit) // 00469980
	{
		Unit* unit_var = unit;
		bool sprite_flags_bool = (unit->sprite->flags & 0x20) == 0;
		Unit* target_node = unit->orderTarget.unit;
		Unit* target_node_copy = unit->orderTarget.unit;
		char resource_type = 0;
		if (!sprite_flags_bool
			|| target_node
			&& unit_is_geyser_ex(target_node, unit)
			&& ((resource_type = unit_var->resourceType) == 0 || resource_type & 3))
		{
			helper::set_gathering_flags(unit_var);
			char order_state = unit_var->mainOrderState;
			if (order_state)
			{
				if (order_state == 5 && !unit_var->mainOrderTimer)
				{
					s32 worker_direction = unit_var->currentDirection1;
					Point16 pos = {};
					if (helper::get_gas_exit_point(unit_var, target_node, &pos, &worker_direction))
					{
						helper::turnUnit(unit_var, worker_direction);
						helper::move_unit(unit_var, pos.x, pos.y);
						helper::show_and_enable_unit(unit_var);
						if (target_node_copy)
						{
							hooks::transferResourceToWorkerHook(target_node_copy, unit_var);
						}
						if (unit_var->worker.repairResourceLossTimer)
						{
							helper::finished_mining(unit_var, *&unit_var->worker.harvestTarget);
						}
						helper::delete_order(unit_var, 0x97u);
						if (!unit_var->orderQueueHead)
						{
							u8 resource_return_order = units_dat::ReturnToIdleOrder[unit_var->id];;
							if (unit_var->resourceType & 1)
							{
								resource_return_order = OrderId::ReturnGas;
							}
							Point16 dummy = {};
							helper::append_order_targeting_ground(resource_return_order, unit_var, dummy);
						}
						helper::prepend_order_targeting_nothing(unit_var, unit_var->orderQueueHead, OrderId::ResetCollision2);
						helper::actUnitReturnToIdle(unit_var);
					}
					else
					{
						unit_var->mainOrderTimer = 18;
					}
				}
			}
			else
			{
				helper::prepare_unit_move(unit_var, 1);
				unit_var->mainOrderState = 5;
				unit_var->mainOrderTimer = 37;
			}
		}
		else
		{
			helper::setResourceTarget(unit_var);
			s32 target_pos_x = unit_var->moveTarget.pt.x;
			s32 target_pos_y = unit_var->moveTarget.pt.y;
			if (target_pos_x != unit_var->nextTargetWaypoint.x || target_pos_y != unit_var->nextTargetWaypoint.y)
			{
				unit_var->nextTargetWaypoint.x = target_pos_x;
				unit_var->nextTargetWaypoint.y = target_pos_y;
			}
			helper::actUnitReturnToIdle(unit_var);
			if (unit_var->worker.repairResourceLossTimer)
			{
				helper::finished_mining(unit_var, *&unit_var->worker.harvestTarget);
			}
		}
	}
}