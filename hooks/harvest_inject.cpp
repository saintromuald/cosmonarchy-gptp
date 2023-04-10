#include "harvest.h"
#include "../hook_tools.h"
#include "SCBW/api.h"

namespace {
	void __declspec(naked) remove_powerup_image_overlay_wrapper() // 004F3900
	{
		static Unit* worker;
		__asm
		{
			MOV worker, EAX
			PUSHAD
		}
		hooks::remove_powerup_image_overlay(worker);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) transfer_resource_to_worker_wrapper() // 004696D0
	{
		static Unit* worker;
		static Unit* resource;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV worker, ECX
			MOV resource, EAX
			PUSHAD
		}
		hooks::transfer_resource_to_worker(worker, resource);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) unit_is_geyser_ex_wrapper() // 00468930
	{
		static Unit* resource_node;
		static Unit* worker;
		static bool result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV worker, EAX
			POP EAX
			MOV resource_node, ECX
			PUSHAD
		}
		result = hooks::unit_is_geyser_ex(resource_node, worker);
		__asm {
			POPAD
			MOV AL, result
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	void __declspec(naked) order_enter_exit_gas_wrapper() // 00469980
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::order_enter_exit_gas(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) can_harvest_gas_wrapper() { // 00469000
		static const u32 jmp_address = 0x004690B0;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::can_harvest_gas(unit);
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	void __declspec(naked) order_acquire_generic_right_click_harvest_proc_wrapper() { // 004557C0
		static u32 result;
		static Unit* node;
		static Unit* worker;
		__asm {
			MOV node, EDI
			MOV worker, ESI
			PUSHAD
		}
		result = hooks::order_acquire_generic_right_click_harvest_proc(node, worker);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) orders_harvest4_wrapper() { // 00468ED0
		static Unit* worker;
		__asm {
			MOV worker, EAX
			PUSHAD
		}
		hooks::orders_harvest4(worker);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) begin_harvest_wrapper() { // 00468C70
		static s32 result;
		static Unit* worker;
		static Unit* node;
		__asm {
			MOV node, EDI
			MOV worker, EAX
			PUSHAD
		}
		result = hooks::begin_harvest(worker, node);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) let_next_unit_mine_wrapper() { // 00468970
		static Unit* result;
		static Unit* worker;
		__asm {
			MOV worker, EBX
			PUSHAD
		}
		result = hooks::let_next_unit_mine(worker);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) orders_entries_move_to_harvest_obscured_wrapper() { // 00469500
		static const u32 jmp_address = 0x0046956D;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_entries_move_to_harvest_obscured(unit);
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	void __declspec(naked) remove_references_node_worker_wrapper() { // 00468EF0
		static Unit* node;
		static Unit* worker;
		static Unit* result; // unused
		__asm {
			MOV worker, ECX
			MOV node, EAX
			PUSHAD
		}
		result = hooks::remove_references_node_worker(node, worker);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
	
	void __declspec(naked) remove_from_current_harvesters_wrapper() { // 004686D0
		static Unit* result;
		static Unit* worker;
		static Unit* node;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV node, EAX
			POP EAX
			MOV worker, EAX
			PUSHAD
		}
		result = hooks::remove_from_current_harvesters(worker, node);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) remove_harvesters_wrapper() { // 00468770
		static Unit* node;
		__asm {
			MOV node, EDI
			PUSHAD
		}
		hooks::remove_harvesters(node);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) update_mineral_field_image_wrapper() { // 00468830
		static u16 unused;
		static Unit* unit;
		__asm {
			MOV unused, AX
			MOV unit, ECX
			PUSHAD
		}
		hooks::update_mineral_field_image(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) return_cargo_to_best_depot_proc_wrapper() { // 00440FA0
		static b32 result;
		static Unit* depot;
		static Unit* worker;
		__asm {
			MOV worker, EDX
			MOV depot, ECX
			PUSHAD
		}
		result = hooks::return_cargo_to_best_depot_proc(depot, worker);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) find_best_return_cargo_target_wrapper() { // 00441310
		static Unit* result;
		static Unit* worker;
		__asm {
			MOV worker, EAX
			PUSHAD
		}
		result = hooks::find_best_return_cargo_target(worker);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	b32 death_geyser_check(Unit* unit) {
		if (scbw::unit_id_is_gas_structure(unit->id)) {
			return true;
		}
		return false;
	}

	void __declspec(naked) death_geyser_check_wrapper() { // 0049F515
		static b32 result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = death_geyser_check(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	Unit* find_better_node(Unit* worker, Unit* current_node) {
		bool is_pathing = worker->pathing_flags & 1;
		u32 original_group_id = scbw::get_region(current_node->position)->group_index;
		Unit* best_node = current_node;
		s32 best_worker_count = get_unit_ext(current_node)->values[ValueActiveHarvesters];
		if (get_unit_ext(worker)->values[ValueIsActiveHarvester]) {
			best_worker_count -= 1; // we don't want to count the current worker
		}
		u32 best_distance = 0; // we don't want to change the current node on distance alone

		EntryArray nodes = {};
		unit_search_radial(&nodes, current_node->position, 8 * 32);
		for (u32 i = 0; i < nodes.count; ++i) {
			Unit* node = nodes.units[i];
			if (node->id < UnitId::ResourceMineralField || node->id > UnitId::ResourceMineralFieldType3) {
				continue;
			}
			if (is_pathing) {
				u32 group_id = scbw::get_region(node->position)->group_index;
				if (original_group_id != group_id) {
					continue;
				}
			}

			u32 worker_count = get_unit_ext(node)->values[ValueAssignedHarvesters];
			u32 distance = is_pathing ? helper::get_path_distance_to(worker, node->position) : scbw::get_distance_fast(worker->position, node->position);
			if (worker_count < best_worker_count || (worker_count == best_worker_count && distance < best_distance)) {
				best_node = node;
				best_worker_count = worker_count;
				best_distance = distance;
			}
		}

		return best_node;
	}

	void worker_wandering(Unit* worker, Unit* current_node) {
		if (!helper::move_to_target(current_node, worker)) {
			return;
		}
		UnitExt* current_node_ext = get_unit_ext(current_node);
		if (current_node_ext->values[ValueActiveHarvesters]) {
			Unit* new_node = find_better_node(worker, current_node);
			if (new_node && new_node != current_node) {
				current_node_ext->values[ValueAssignedHarvesters] -= 1;
				get_unit_ext(new_node)->values[ValueAssignedHarvesters] += 1;
				worker->order_target.unit = new_node;
				worker->order_target.pt.x = new_node->position.x;
				worker->order_target.pt.y = new_node->position.y;
				worker->worker.target_resource.unit = new_node;
				worker->worker.target_resource.pt.x = new_node->position.x;
				worker->worker.target_resource.pt.y = new_node->position.y;
				helper::move_to_target(new_node, worker);
				return;
			}
		}
		worker->main_order_state = 3;
	}

	void __declspec(naked) worker_wandering_wrapper() {
		static const u32 jump_address = 0x0046940F; 
		static Unit* worker;
		static Unit* resource;
		__asm {
			MOV worker, ESI
			MOV resource, EBX
			PUSHAD
		}
		worker_wandering(worker, resource);
		__asm {
			POPAD
			JMP jump_address
		}
	}
}

namespace hooks {
	void inject_harvest_resource() {
		jump_patch(remove_powerup_image_overlay_wrapper,					0x004F3900, 2);
		jump_patch(transfer_resource_to_worker_wrapper,						0x004696D0, 0);

		jump_patch(unit_is_geyser_ex_wrapper,								0x00468930, 5);
		jump_patch(order_enter_exit_gas_wrapper,							0x00469980, 1);
		jump_patch(can_harvest_gas_wrapper,									0x00469000, 1);
		jump_patch(order_acquire_generic_right_click_harvest_proc_wrapper,	0x004557C0, 2);
		jump_patch(orders_harvest4_wrapper,									0x00468ED0, 1);
		jump_patch(begin_harvest_wrapper,									0x00468C70, 4);
		jump_patch(let_next_unit_mine_wrapper,								0x00468970, 1);
		jump_patch(orders_entries_move_to_harvest_obscured_wrapper,			0x00469500, 4);

		jump_patch(remove_references_node_worker_wrapper,					0x00468EF0,	4);
		jump_patch(remove_from_current_harvesters_wrapper,					0x004686D0,	4);
		jump_patch(remove_harvesters_wrapper,								0x00468770,	1);
		jump_patch(update_mineral_field_image_wrapper,						0x00468830,	6);
		jump_patch(return_cargo_to_best_depot_proc_wrapper,					0x00440FA0,	1);
		jump_patch(find_best_return_cargo_target_wrapper,					0x00441310,	2);

		call_patch(death_geyser_check_wrapper,								0x0049F515, 0);
		jump_patch(worker_wandering_wrapper,								0x00469393,	1);
	}
}