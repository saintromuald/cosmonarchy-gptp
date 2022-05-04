#include "harvest.h"
#include "../hook_tools.h"

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

void __declspec(naked) AI_getVespeneType_Wrapper() { // 004326E0
	static u32 result;
	static Unit* unit;
	static u32 player_id;
	__asm {
		MOV unit, EAX
		MOV player_id, EDX
		PUSHAD
	}
	result = hooks::AI_getVespeneType(unit, player_id);
	__asm {
		POPAD
		MOV EAX, result
		RETN
	}
}

namespace hooks
{
	void injectHarvestResource()
	{
		jmpPatch(transferResourceToWorkerWrapper,	0x004696D0, 0);
		jmpPatch(unit_is_geyser_ex_Wrapper,			0x00468930, 5);
		jmpPatch(order_enter_exit_gas_Wrapper,		0x00469980, 1);
		jmpPatch(CanHarvestGas_Wrapper,				0x00469000, 0);
		jmpPatch(AI_getVespeneType_Wrapper,         0x004326E0, 2);
	}
}
