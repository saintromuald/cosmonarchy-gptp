#include "harvest.h"
#include "../SCBW/api.h"

#include "trigger_system/trigger_system.h"

// Used for AI calcs (currently supported values: 4, 3, 2, 1)
u32 get_gas_tier(Unit* unit) {
	if (unit) {
		switch (unit->id) {
			case UnitId::ResourceVespeneRidge:
				return 1;

			case UnitId::ResourceVespeneGeyser:
				return 2;

			case UnitId::TerranReservoir:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExcisant:
				return 3;

			case UnitId::TerranWellbore:
			case UnitId::ZergEzigrisant:
				return 4;
		}
	}
	return 0;
}

u32 get_harvest_amount(Unit* unit) {
	if (!unit) {
		return 0;
	}
	u32 result = 0;
	switch (unit->id) {
		case UnitId::ResourceMineralField:
		case UnitId::ResourceMineralFieldType2:
		case UnitId::ResourceMineralFieldType3: {
			return unit->building.resource.resource_amount > globals.get_yield(YieldType_MineralDepleted) ? globals.get_yield(YieldType_Mineral) : globals.get_yield(YieldType_MineralDepleted);
		} break;
		default: {
			switch (get_gas_tier(unit)) {
				case 1: return globals.get_yield(YieldType_GasRidge);
				case 2: return globals.get_yield(YieldType_GasGeyser);
				case 3: return globals.get_yield(YieldType_GasTier2);
				case 4: return globals.get_yield(YieldType_GasTier3);
			}
		} break;
	}
	return 0;
}

namespace hooks {
	bool is_gas_container(u16 unit_id) {
		if (unit_id_is_capped_gas_structure(unit_id)) {
			return true;
		}
		return false;
	}

	void remove_powerup_image_overlay(Unit* worker) { // 004F3900
		worker->sprite->remove_overlay(ImageId::MineralChunkShadow, ImageId::PsiEmitterShadow_Carried);
		worker->sprite->remove_overlay(ImageId::Flag, ImageId::TerranGasTankType2);
		worker->sprite->remove_overlay(ImageId::Uraj, ImageId::Khalis);
		worker->sprite->remove_overlay(ImageId::GasRaw, ImageId::GasRaw2);
		worker->sprite->remove_overlay(ImageId::Christmas_MineralChunk1, ImageId::Christmas_MineralChunk2);
		worker->sprite->remove_overlay(ImageId::Christmas_VespeneChunk1, ImageId::Christmas_VespeneChunk2);
	}

	u8 harvest_resource_from(Unit* resource, bool is_mineral) {
		u32 harvest_amount = get_harvest_amount(resource);
		if (is_mineral) {
			if (resource->building.resource.resource_amount < globals.get_yield(YieldType_Mineral)) {
				resource->building.resource.resource_amount = globals.get_yield(YieldType_MineralDepleted);
				resource->status |= UnitStatus::NoCollide;
			} else if (resource->building.resource.resource_amount -  harvest_amount > globals.get_yield(YieldType_MineralDepleted)) {
				resource->building.resource.resource_amount -= harvest_amount;
			} else {
				resource->building.resource.resource_amount = globals.get_yield(YieldType_MineralDepleted);
				resource->status |= UnitStatus::NoCollide; // Not sure if we should |= every time this function is called? -Pr0nogo
			}
			update_mineral_field_image(resource);
		}
		return harvest_amount;
	}

	void transfer_resource_to_worker(Unit* worker, Unit* resource) { // 004696D0
		u32 chunk_image_id;
		bool is_mineral = false;
		bool no_chunk_image = false;

		if (unit_id_is_minerals(resource->id)) {
			chunk_image_id = ImageId::MineralChunkType1;
			if (globals.celebration == Celebration_Christmas) {
				chunk_image_id = ImageId::Christmas_MineralChunk1;
			}
			is_mineral = true;
		} else if (resource->id == UnitId::ProtossAquifer) {
			chunk_image_id = ImageId::ProtossGasOrbType1;
		} else if (resource->id == UnitId::ZergExcisant || resource->id == UnitId::ZergEzigrisant) {
			chunk_image_id = ImageId::ZergGasSacType1;
		} else if (resource->id == UnitId::TerranReservoir || resource->id == UnitId::TerranWellbore) {
			chunk_image_id = ImageId::TerranGasTankType1;
		} else if (resource->id == UnitId::ResourceVespeneRidge || resource->id == UnitId::ResourceVespeneGeyser) {
			chunk_image_id = ImageId::GasRaw;
		} else {
			no_chunk_image = true;
		}

		if (!no_chunk_image) {
			if (globals.celebration == Celebration_Christmas) {
				if (!is_mineral) {
					chunk_image_id = ImageId::Christmas_VespeneChunk1;
				}
			}

			u8 resource_amount = harvest_resource_from(resource, is_mineral);
			if (resource_amount <= 2) {
				chunk_image_id += 1; // Use depleted resource image
			}
			if (resource_amount > 0) {
				if (worker->resource_type & ResourceType::Either) {
					helper::remove_powerup_image_overlay(worker);
					worker->resource_type = ResourceType::None;
				}

				// Resource return multipliers
				switch (bw::get_upgrade_level(worker->player, UpgradeId::MiningEfficiency)) {
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
				helper::set_resource_amount_carried(worker, resource_amount, chunk_image_id, is_mineral);
				if (worker->id == UnitId::ProtossArtisan) {
					UnitExt* unit_ext = get_unit_ext(worker);
					unit_ext->timers[TimerArtisanReturn] = 24 * 2 + 1;
					helper::play_sound_from_unit(SoundId::ArtisanRadiantChannel, worker, 1, 0);
					worker->freeze();
				}
			}
		}
	}

	// Hooked as part of the vespene rework. -Pr0nogo
	bool unit_is_geyser_ex(Unit* node, Unit* worker, bool ignore_completed) // 00468930
	{
		switch (node->id) {
			case UnitId::ResourceVespeneRidge:
			case UnitId::ResourceVespeneGeyser: {
				return true;
			} break;
			case UnitId::TerranReservoir:
			case UnitId::TerranWellbore:
			case UnitId::ProtossAquifer:
			case UnitId::ZergExcisant:
			case UnitId::ZergEzigrisant:
			{
				if ((ignore_completed || node->status & UnitStatus::Completed) && (node->player == worker->player || bw::are_mutual_allies(node->player, worker->player)))
				{
					return true;
				}
			} break;
		}
		return false;
	}

	// Hooked as part of the vespene rework. -Pr0nogo
	void order_enter_exit_gas(Unit* unit) { // 00469980
		Unit* node = unit->order_target.unit;
		if (node && !(node->status & UnitStatus::Completed) && !unit->unit_ai) {
			if (unit->sprite->main_graphic->animation == IscriptAnimation::AlmostBuilt) {
				unit->play_iscript_animation(IscriptAnimation::GndAttkToIdle);
			}
			return;
		}

		if (unit->sprite->flags & SpriteFlags::Hidden || node
		&&  unit_is_geyser_ex(node, unit)
		&& (unit->resource_type == ResourceType::None || unit->resource_type & ResourceType::Either))
		{
			helper::set_gathering_flags(unit);
			if (unit->main_order_state) {
				if (unit->main_order_state == 5 && !unit->main_order_timer) {
					s32 worker_direction = unit->current_direction_1;
					Point16 pos = {};
					if (helper::get_gas_exit_point(unit, node, &pos, &worker_direction)) {
						if (node) {
							hooks::transfer_resource_to_worker(unit, node);
						}
						if (unit->worker.is_harvesting) {
							helper::finished_mining(unit, unit->worker.harvest_target);
						}
						helper::delete_order(unit, OrderId::HarvestInterrupt);
						helper::delete_order(unit, OrderId::ResetCollision2);
						if (!unit->order_queue_head) {
							u8 resource_return_order = unit->resource_type & ResourceType::VespeneGas ? OrderId::ReturnGas : units_dat::return_to_idle_order[unit->id];
							helper::order_targeting_nothing(resource_return_order, unit, 0);
						}
						helper::prepend_order_targeting_nothing(unit, unit->order_queue_head, OrderId::ResetCollision2);
						helper::force_order_done(unit);
					} else {
						unit->main_order_timer = 18;
					}
				}
			} else {
				helper::turn_unit(unit, bw::get_angle(unit->position, node->position));
				unit->play_iscript_animation(IscriptAnimation::AlmostBuilt);
				unit->main_order_state = 5;
				unit->main_order_timer = HarvestTimer;
			}
		} else {
			helper::set_resource_target(unit);
			s32 target_pos_x = unit->move_target.pt.x;
			s32 target_pos_y = unit->move_target.pt.y;
			if (target_pos_x != unit->next_target_waypoint.x || target_pos_y != unit->next_target_waypoint.y) {
				unit->next_target_waypoint.x = target_pos_x;
				unit->next_target_waypoint.y = target_pos_y;
			}
			helper::force_order_done(unit);
			if (unit->worker.is_harvesting) {
				helper::finished_mining(unit, unit->worker.harvest_target);
			}
		}
	}

	void can_harvest_gas(Unit* unit) { // 00469000
		Unit* node = unit->order_target.unit;

		bool valid_structure = false;
		if (node) {
			if (node->id == UnitId::ResourceVespeneRidge || node->id == UnitId::ResourceVespeneGeyser) {
				valid_structure = true;
			} else if ((node->id == UnitId::TerranReservoir || node->id == UnitId::ProtossAquifer || node->id == UnitId::ZergExcisant
			||			node->id == UnitId::TerranWellbore	|| node->id == UnitId::ZergEzigrisant)
				&&		(node->player == unit->player || bw::are_mutual_allies(node->player, unit->player)))
			{
				valid_structure = true;
			} 
		}

		if (node
			&& node->status & UnitStatus::Completed
			&& valid_structure
			&& (unit->resource_type == ResourceType::None || unit->resource_type & ResourceType::Either)) {
			helper::turn_unit(unit, bw::get_angle(unit->position, node->position));
			if (!unit->main_order_state) {
				helper::Order_BeginHarvest(node, unit);
			}
		} else {
			helper::set_resource_target(unit);
			u32 harvest_gas_x = unit->order_target.pt.x;
			u32 harvest_gas_y = unit->order_target.pt.y;
			if (harvest_gas_x != unit->next_target_waypoint.x || harvest_gas_y != unit->next_target_waypoint.y) {
				unit->next_target_waypoint.x = harvest_gas_x;
				unit->next_target_waypoint.y = harvest_gas_y;
			}
			unit->order_to_idle();
		}
	}

	// Hooked to investigate right-click error on geysers. -Pr0nogo
	u32 order_acquire_generic_right_click_harvest_proc(Unit* node, Unit* worker) { // 004557C0
		u16 node_id = node->id;
		if (units_dat::base_property[node_id] & UnitStatus::CloakingForFree) {
			return OrderId::Move;
		}

		u32 result;
		u8 worker_resource_type;
		if (!unit_id_is_minerals(node_id)) {
			bool ignore_completed = true;
			if ((node->id == UnitId::TerranReservoir || node->id == UnitId::TerranWellbore)
			&& !(node->status & UnitStatus::Completed) && (!node->connected_unit || node->connected_unit->order_target.unit != node)) {
				ignore_completed = false;
			}

			if (unit_is_geyser_ex(node, worker, ignore_completed) || node_id == UnitId::ResourceVespeneRidge || node_id == UnitId::ResourceVespeneGeyser) {
				worker_resource_type = worker->resource_type;
				if (!worker_resource_type || worker_resource_type & ResourceType::Either) {
					result = OrderId::HarvestGas1;
				} else {
					result = OrderId::Move;
				}
			} else {
				if (!helper::unit_is_active_resource_depot(node)
				||   node->player != worker->player && (!bw::is_allied_to(node->player, worker->player) || !bw::is_allied_to(worker->player, node->player)))
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
		} else { // is not gas node
			worker_resource_type = worker->resource_type;
			if (!worker_resource_type || worker_resource_type & ResourceType::Either) {
				result = OrderId::MoveToMinerals;
			} else {
				result = OrderId::Move;
			}
		}
		return result;
	}

	// Hooked to resolve delay on initial multi-worker vespene harvesting. -Pr0nogo
	void orders_harvest4(Unit* worker) { // 00468ED0
		if (worker->worker.harvest_target) {
			worker = helper::remove_from_current_harvesters(worker, worker->worker.harvest_target);
		}
		helper::force_order_done(worker);
	}

	// Hooked as part of the vespene overhaul. -Pr0nogo
	s32 begin_harvest(Unit* worker, Unit* node) { // 00468CAA
		if (node->building.resource.is_mining) {
			return 0;
		}
		node->building.resource.is_mining = 1;
		worker->worker.is_harvesting = 1;
		worker->worker.harvest_target = node;

		u8 harvest_order_id = OrderId::MiningMinerals;
		if (worker->main_order_id == OrderId::CanEnterGasMine) {
			harvest_order_id = OrderId::HarvestGas3;
		}
		helper::order_immediate(worker, OrderId::HarvestInterrupt); // This needs to be called for harvesting gas too, since it's now interruptable
		helper::prepend_order_targeting_unit(worker, node, harvest_order_id);

		worker->order_to_idle();
		return 1;
	}

	// Hooked to triage issues with the vespene rework. -Pr0nogo
	Unit* let_next_unit_mine(Unit* node) { // 00468970
		Unit* worker = node->building.resource.next_gatherer;
		if (!worker) {
			return 0;
		}

		u32 i = 0;
		Unit* final_worker = 0;
		while (worker) {
			u32 previous_status = worker->status;
			if (!bw::unit_is_stunned(worker)) {
				if (worker->main_order_id == OrderId::CanEnterGasMine || worker->main_order_id == OrderId::CanHarvestOre) {
					final_worker = worker;
				}
			}
			worker = worker->worker.harvest_link.next;
		}

		if (!final_worker) {
			return 0;
		}

		Unit* prev = final_worker->worker.harvest_link.prev;
		Unit* next = final_worker->worker.harvest_link.next;
		if (prev) {
			prev->worker.harvest_link.next = next;
		} else {
			node->building.resource.next_gatherer = final_worker->worker.harvest_link.next;
		}
		if (next) {
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
			worker->order_to_idle();
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
			if (unit_id_is_minerals(node_id)) {
				helper::prepend_order(worker, OrderId::MoveToMinerals, worker->order_target.pt, worker->order_target.unit);
				helper::to_idle(worker);
			} else {
				if (helper::move_to_target(target, worker)) {
					worker->worker.target_resource.pt = worker->order_target.pt;
					worker->worker.target_resource.unit = worker->order_target.unit;
					worker->main_order_state = 3;
				}
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
				node->building.resource.is_mining = 0;
				Unit* next_harvester = helper::let_next_unit_mine(node);
				if (next_harvester)
				{
					return (Unit*)helper::begin_harvest(next_harvester, node);
				}
			}
		} else {
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

	// Hooked to investigate crashes. -Pr0nogo
	void remove_harvesters(Unit* node) { // 00468770
		node->building.resource.is_mining = 0;
		for (Unit* worker = node->building.resource.next_gatherer; worker; worker = worker->worker.harvest_link.next) {
			Unit* prev_worker = worker->worker.harvest_link.prev;
			if (prev_worker) {
				prev_worker->worker.harvest_link.next = prev_worker;
			} else {
				node->building.resource.next_gatherer = 0;
			}
			Unit* next_worker = worker->worker.harvest_link.next;
			if (next_worker) {
				next_worker->worker.harvest_link.prev = worker->worker.harvest_link.prev;
			}
			worker->worker.harvest_link.prev = 0;
			worker->worker.harvest_link.next = 0;
			worker->worker.harvest_target = 0;
		}
	}

	// Hooked to change the breakpoints for mineral images.
	void update_mineral_field_image(Unit* unit) { // 00468830
		IscriptAnimation::Enum anim = IscriptAnimation::Built;
		if (unit->building.resource.resource_amount < 4) {
			anim = IscriptAnimation::Landing;
		} else if (unit->building.resource.resource_amount < 250) {
			anim = IscriptAnimation::SpecialState1;
		} else if (unit->building.resource.resource_amount < 500) {
			anim = IscriptAnimation::SpecialState2;
		} else if (unit->building.resource.resource_amount < 750) {
			anim = IscriptAnimation::AlmostBuilt;
		}

		if (unit->building.resource.resource_iscript != anim) {
			unit->building.resource.resource_iscript = anim;
			unit->sprite->play_iscript_animation(anim);
		}
	}

	static Unit* find_better_node(Unit* worker, Unit* current_node) {
		bool is_pathing = worker->pathing_flags & 1;
		u32 original_group_id = bw::get_region(current_node->position)->group_index;
		u32 original_height = bw::get_active_tile_at_coords(current_node->position)->ground_height;

		Unit* townhall = find_best_return_cargo_target(worker);
		Unit* best_node = 0;
		s32 best_worker_count = get_unit_ext(current_node)->values[ValueActiveHarvesters];
		if (bw::harvester_is_active(worker)) {
			best_worker_count -= 1; // we don't want to count the current worker
		}
		u32 best_distance = 0; // we don't want to change the current node on distance alone

		EntryArray nodes = {};
		unit_search_radial(&nodes, current_node->position, 8 * 32);
		for (u32 i = 0; i < nodes.count; ++i) {
			Unit* node = nodes.units[i];
			if (node == current_node || !unit_id_is_minerals(node->id)) {
				continue;
			}
			if (townhall && bw::get_distance_square(node->position, townhall->position) > 10 * 32) {
				continue;
			}
			if (is_pathing) {
				u32 group_id = bw::get_region(node->position)->group_index;
				if (group_id != original_group_id) {
					continue;
				}
				u32 height = bw::get_active_tile_at_coords(node->position)->ground_height;
				if (height != original_height) {
					continue;
				}
			}

			u32 worker_count = get_unit_ext(node)->values[ValueAssignedHarvesters];
			u32 distance = bw::get_worker_harvest_distance(worker, node->position);
			if (worker_count < best_worker_count || (worker_count == best_worker_count && distance < best_distance)) {
				best_node = node;
				best_worker_count = worker_count;
				best_distance = distance;
			}
		}

		return best_node;
	}

	// Hooked to improve wandering behavior -Veeq7
	void worker_wandering(Unit* worker, Unit* current_node) { // 00469393 partial
		if (!helper::move_to_target(current_node, worker)) {
			return;
		}
		UnitExt* current_node_ext = get_unit_ext(current_node);
		if (current_node_ext->values[ValueActiveHarvesters] && !worker->worker.is_harvesting) {
			Unit* better_node = find_better_node(worker, current_node);
			if (better_node) {
				current_node_ext->values[ValueAssignedHarvesters] -= 1;
				UnitExt* worker_ext = get_unit_ext(worker);
				if (bw::harvester_is_active(worker)) {
					current_node_ext->values[ValueActiveHarvesters] -= 1; // we don't want to count the current worker
				}
				worker_ext->values[ValueHarvesterActivity] = 0;
				get_unit_ext(better_node)->values[ValueAssignedHarvesters] += 1;

				Target new_target = make_target(better_node);
				worker->order_target = new_target;
				worker->worker.target_resource = new_target;
				helper::move_to_target(better_node, worker);
				// NOTE: Fixes an edge case where the unit would not move to a new node correctly if it's already in range,
				//       since move_to_target wouldn't update the move_target in such case
				worker->move_target = new_target;
				return;
			}
		}
		worker->main_order_state = 3;
	}

	// Hooked to enable returns to allied town centers. -Pr0nogo
	b32 return_cargo_to_best_depot_proc(Unit* depot, Unit* worker) { // 00440FA0
		if (helper::unit_is_active_resource_depot(depot)) {
			if (depot->player == worker->player || bw::are_mutual_allies(depot->player, worker->player)) {
				if (!(worker->pathing_flags & PathingFlags_ProvidesCollision) || helper::unit_has_path_to_destination(worker, depot->position.x, depot->position.y)) {
					return true;
				}
			}
		}
		return false;
	}

	// Hooked to make workers always return to the closest town center by pathing distance. -Pr0nogo
	Unit* find_best_return_cargo_target(Unit* worker) { // 00441310
		if (worker->id == UnitId::ProtossArtisan) {
			return 0;
		}

		Unit* best_depot = 0;
		u32 best_distance = UINT_MAX;
		for (Unit* depot = *g_first_visible_unit; depot; depot = depot->link.next) {
			if (!return_cargo_to_best_depot_proc(depot, worker)) {
				continue;
			}

			u32 distance = bw::get_worker_harvest_distance(worker, depot->position);
			if (distance < best_distance) {
				best_depot = depot;
				best_distance = distance;
			}
		}
		return best_depot;
	}

	// Hooked to investigate depleted mineral messages. Not currently called from anywhere. -Pr0nogo
	u8 mine_resources(Unit* node, bool is_minerals) // 00469650
	{
		u8 resources_returned = get_harvest_amount(node);
		u32 node_resources = node->building.resource.resource_amount;
		if (is_minerals)
		{
			if (node_resources <= 4) {
				node->building.resource.resource_amount = 2;
			} else {
				node->building.resource.resource_amount -= resources_returned;
			}
			update_mineral_field_image(node);
			/* // The code below handles messaging for depleted vespene nodes, which is not currently in spec. -Pr0nogo
				if (node_resources_final < 8)
				{
					helper::display_error_message_id_with_sfx(node->player, 875, SoundId::Misc_OutOfGas_wav);
				}
				resources_returned = 8;
			*/
		}
		return resources_returned;
	}
}