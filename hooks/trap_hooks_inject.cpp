#include "trap_hooks.h"
#include "hook_tools.h"
#include "BW/api.h"

namespace
{
	void __declspec(naked) unit_is_floor_trap_wrapper() // 0047B070
	{
		static Unit* unit;
		static u32 result;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_is_floor_trap(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unit_is_installation_asset_wrapper() // 00401430
	{
		static u32 id;
		static u32 result;
		__asm
		{
			MOV id, EAX
			PUSHAD
		}
		result = hooks::unit_is_installation_asset(id);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) disable_trap_wrapper()
	{
		static Unit* unit;
		static u32 result;
		__asm
		{
			MOV unit, EDI
			PUSHAD
		}
		result = hooks::disable_trap(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_trap_hooks()
	{
		jump_patch(unit_is_floor_trap_wrapper,			0x0047B070, 3);
		jump_patch(unit_is_installation_asset_wrapper,	0x00401430, 3);
	}
}