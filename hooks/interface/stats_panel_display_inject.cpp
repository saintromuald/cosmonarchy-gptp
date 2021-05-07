#include "stats_panel_display.h"
#include <hook_tools.h>

namespace
{
	void __declspec(naked) stats_panel_display_Wrapper()
	{
		static BinDlg* dialog;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV dialog, EAX
			PUSHAD
		}
		hooks::stats_panel_display(dialog);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) drawBuildingInfo_Wrapper()
	{
		static BinDlg* dialog;
		__asm {
			MOV dialog, ECX
			PUSHAD
		}
		hooks::drawBuildingInfo(dialog);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) getUnitRankString_Wrapper()
	{ // 0042F370
		static Unit* a1;
		static char* result;
		__asm {
			MOV a1, ESI
			PUSHAD
		}
		result = hooks::getUnitRankString(a1);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) statsArmorLevel_Wrapper()
	{ // 00425600
		static BinDlg* dialog;
		static s32 unused;
		static u16 buttonRelated;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV buttonRelated, AX
			POP EAX
			MOV unused, EDX
			MOV dialog, EAX
			PUSHAD
		}
		hooks::statsArmorLevel(dialog, unused, buttonRelated);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) statsWeaponLevel_Wrapper()
	{ // 00425790
		static BinDlg* dialog;
		static u16 buttonRelated;
		static u8 weaponId;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0xC]
			MOV weaponId, AL
			MOV EAX, [EBP + 0x08]
			MOV buttonRelated, AX
			POP EAX
			MOV dialog, EAX
			PUSHAD
		}
		hooks::statsWeaponLevel(dialog, buttonRelated, weaponId);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) getUnitName_Wrapper() // 0047B5A0
	{
		static const char* result;
		static Unit* unit;
		__asm {
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::getUnitName(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

}

namespace hooks
{
	void injectStatsPanelDisplayHook()
	{
		jmpPatch(stats_panel_display_Wrapper,	0x00426C60, 1);
		jmpPatch(getUnitRankString_Wrapper,		0x0042F370, 2);
//		jmpPatch(drawBuildingInfo_Wrapper,		0x00427890, 3);
//		jmpPatch(statsArmorLevel_Wrapper,		0x00425600, 1);
//		jmpPatch(statsWeaponLevel_Wrapper,		0x00425790, 1);
		jmpPatch(getUnitName_Wrapper,			0x0047B5A0, 2);
	}
}
