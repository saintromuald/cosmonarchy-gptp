#include "building_terran.h"
#include <hook_tools.h>
#include "BW/api.h"
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

	void __declspec(naked) btnsact_place_addon_wrapper() {
		static char* buffer;
		static u16 unit_id;
		__asm {
			MOV buffer, EDI
			MOV unit_id, CX
			PUSHAD
		}
		hooks::btnsact_place_addon(unit_id, buffer);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) play_building_complete_sound_wrapper() { // 0048F310
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::play_building_complete_sound(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_place_addon_wrapper() { // 004E6880
		static u32 unused;
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			MOV unused, ESI
			PUSHAD
		}
		hooks::orders_place_addon(unused, unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) unit_is_geyser_wrapper() { // 004688B0
		static b32 result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::unit_is_geyser(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) is_constructing_addon_wrapper() { // 004E66B0
		static b32 result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::is_constructing_addon(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) generic_progress_build_wrapper() { // 004679A0
		static b32 result;
		static Unit* unit;
		static u32 hp_gain;
		static b32 is_incomplete;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV is_incomplete, EAX
			POP EAX
			MOV hp_gain, EDX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::generic_progress_build(unit, hp_gain, is_incomplete);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) disconnect_addon_wrapper() { // 00464930
		static Unit* unit;
		static Sprite* unused;
		__asm {
			MOV unit, EAX
			MOV unused, EDX
			PUSHAD
		}
		hooks::disconnect_addon(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) complete_addon_wrapper() { // 00463D50
		static Unit* addon;
		static Unit* parent;
		__asm {
			MOV parent, EDI
			MOV addon, EAX
			PUSHAD
		}
		hooks::complete_addon(addon, parent);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) play_addon_detach_sound_wrapper() { // 0048F8A0
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::play_addon_detach_sound(unit);
		__asm {
			POPAD
			RETN
		}
	}

	const u32 orders_place_addon_E6891 = 0x004E6B91;
	void __declspec(naked) orders_place_addon_patch() {
		__asm {
			PUSH	EAX
			MOV		EAX, [EDI + 0xC]	// unit->sprite
			MOV		AL, [EAX + 0xA]		// unit->sprite->player
			PUSH	ECX
			MOV		ECX, [EBX + 0xC]	// addon->sprite
			MOV[ECX + 0xA], AL			// addon->sprite->player = unit->sprite->player
			POP		ECX
			POP		EAX
			MOV[EBX + 0x80], EDI
			JMP		orders_place_addon_E6891
		}
	}

	void __declspec(naked) complete_addon_patch() {
		__asm {
			MOV		EDX, [EDI + 0xC]
			MOVZX	EDX, byte ptr[EDX + 0xA]
			MOV		ECX, ESI
			RET
		}
	}

	const u32 decollide_unit_F2E3A = 0x004F2E3A;
	const u32 decollide_unit_F2E4C = 0x004F2E4C;
	void __declspec(naked) decollide_unit_F2E35() {
		__asm {
			PUSH	EBX // unit
			PUSH	EDI // unit->velocity_direction_1
			MOV		AX, [ESI + 0x64]
			CMP		AX, 20 // unit->id == UnitId::TerranTalos
			JE		offset isTalos
			CMP		AX, 7 // unit->id == UnitId::TerranMason
			JNE		offset isNotSCV
		isTalos:
			MOV		EAX, [ESI + 0xE8]
			TEST	EAX, EAX // unit->secondaryOrderPos != null
			JE		offset isNotSCV
			MOV[EBP - 4], EAX // unit->sprite->position = unit->secondaryOrderPos
			XOR[ESI + 0xE8], EAX // unit->secondaryOrderPos = null
			JMP		decollide_unit_F2E4C
		isNotSCV:
			LEA		ECX, [EBP - 4]
			JMP		decollide_unit_F2E3A
		}
	}
}

namespace hooks {
	void inject_building_terran_hook() {
		jump_patch(orders_terran_build_self_wrapper,		0x00467760, 2);
		jump_patch(orders_scv_build_2_wrapper,			0x00467A70, 1);
		jump_patch(orders_scv_build_wrapper,				0x00467FD0, 0);
	//	jump_patch(refresh_screen_wrapper,								0x0048DDC0, 4);
	//	jump_patch(btnsact_place_addon_wrapper,							0x00423D10, 4);
		jump_patch(play_building_complete_sound_wrapper,	0x0048F310, 3);
	//	jump_patch(orders_place_addon_wrapper,							0x004E6880, 1); // bugged, patched instead
		jump_patch(unit_is_geyser_wrapper,				0x004688B0, 2);
	//	jump_patch(is_constructing_addon_wrapper,						0x004E66B0, 2); // Probably fine, but not necessary
		jump_patch(generic_progress_build_wrapper,		0x004679A0,	1);
		jump_patch(disconnect_addon_wrapper,				0x00464930,	5);
		jump_patch(complete_addon_wrapper,				0x00463D50,	1);
		jump_patch(play_addon_detach_sound_wrapper,		0x0048F8A0,	2);

		jump_patch(orders_place_addon_patch,				0x004E6B8B, 1);
		call_patch(complete_addon_patch,					0x00463D69, 0);
		jump_patch(decollide_unit_F2E35,					0x004F2E35);	 // Make Masons return to their initial point of construction after completion. -DF
	}
}