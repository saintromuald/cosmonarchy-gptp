#include "load_unload_proc.h"
#include "SCBW\structures.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) load_unit_proc_wrapper() // 004E78E0
	{
		static Unit* unit;
		static Unit* unitToLoad;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			MOV unitToLoad, ECX
			PUSHAD
		}
		hooks::load_unit_proc(unit, unitToLoad);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) unload_unit_proc_wrapper() // 004E7F70
	{
		static Unit* unit;
		static u32 return_value;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		return_value = hooks::unload_unit_proc(unit);
		__asm {
			POPAD
			MOV EAX, return_value
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) update_unit_transport_data_wrapper() { // 00401EA0
		static char result;
		static Unit* cargo;
		static Unit* unused;
		__asm {
			MOV unused, EAX
			MOV cargo, ESI
			PUSHAD
		}
		result = hooks::update_unit_transport_data(cargo, unused);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) kill_all_loaded_units_wrapper() { // 0049FDD0
		static Unit* transport;
		__asm {
			MOV transport, EBX
			PUSHAD
		}
		hooks::kill_all_loaded_units(transport);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) kill_units_at_location_proc_wrapper() { // 004C7E20
		static bool result;
		static Unit* unit;
		static KillUnitArgs* args;
		__asm {
			MOV args, EDX
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::kill_units_at_location_proc(unit, args);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) get_loaded_unit_from_index_wrapper() { // 004E6C40
		static Unit* result;
		static Unit* transport;
		static u32 index;
		__asm {
			MOV index, ECX
			MOV transport, EAX
			PUSHAD
		}
		result = hooks::get_loaded_unit_from_index(transport, index);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) get_first_loaded_unit_wrapper() { // 004E6C90
		static Unit* result;
		static Unit* transport;
		__asm {
			MOV transport, EAX
			PUSHAD
		}
		result = hooks::get_first_loaded_unit(transport);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void injectLoadUnloadProcHooks()
	{
		jump_patch(load_unit_proc_wrapper,				0x004E78E0, 0);
		jump_patch(unload_unit_proc_wrapper,			0x004E7F70, 1);
	//	jump_patch(update_unit_transport_data_wrapper,	0x00401EA0, 1);
	//	jump_patch(kill_all_loaded_units_wrapper,		0x0049FDD0, 1);
	//	jump_patch(kill_units_at_location_proc_wrapper, 0x004C7E20, 1);
	//	jump_patch(get_loaded_unit_from_index_wrapper,	0x004E6C40, 3);
		jump_patch(get_first_loaded_unit_wrapper,		0x004E6C90, 0);
	}
}