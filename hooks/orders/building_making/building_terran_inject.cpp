#include "building_terran.h"
#include <hook_tools.h>
#include "SCBW/api.h"
namespace
{
	void __declspec(naked) orders_terran_build_self_wrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_terran_build_self(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_scv_build_2_wrapper()
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_scv_build_2(unit);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_scv_build_wrapper()
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			PUSHAD
		}
		hooks::orders_scv_build(unit);
		__asm
		{
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) refresh_screen_wrapper() // 0048DDC0
	{
		static s32 ecx0;
		static char result;
		__asm
		{
			MOV ecx0, ECX
			PUSHAD
		}
		result = hooks::refresh_screen(ecx0);
		__asm
		{
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	const u32 updatePlacementWrapperJmp1 = 0x0048DD49;
	void __declspec(naked) update_placement_simple_wrapper()
	{
		static u16 addonOffsetY;
		__asm
		{
			MOVSX EAX, [ECX + 0x6626E2] // y
			MOV addonOffsetY, AX
			PUSHAD
		}
		scbw::adjust_addon_y(*placeBuildingUnitType, *placeBuildBaseType, &addonOffsetY);
		__asm
		{
			POPAD
			MOV AX, addonOffsetY
			JMP updatePlacementWrapperJmp1
		}
	}

	const u32 updatePlacementWrapperJmp2 = 0x0048DD68;
	void __declspec(naked) update_placement_simple_wrapper_2()
	{
		static u16 addonOffsetX;
		__asm
		{
			MOVSX EAX, [ECX + 0x6626E0] // x
			MOV addonOffsetX, AX
			PUSHAD
		}
		scbw::adjust_addon_x(*placeBuildingUnitType, *placeBuildBaseType, &addonOffsetX);
		__asm
		{
			POPAD
			MOV AX, addonOffsetX
			JMP updatePlacementWrapperJmp2
		}
	}

	void placementSimpleWrappers()
	{
		jump_patch(update_placement_simple_wrapper,		0x0048DD42, 2);
		jump_patch(update_placement_simple_wrapper_2,	0x0048DD61, 2);
	}
}

namespace hooks
{
	void injectBuildingTerranHook()
	{
		jump_patch(orders_terran_build_self_wrapper,			0x00467760, 2);
		jump_patch(orders_scv_build_2_wrapper,					0x00467A70, 1);
		jump_patch(orders_scv_build_wrapper,					0x00467FD0, 0);
//		placementSimpleWrappers(); // causes insane bug

//		jump_patch(refresh_screen_wrapper,					0x0048DDC0, 4);
	}
}
