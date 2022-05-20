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
		if (unitId == UnitId::ZergExcisant)
		{
			return true;
		}
		return false;
	}

	void remove_powerup_image_overlay(Unit* worker) // 004F3900
	{
		worker->sprite->removeOverlay(ImageId::MineralChunkShadow, ImageId::PsiEmitterShadow_Carried);
		worker->sprite->removeOverlay(ImageId::Flag, ImageId::TerranGasTankType2);
		worker->sprite->removeOverlay(ImageId::Uraj, ImageId::Khalis);
		worker->sprite->removeOverlay(ImageId::GasRaw, ImageId::GasRaw2);
	}

	u8 harvestResourceFrom(Unit* resource, bool isMineral)
	{
		u8 return_value;
		if (resource->building.resource.resourceAmount < RESOURCE_HARVESTED)
		{
			return_value = std::max((s32)resource->building.resource.resourceAmount, 2);
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
					resource->building.resource.resourceAmount = 0;
				}
			}
			else if (resource->building.resource.resourceAmount < RESOURCE_HARVESTED
			&&		 isGasContainer(resource->id))
			{
				//scbw::showErrorMessageWithSfx(resource->playerId, 875, 20); // Gas depleted message and sound
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
		else if (resource->id == UnitId::ZergExcisant)
		{
			chunkImageId = ImageId::ZergGasSacType1;
		}
		else if (resource->id == UnitId::TerranReservoir)
		{
			chunkImageId = ImageId::TerranGasTankType1;
		}
		else if (resource->id == UnitId::ResourceVespeneGeyser)
		{
			chunkImageId = ImageId::GasRaw;
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
				chunkImageId += 1;	// Use depleted resource image
			}
			if (resourceAmount > 0)
			{
				if (worker->resourceType & ResourceType::Either)
				{
					helper::remove_powerup_image_overlay(worker);
					worker->resourceType = ResourceType::None;
				}

				// Resource return multipliers
				switch (scbw::getUpgradeLevel(worker->playerId, UpgradeId::MiningEfficiency))
				{
					case 0:
					case 99:
					{
					} break;
					case 1:
					{
						// 125% - Default AI cheat
						resourceAmount += (resourceAmount / 4);
					} break;
					case 2:
					{
						// 133%
						resourceAmount += (resourceAmount / 3);
					} break;
					case 3:
					{
						// 150%
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
	bool unit_is_geyser_ex(Unit* node, Unit* worker, bool ignore_completed) // 00468930
	{
		switch (node->id) {
			case UnitId::ResourceVespeneGeyser: {
				return true;
			} break;
			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExcisant: {
				if ((ignore_completed || node->status & UnitStatus::Completed) && node->playerId == worker->playerId) {
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
			&& (unit->resourceType == ResourceType::None || unit->resourceType & ResourceType::Either))
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
						if (node)
						{
							hooks::transferResourceToWorkerHook(unit, node);
						}
						if (unit->worker.resourceCarryAmount)
						{
							helper::finished_mining(unit, unit->worker.harvest_target);
						}
						helper::delete_order(unit, OrderId::HarvestInterrupt);
						helper::delete_order(unit, OrderId::ResetCollision2);
						u8 resource_return_order = unit->resourceType & ResourceType::VespeneGas ? OrderId::ReturnGas : units_dat::ReturnToIdleOrder[unit->id];
						if (!unit->orderQueueHead)
						{
							helper::order_targeting_nothing(resource_return_order, unit, 0);
						}
						else if (unit->orderQueueHead->orderId == OrderId::HarvestFinish)
						{
							// Fix HarvestFinish order freeze during Vespene Geyser mining
							// May cause issues if this is actually used for something important.
							// By the time units have HarvestFinish when harvesting from geysers, they no longer have a resource target. We could check for resource link instead...
							unit->orderQueueHead->orderId = resource_return_order;
						}
						helper::prepend_order_targeting_nothing(unit, unit->orderQueueHead, OrderId::ResetCollision2);
						helper::force_order_done(unit);
					}
					else
					{
						unit->mainOrderTimer = 18;
					}
				}
			}
			else
			{
				helper::turnUnit(unit, scbw::getAngle(unit->position, node->position));
				u32 timer = 30;
				if (node && node->id == UnitId::ResourceVespeneGeyser) {
					timer = 75;
					unit->playIscriptAnim(IscriptAnimation::AlmostBuilt);
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
			helper::force_order_done(unit);
			if (unit->worker.resourceCarryAmount)
			{
				helper::finished_mining(unit, unit->worker.harvest_target);
			}
		}
	}

	void CanHarvestGas(Unit* unit) { // 00469000
		Unit* node = unit->orderTarget.unit;

		bool valid_structure = false;
		if (node) {
			if (node->id == UnitId::ResourceVespeneGeyser) {
				valid_structure = true;
			} else if ((node->id == UnitId::TerranReservoir || node->id == UnitId::ProtossAquifer || node->id == UnitId::ZergExcisant)
				&& node->playerId == unit->playerId) {
				valid_structure = true;
			}
		}

		if (node
			&& node->status & UnitStatus::Completed
			&& valid_structure
			&& (unit->resourceType == ResourceType::None || unit->resourceType & ResourceType::Either))
		{
			helper::turnUnit(unit, scbw::getAngle(unit->position, node->position));
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
				helper::orders_nothing_2(unit);
			}
			else if (unit->unit_ai)
			{
				unit->orderComputer(OrderId::ComputerAI);
			}
			else
			{
				unit->orderComputer(units_dat::ReturnToIdleOrder[unit->id]);
			}
		}
	}

	// Hooked to investigate right-click error on geysers. -Pr0nogo
	u32 order_acquire_generic_right_click_harvest_proc(Unit* node, Unit* worker) // 004557C0
	{
		u16 node_id = node->id;
		if (units_dat::BaseProperty[node_id] & UnitStatus::CloakingForFree)
		{
			return OrderId::Move;
		}

		u32 result;
		char worker_resource_type;
		if (node_id < UnitId::ResourceMineralField || node_id > UnitId::ResourceMineralFieldType3)
		{
			if (unit_is_geyser_ex(node, worker, true) || node_id == UnitId::ResourceVespeneGeyser)
			{
				worker_resource_type = worker->resourceType;
				if (!worker_resource_type || worker_resource_type & ResourceType::Either)
				{
					result = OrderId::HarvestGas1;
				}
				else
				{
					result = OrderId::Move;
				}
			}
			else
			{
				if (!helper::unit_is_active_resource_depot(node) || node->playerId != worker->playerId)
				{
					result = OrderId::None;
					return result;
				}
				worker_resource_type = worker->resourceType;
				if (worker_resource_type & ResourceType::Minerals)
				{
					return OrderId::ReturnMinerals;
				}
				if (worker_resource_type & ResourceType::VespeneGas)
				{
					result = OrderId::ReturnGas;
				}
				else
				{
					result = OrderId::None;
				}
			}
		}
		else
		{
			worker_resource_type = worker->resourceType;
			if (!worker_resource_type || worker_resource_type & ResourceType::Either)
			{
				result = OrderId::MoveToMinerals;
			}
			else
			{
				result = OrderId::Move;
			}
		}
		return result;
	}

	// Hooked to resolve delay on first-time multi-worker vespene harvesting. -Pr0nogo
	void orders_harvest4(Unit* worker) // 00468ED0
	{
		if (worker->worker.harvest_target && worker->worker.harvest_target->id != UnitId::ResourceVespeneGeyser)
		{
			worker = helper::remove_from_current_harvesters(worker, worker->worker.harvest_target);
		}
		helper::force_order_done(worker);
	}

	// Hooked as part of the vespene overhaul. -Pr0nogo
	s32 begin_harvest(Unit* worker, Unit* node) // 00468CAA
	{
		if (node->building.resource.gatherQueueCount)
		{
			return 0;
		}
		node->building.resource.gatherQueueCount = 1;
		worker->worker.isCarryingSomething = 1;
		worker->worker.harvest_target = node;

		u8 harvest_order_id = OrderId::HarvestGas3;
		if (worker->mainOrderId != OrderId::CanEnterGasMine)
		{
			harvest_order_id = OrderId::MiningMinerals;
		}
		helper::order_immediate(worker, OrderId::HarvestInterrupt); // This needs to be called for harvesting gas too, since it's now interruptable
		helper::prepend_order_targeting_unit(worker, node, harvest_order_id);

		if (worker->orderQueueHead)
		{
			worker->userActionFlags |= 1u;
			helper::orders_nothing_2(worker);
		}
		else
		{
			if (worker->unit_ai)
			{
				helper::issue_order_targeting_nothing(worker, OrderId::ComputerAI);
			}
			else
			{
				helper::issue_order_targeting_nothing(worker, units_dat::ReturnToIdleOrder[worker->id]);
			}
		}
		return 1;
	}

	// Hooked to triage issues with the vespene rework. -Pr0nogo
	Unit* let_next_unit_mine(Unit* node) // 00468970
	{
		Unit* worker = node->building.resource.nextGatherer;
		if (!worker) {
			return 0;
		}

		u32 i = 0;
		Unit* final_worker = 0;
		while (worker) {
			u32 previous_status = worker->status;
			if (!(previous_status & UnitStatus::Frozen) // TODO: Replace this whole if with API is_disabled check
			&&  !worker->lockdownTimer
			&&  !worker->stasisTimer
			&&  !worker->maelstromTimer)
			{
				if (worker->mainOrderId == OrderId::CanEnterGasMine || worker->mainOrderId == OrderId::CanHarvestOre)
				{
					final_worker = worker;
				}
			}
			worker = worker->worker.harvest_link.next;
			if (++i > 100) {
				break; // This is a bandage fix for harvest_link recursing indefinitely  TODO: Fix the actual cause
			}
		}

		if (!final_worker)
		{
			return 0;
		}

		Unit* prev = final_worker->worker.harvest_link.prev;
		Unit* next = final_worker->worker.harvest_link.next;
		if (prev)
		{
			prev->worker.harvest_link.next = next;
		}
		else
		{
			node->building.resource.nextGatherer = final_worker->worker.harvest_link.next;
		}
		if (next)
		{
			next->worker.harvest_link.prev = prev;
		}
		final_worker->worker.harvest_link.prev = 0;
		final_worker->worker.harvest_link.next = 0;
		final_worker->worker.harvest_target = 0;
		helper::delete_order(final_worker, OrderId::HarvestFinish);
		return final_worker;
	}

	// Single-case hook of orders_entries to fix order delay on workers harvesting from vespene geysers. -Pr0nogo
	// Also used for OrderId::Harvest1
	void orders_entries_move_to_harvest_obscured(Unit* worker) { // 00469500
		if (worker->resourceType && !(worker->resourceType & ResourceType::Either)) {
			helper::set_resource_target(worker);
			u32 fixed_x = worker->moveTarget.pt.x;
			u32 fixed_y = worker->moveTarget.pt.y;
			if (fixed_x != worker->nextTargetWaypoint.x || fixed_y != worker->nextTargetWaypoint.y) {
				worker->nextTargetWaypoint.x = fixed_x;
				worker->nextTargetWaypoint.y = fixed_y;
			}
			if (worker->orderQueueHead) {
				worker->userActionFlags |= 1;
				helper::orders_nothing_2(worker);
			} else if (worker->unit_ai) {
				helper::issue_order_targeting_nothing(worker, OrderId::ComputerAI);
			} else {
				helper::issue_order_targeting_nothing(worker, units_dat::ReturnToIdleOrder[worker->id]);
			}
			return;
		}
		helper::set_gathering_flags(worker);
		Unit* target = worker->orderTarget.unit;
		if (worker->mainOrderState) {
			u32 is_movable = helper::is_unit_movable(worker);
			if (worker->mainOrderState == 3 && is_movable) {
				if (target && !(target->status & UnitStatus::Completed)) {
					return;
				}
				if (target && helper::unit_is_geyser_ex(target, worker)) {
					if (is_movable == 2) {
						worker->mainOrderState = 0;
					} else if (worker->resourceType & ResourceType::VespeneGas) {
						helper::issue_order_targeting_nothing(worker, OrderId::ReturnGas);
					} else {
						helper::prepend_order_targeting_unit(worker, target, OrderId::CanEnterGasMine);
						helper::to_idle(worker);
					}
					return;
				}
				helper::make_to_hold_position(worker);
				helper::to_idle(worker);
				return;
			}
		} else {
			if (!target) {
				helper::make_to_hold_position(worker);
				helper::to_idle(worker);
				return;
			}
			u16 node_id = target->id;
			if (node_id < UnitId::ResourceMineralField || node_id > UnitId::ResourceMineralFieldType3) {
				if (helper::move_to_target(target, worker)) {
					worker->worker.targetResource.pt = worker->orderTarget.pt;
					worker->worker.targetResource.unit = worker->orderTarget.unit;
					worker->mainOrderState = 3;
				}
			} else {
				helper::prepend_order(worker, OrderId::MoveToMinerals, worker->orderTarget.pt, worker->orderTarget.unit);
				helper::to_idle(worker);
			}
		}
	}
}