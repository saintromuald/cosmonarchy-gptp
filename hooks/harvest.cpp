#include "harvest.h"
#include "../SCBW/api.h"

namespace hooks
{
	const s32 RESOURCE_HARVESTED = 8;
	// Harvests minerals/gas from the @p resource and returns the amount that a worker should carry.
	// Identical to sub_469650
	bool isGasContainer(u16 unit_id)
	{
		if (unit_id == UnitId::ProtossAquifer)
		{
			return true;
		}
		if (unit_id == UnitId::TerranReservoir)
		{
			return true;
		}
		if (unit_id == UnitId::ZergExcisant)
		{
			return true;
		}
		return false;
	}

	void remove_powerup_image_overlay(Unit* worker) // 004F3900
	{
		worker->sprite->remove_overlay(ImageId::MineralChunkShadow, ImageId::PsiEmitterShadow_Carried);
		worker->sprite->remove_overlay(ImageId::Flag, ImageId::TerranGasTankType2);
		worker->sprite->remove_overlay(ImageId::Uraj, ImageId::Khalis);
		worker->sprite->remove_overlay(ImageId::GasRaw, ImageId::GasRaw2);
	}

	u8 harvestResourceFrom(Unit* resource, bool isMineral)
	{
		u8 return_value;
		if (resource->building.resource.resource_amount < RESOURCE_HARVESTED)
		{
			return_value = std::max((s32)resource->building.resource.resource_amount, 2);
			resource->building.resource.resource_amount = 0;
		}
		else
		{
			resource->building.resource.resource_amount -= RESOURCE_HARVESTED;
			if (isMineral)
			{
				if (resource->building.resource.resource_amount > 0)
				{
					helper::updateMineralPatchImage(resource);	// equivalent to setResourceCount @ 00468830 ?
				}
				else
				{
					resource->building.resource.resource_amount = 0;
				}
			}
			else if (resource->building.resource.resource_amount < RESOURCE_HARVESTED
			&&		 isGasContainer(resource->id))
			{
				//scbw::show_error_message_with_sfx(resource->player, 875, 20); // Gas depleted message and sound
			}
			return_value = RESOURCE_HARVESTED;
		}
		return return_value;
	}

	void transfer_resource_to_worker(Unit* worker, Unit* resource) // 004696D0
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
			u8 resource_amount = harvestResourceFrom(resource, isMineral);
			if (resource_amount < RESOURCE_HARVESTED)
			{
				chunkImageId += 1;	// Use depleted resource image
			}
			if (resource_amount > 0)
			{
				if (worker->resource_type & ResourceType::Either)
				{
					helper::remove_powerup_image_overlay(worker);
					worker->resource_type = ResourceType::None;
				}

				// Resource return multipliers
				switch (scbw::get_upgrade_level(worker->player, UpgradeId::MiningEfficiency))
				{
					case 0:
					case 99:
					{
					} break;
					case 1:
					{
						// 125% - Default AI cheat
						resource_amount += (resource_amount / 4);
					} break;
					case 2:
					{
						// 133%
						resource_amount += (resource_amount / 3);
					} break;
					case 3:
					{
						// 150%
						resource_amount += (resource_amount / 2);
					} break;
					case 4:
					{
						// 175%
						resource_amount += ((resource_amount / 4) + (resource_amount / 4) + (resource_amount / 4));
					} break;
					case 5:
					{
						// 200%
						resource_amount *= 2;
					} break;
					case 6:
					{
						// 225%
						resource_amount *= (2 + (1 / 4));
					} break;
					default:
					{
						// 250%
						resource_amount *= (2 + (1 / 2));
					}
				}
				helper::set_resource_amount_carried(worker, resource_amount, chunkImageId, isMineral);
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
				if ((ignore_completed || node->status & UnitStatus::Completed) && node->player == worker->player) {
					return true;
				}
			} break;
		}
		return false;
	}

	// Hooked as part of the vespene rework. -Pr0nogo
	void order_enter_exit_gas(Unit* unit) // 00469980
	{
		Unit* node = unit->order_target.unit;
		if (node && !(node->status & UnitStatus::Completed) && !unit->unit_ai) {
			if (unit->sprite->main_graphic->animation == IscriptAnimation::AlmostBuilt) {
				unit->play_iscript_animation(IscriptAnimation::GndAttkToIdle);
			}
			return;
		}

		if (unit->sprite->flags & SpriteFlags::Hidden || node
			&& unit_is_geyser_ex(node, unit)
			&& (unit->resource_type == ResourceType::None || unit->resource_type & ResourceType::Either))
		{
			helper::set_gathering_flags(unit);
			if (unit->main_order_state)
			{
				if (unit->main_order_state == 5 && !unit->main_order_timer)
				{
					s32 worker_direction = unit->current_direction_1;
					Point16 pos = {};
					if (helper::get_gas_exit_point(unit, node, &pos, &worker_direction))
					{
						if (node)
						{
							hooks::transfer_resource_to_worker(unit, node);
						}
						if (unit->worker.is_harvesting)
						{
							helper::finished_mining(unit, unit->worker.harvest_target);
						}
						helper::delete_order(unit, OrderId::HarvestInterrupt);
						helper::delete_order(unit, OrderId::ResetCollision2);
						u8 resource_return_order = unit->resource_type & ResourceType::VespeneGas ? OrderId::ReturnGas : units_dat::ReturnToIdleOrder[unit->id];
						if (!unit->order_queue_head)
						{
							helper::order_targeting_nothing(resource_return_order, unit, 0);
						}
						helper::prepend_order_targeting_nothing(unit, unit->order_queue_head, OrderId::ResetCollision2);
						helper::force_order_done(unit);
					}
					else
					{
						unit->main_order_timer = 18;
					}
				}
			}
			else
			{
				helper::turnUnit(unit, scbw::get_angle(unit->position, node->position));
				u32 timer = 30;
				if (node && node->id == UnitId::ResourceVespeneGeyser) {
					timer = 75;
					unit->play_iscript_animation(IscriptAnimation::AlmostBuilt);
				}

				unit->main_order_state = 5;
				unit->main_order_timer = timer;
			}
		}
		else
		{
			helper::set_resource_target(unit);
			s32 target_pos_x = unit->move_target.pt.x;
			s32 target_pos_y = unit->move_target.pt.y;
			if (target_pos_x != unit->next_target_waypoint.x || target_pos_y != unit->next_target_waypoint.y)
			{
				unit->next_target_waypoint.x = target_pos_x;
				unit->next_target_waypoint.y = target_pos_y;
			}
			helper::force_order_done(unit);
			if (unit->worker.is_harvesting)
			{
				helper::finished_mining(unit, unit->worker.harvest_target);
			}
		}
	}

	void can_harvest_gas(Unit* unit) { // 00469000
		Unit* node = unit->order_target.unit;

		bool valid_structure = false;
		if (node) {
			if (node->id == UnitId::ResourceVespeneGeyser) {
				valid_structure = true;
			} else if ((node->id == UnitId::TerranReservoir || node->id == UnitId::ProtossAquifer || node->id == UnitId::ZergExcisant)
				&& node->player == unit->player) {
				valid_structure = true;
			}
		}

		if (node
			&& node->status & UnitStatus::Completed
			&& valid_structure
			&& (unit->resource_type == ResourceType::None || unit->resource_type & ResourceType::Either))
		{
			helper::turnUnit(unit, scbw::get_angle(unit->position, node->position));
			if (!unit->main_order_state) {
				helper::Order_BeginHarvest(node, unit);
			}
		}
		else
		{
			helper::set_resource_target(unit);
			u32 harvest_gas_x = unit->order_target.pt.x;
			u32 harvest_gas_y = unit->order_target.pt.y;
			if (harvest_gas_x != unit->next_target_waypoint.x || harvest_gas_y != unit->next_target_waypoint.y)
			{
				unit->next_target_waypoint.x = harvest_gas_x;
				unit->next_target_waypoint.y = harvest_gas_y;
			}
			if (unit->order_queue_head)
			{
				unit->user_action_flags |= 1u;
				helper::orders_nothing_2(unit);
			}
			else if (unit->unit_ai)
			{
				unit->order_computer(OrderId::ComputerAI);
			}
			else
			{
				unit->order_computer(units_dat::ReturnToIdleOrder[unit->id]);
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
			bool ignore_completed = true;
			if (node->id == UnitId::TerranReservoir && !(node->status & UnitStatus::Completed) && (!node->connected_unit || node->connected_unit->order_target.unit != node)) {
				ignore_completed = false;
			}

			if (unit_is_geyser_ex(node, worker, ignore_completed) || node_id == UnitId::ResourceVespeneGeyser)
			{
				worker_resource_type = worker->resource_type;
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
				if (!helper::unit_is_active_resource_depot(node) || node->player != worker->player)
				{
					result = OrderId::None;
					return result;
				}
				worker_resource_type = worker->resource_type;
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
			worker_resource_type = worker->resource_type;
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
		if (node->building.resource.gather_queue_count)
		{
			return 0;
		}
		node->building.resource.gather_queue_count = 1;
		worker->worker.is_harvesting = 1;
		worker->worker.harvest_target = node;

		u8 harvest_order_id = OrderId::HarvestGas3;
		if (worker->main_order_id != OrderId::CanEnterGasMine)
		{
			harvest_order_id = OrderId::MiningMinerals;
		}
		helper::order_immediate(worker, OrderId::HarvestInterrupt); // This needs to be called for harvesting gas too, since it's now interruptable
		helper::prepend_order_targeting_unit(worker, node, harvest_order_id);

		if (worker->order_queue_head)
		{
			worker->user_action_flags |= 1u;
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
		Unit* worker = node->building.resource.next_gatherer;
		if (!worker) {
			return 0;
		}

		u32 i = 0;
		Unit* final_worker = 0;
		while (worker) {
			u32 previous_status = worker->status;
			if (!(previous_status & UnitStatus::Frozen) // TODO: Replace this whole if with API is_disabled check
			&&  !worker->lockdown_timer
			&&  !worker->stasis_timer
			&&  !worker->maelstrom_timer)
			{
				if (worker->main_order_id == OrderId::CanEnterGasMine || worker->main_order_id == OrderId::CanHarvestOre)
				{
					final_worker = worker;
				}
			}
			worker = worker->worker.harvest_link.next;
			if (++i > 100) {
				break; // This is a bandage fix for harvest_link recursing indefinitely. TODO: Fix the actual cause.
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
			node->building.resource.next_gatherer = final_worker->worker.harvest_link.next;
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
		if (worker->resource_type && !(worker->resource_type & ResourceType::Either)) {
			helper::set_resource_target(worker);
			u32 fixed_x = worker->move_target.pt.x;
			u32 fixed_y = worker->move_target.pt.y;
			if (fixed_x != worker->next_target_waypoint.x || fixed_y != worker->next_target_waypoint.y) {
				worker->next_target_waypoint.x = fixed_x;
				worker->next_target_waypoint.y = fixed_y;
			}
			if (worker->order_queue_head) {
				worker->user_action_flags |= 1;
				helper::orders_nothing_2(worker);
			} else if (worker->unit_ai) {
				helper::issue_order_targeting_nothing(worker, OrderId::ComputerAI);
			} else {
				helper::issue_order_targeting_nothing(worker, units_dat::ReturnToIdleOrder[worker->id]);
			}
			return;
		}
		helper::set_gathering_flags(worker);
		Unit* target = worker->order_target.unit;
		if (worker->main_order_state) {
			u32 is_movable = helper::is_unit_movable(worker);
			if (worker->main_order_state == 3 && is_movable) {
				if (target && !(target->status & UnitStatus::Completed) && !worker->unit_ai) {
					if (worker->sprite->main_graphic->animation == IscriptAnimation::AlmostBuilt) {
						worker->play_iscript_animation(IscriptAnimation::GndAttkToIdle);
					}
					return;
				}
				if (target && helper::unit_is_geyser_ex(target, worker)) {
					if (is_movable == 2) {
						worker->main_order_state = 0;
					} else if (worker->resource_type & ResourceType::VespeneGas) {
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
					worker->worker.target_resource.pt = worker->order_target.pt;
					worker->worker.target_resource.unit = worker->order_target.unit;
					worker->main_order_state = 3;
				}
			} else {
				helper::prepend_order(worker, OrderId::MoveToMinerals, worker->order_target.pt, worker->order_target.unit);
				helper::to_idle(worker);
			}
		}
	}

	// Hooked to investigate obscure linked list crash. -Pr0nogo
	Unit* remove_references_node_worker(Unit* node, Unit* worker) // 00468EF0
	{
		Unit* result = 0;

		if (worker->worker.is_harvesting)
		{
			worker->worker.is_harvesting = 0;
			worker->worker.harvest_target = 0;
			if (node)
			{
				--node->building.resource.gather_queue_count;
				Unit* next_harvester = helper::let_next_unit_mine(node);
				if (next_harvester)
				{
					return (Unit*)helper::begin_harvest(next_harvester, node);
				}
			}
		}
		else
		{
			Unit* next_gatherer = node->building.resource.next_gatherer;
			if (next_gatherer)
			{
				while (next_gatherer != worker)
				{
					next_gatherer = next_gatherer->worker.harvest_link.next;
					if (!next_gatherer)
					{
						return 0;
					}
				}
				result = helper::remove_from_current_harvesters(next_gatherer, node);
			}
		}
		return result;
	}

	// Hooked to investigate obscure linked list crash. -Veeq7
	Unit* remove_from_current_harvesters(Unit* worker, Unit* node) { // 004686D0
		Unit* prev = worker->worker.harvest_link.prev;
		Unit* next = worker->worker.harvest_link.next;
		if (prev) {
			prev->worker.harvest_link.next = next;
		} else {
			node->building.resource.next_gatherer = next;
		}
		if (next) {
			next->worker.harvest_link.prev = prev;
		}
		worker->worker.harvest_target = 0;
		worker->worker.harvest_link.next = 0;
		worker->worker.harvest_link.prev = 0;
		return worker;
	}
}