#include "harvest.h"
#include "../hook_tools.h"

namespace {
	void __declspec(naked) remove_powerup_image_overlay_Wrapper() // 004F3900
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
	void __declspec(naked) transferResourceToWorkerWrapper() // 004696D0
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
		hooks::transferResourceToWorkerHook(worker, resource);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) unit_is_geyser_ex_Wrapper() // 00468930
	{
		static Unit* resource_node;
		static Unit* worker;
		static bool result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
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


	void __declspec(naked) order_enter_exit_gas_Wrapper() // 00469980
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

	void __declspec(naked) CanHarvestGas_Wrapper() { // 00469000
		static const u32 jmp_address = 0x004690B0;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::CanHarvestGas(unit);
		__asm {
			POPAD
			JMP jmp_address
		}
	}

	void __declspec(naked) order_acquire_generic_right_click_harvest_proc_Wrapper() { // 004557C0
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

	void __declspec(naked) orders_harvest4_Wrapper() { // 00468ED0
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

	void __declspec(naked) begin_harvest_Wrapper() { // 00468C70
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

	void __declspec(naked) let_next_unit_mine_Wrapper() { // 00468970
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

	void __declspec(naked) orders_entries_move_to_harvest_obscured_Wrapper() { // 00469500
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
			MOV EAX, [EBP + 0x8]
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
		}
	}
}

namespace hooks
{
	void injectHarvestResource()
	{
		jmpPatch(remove_powerup_image_overlay_Wrapper,						0x004F3900, 2);
		jmpPatch(transferResourceToWorkerWrapper,							0x004696D0, 0);
		jmpPatch(unit_is_geyser_ex_Wrapper,									0x00468930, 5);
		jmpPatch(order_enter_exit_gas_Wrapper,								0x00469980, 1);
		jmpPatch(CanHarvestGas_Wrapper,										0x00469000, 0);
		jmpPatch(order_acquire_generic_right_click_harvest_proc_Wrapper,	0x004557C0, 2);
		jmpPatch(orders_harvest4_Wrapper,									0x00468ED0, 1);
		jmpPatch(begin_harvest_Wrapper,										0x00468C70, 4);
		jmpPatch(let_next_unit_mine_Wrapper,								0x00468970, 1);
		jmpPatch(orders_entries_move_to_harvest_obscured_Wrapper,			0x00469500, 4);
		jmpPatch(remove_references_node_worker_wrapper,						0x00468EF0,	4);
		jmpPatch(remove_from_current_harvesters_wrapper,					0x004686D0,	4);
	}
}