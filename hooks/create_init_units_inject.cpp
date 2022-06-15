//Injector source file for the Create Initial Units hook module.
#include "create_init_units.h"
#include <hook_tools.h>
#include "SCBW/scbwhelper.h"

namespace
{
	void __declspec(naked) CreateInitialMeleeBuildingsWrapper()
	{
		static u32 player;
		static u8 raceId;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV raceId, AL
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			PUSHAD
		}
		hooks::CreateInitialMeleeBuildings(raceId, player);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) CreateInitialMeleeUnitsWrapper()
	{
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::CreateInitialMeleeUnits();
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) CreateInitialOverlordWrapper()
	{
		static u8 player;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV AL, byte ptr[EBP + 0x08]
			MOV player, AL
			PUSHAD
		}
		hooks::CreateInitialOverlord(player);
		__asm
		{
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) CreateUnitXY_Wrapper() { // 004CD360
		static s16 player_id;
		static s16 id;
		static s32 x;
		static s32 y;
		static Unit* result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV x, EAX
			MOV EAX, [EBP + 0x0C]
			MOV y, EAX
			POP EAX
			MOV player_id, AX
			MOV id, CX
			PUSHAD
		}
		result = hooks::CreateUnitXY(player_id, id, x, y, false);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			MOV EAX, result
			RETN 0x8
		}
	}

	void __declspec(naked) CreatePreplacedUnit_Wrapper() { // 004CD740
		static CHKUnit* chkunit;
		static Unit* result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			MOV chkunit, ESI
			PUSHAD
		}
		result = hooks::CreatePreplacedUnit(chkunit);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_create_initial_units_hooks()
	{
		jump_patch(CreateInitialMeleeBuildingsWrapper,	0x0049D7C0, 1);
		jump_patch(CreateInitialMeleeUnitsWrapper,		0x0049DA40, 1);
		jump_patch(CreateInitialOverlordWrapper,		0x0049D6C0, 3);

		jump_patch(CreateUnitXY_Wrapper,				0x004CD360, 4);
		jump_patch(CreatePreplacedUnit_Wrapper,			0x004CD740, 3);
	}
}