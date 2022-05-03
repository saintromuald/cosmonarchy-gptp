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
		MOV result, AL
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

namespace hooks
{
	void injectHarvestResource()
	{
		jmpPatch(transferResourceToWorkerWrapper,	0x004696D0, 0);
		jmpPatch(unit_is_geyser_ex_Wrapper,			0x00468930, 5);
		jmpPatch(order_enter_exit_gas_Wrapper,		0x00469980, 1);
	}
}
