#include "building_terran.h"
#include <hook_tools.h>
#include "SCBW/api.h"
namespace
{
	void __declspec(naked) orders_TerranBuildSelfWrapper()
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_TerranBuildSelf(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_SCVBuild2Wrapper()
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_SCVBuild2(unit);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) orders_SCVBuildWrapper()
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
		hooks::orders_SCVBuild(unit);
		__asm
		{
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) refreshScreen_Wrapper() // 0048DDC0
	{
		static s32 ecx0;
		static char result;
		__asm
		{
			MOV ecx0, ECX
			PUSHAD
		}
		result = hooks::refreshScreen(ecx0);
		__asm
		{
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	const u32 updatePlacementWrapperJmp1 = 0x0048DD49;
	void __declspec(naked) updatePlacementSimpleWrapper()
	{
		static u16 addonOffsetY;
		__asm
		{
			MOVSX EAX, [ECX + 0x6626E2] // y
			MOV addonOffsetY, AX
			PUSHAD
		}
		scbw::adjustAddonY(*placeBuildingUnitType, *placeBuildBaseType, &addonOffsetY);
		__asm
		{
			POPAD
			MOV AX, addonOffsetY
			JMP updatePlacementWrapperJmp1
		}
	}

	const u32 updatePlacementWrapperJmp2 = 0x0048DD68;
	void __declspec(naked) updatePlacementSimpleWrapper2()
	{
		static u16 addonOffsetX;
		__asm
		{
			MOVSX EAX, [ECX + 0x6626E0] // x
			MOV addonOffsetX, AX
			PUSHAD
		}
		scbw::adjustAddonX(*placeBuildingUnitType, *placeBuildBaseType, &addonOffsetX);
		__asm
		{
			POPAD
			MOV AX, addonOffsetX
			JMP updatePlacementWrapperJmp2
		}
	}

	void placementSimpleWrappers()
	{
		jump_patch(updatePlacementSimpleWrapper, 0x0048DD42, 2);
		jump_patch(updatePlacementSimpleWrapper2, 0x0048DD61, 2);
	}
}

namespace hooks
{
	void injectBuildingTerranHook()
	{
		jump_patch(orders_TerranBuildSelfWrapper,				0x00467760, 2);
		jump_patch(orders_SCVBuild2Wrapper,					0x00467A70, 1);
		jump_patch(orders_SCVBuildWrapper,					0x00467FD0, 0);
//		placementSimpleWrappers();							// causes insane bug

//		jump_patch(refreshScreen_Wrapper,						0x0048DDC0, 4);
	}
}
