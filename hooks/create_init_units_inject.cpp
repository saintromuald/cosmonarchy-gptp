#include "create_init_units.h"
#include <hook_tools.h>
#include "BW/bwhelper.h"

namespace {
	void __declspec(naked) create_initial_melee_buildings_wrapper() {
		static u32 player;
		static u8 race_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV race_id, AL
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			PUSHAD
		}
		hooks::create_initial_melee_buildings(race_id, player);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) create_initial_melee_units_wrapper() {
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSHAD
		}
		hooks::create_initial_melee_units();
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN
		}
	}

	void __declspec(naked) create_initial_ovileth_wrapper() {
		static u8 player;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV AL, byte ptr[EBP + 0x08]
			MOV player, AL
			PUSHAD
		}
		hooks::create_initial_ovileth(player);
		__asm {
			POPAD
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) create_unit_xy_wrapper() { // 004CD360
		static s16 player_id;
		static s16 id;
		static s32 x;
		static s32 y;
		static Unit* result;
		__asm {
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
		result = hooks::create_unit_xy(player_id, id, x, y, false);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			MOV EAX, result
			RETN 0x8
		}
	}

	void __declspec(naked) create_preplaced_unit_wrapper() { // 004CD740
		static ChkUnit* chk_unit;
		static Unit* result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV chk_unit, ESI
			PUSHAD
		}
		result = hooks::create_preplaced_unit(chk_unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) create_initial_team_melee_units_wrapper() { // 0049D8E0
		static u32 result;
		static s32 player;
		__asm {
			MOV player, EDI
			PUSHAD
		}
		result = hooks::create_initial_team_melee_units(player);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) chkunit_invalid_for_melee_wrapper() { // 004CBE20
		static ChkUnit* chkunit;
		static bool result;
		__asm {
			MOV chkunit, EAX
			PUSHAD
		}
		result = hooks::chkunit_invalid_for_melee(chkunit);
		__asm {
			POPAD
			MOVZX EAX, result
			RETN
		}
	}

	void __declspec(naked) get_player_force_wrapper() { // 004A8B90
		static u8 player;
		static u32 result;
		__asm {
			MOV player, AL
			PUSHAD
		}
		result = hooks::get_player_force(player);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks {
	void inject_create_initial_units_hooks() {
		jump_patch(create_initial_melee_buildings_wrapper,	0x0049D7C0, 1);
		jump_patch(create_initial_melee_units_wrapper,		0x0049DA40, 1);
		jump_patch(create_initial_ovileth_wrapper,			0x0049D6C0, 3);

		jump_patch(create_unit_xy_wrapper,					0x004CD360, 4);
		jump_patch(create_preplaced_unit_wrapper,			0x004CD740, 3);
		jump_patch(create_initial_team_melee_units_wrapper,	0x0049D8E0, 1);
		jump_patch(chkunit_invalid_for_melee_wrapper,		0x004CBE20, 1);
		jump_patch(get_player_force_wrapper,				0x004A8B90, 1);
	}
}