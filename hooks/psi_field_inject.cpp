#include "psi_field.h"
#include <hook_tools.h>
#include <SCBW/api.h>

//-------- Unit selection hooks --------//

void show_all_psi_fields() {
	static const u32 Func_showAllPsiFields = 0x00493640;
	__asm {
		PUSHAD
		CALL Func_showAllPsiFields
		POPAD
	}
}

void hide_all_psi_fields() {
	static const u32 Func_hideAllPsiFields = 0x00494100;
	__asm {
		PUSHAD
		CALL Func_hideAllPsiFields
		POPAD
	}
}

// If @p unit is a psi provider, make all psi field graphics visible.
void show_all_psi_fields_on_select(Unit* unit) {
	if (!scbw::isInReplay() && hooks::unit_can_show_power_for(unit, *LOCAL_NATION_ID)) {
		if ((unit->status & UnitStatus::Completed) && hooks::is_ready_to_make_psi_field(unit)) {
			show_all_psi_fields();
		}
	}
}

// If @p unit is a psi provider, make all psi field graphics invisible.
void hide_all_psi_fields_on_unselect(Unit* unit) {
	if (!scbw::isInReplay() && hooks::unit_can_show_power_for(unit, *LOCAL_NATION_ID)) {
		if ((unit->status & UnitStatus::Completed) && hooks::is_ready_to_make_psi_field(unit)) {
			hide_all_psi_fields();
		}
	}
}

void __declspec(naked) show_all_psi_fields_on_select_wrapper() { // 0x004E6224
	static Unit* unit;
	__asm {
		MOV unit, EDI
		PUSHAD
	}
	show_all_psi_fields_on_select(unit);
	__asm {
		POPAD
		POP EDI
		POP ESI
		POP EBX
		MOV ESP, EBP
		POP EBP
		RETN 4
	}
}

void __declspec(naked) hide_all_psi_fields_on_unselect_wrapper() { // 0x004E62BE
	static Unit* unit;
	__asm {
		MOV unit, ESI
		PUSHAD
	}
	hide_all_psi_fields_on_unselect(unit);
	__asm {
		POPAD
		POP EDI	// used to be in first asm block
		POP ESI
		RETN
	}
}

//-------- Save & load game hooks --------//

void __declspec(naked) pack_unit_data_psi_provider() { // 004E3935
	static const u32 jmp_address = 0x004E3961;
	static Unit* unit;
	static u16 unit_id;
	__asm {
		MOV unit_id, CX
		MOV unit, ESI
		PUSHAD
	}
	if (hooks::can_make_psi_field(unit_id)) {
		if (unit->building.pylon_aura) {
			*((u32*)&unit->building.pylon_aura) = (unit->building.pylon_aura - spriteTable) / sizeof(Sprite) + 1;
		}
	}
	__asm {
		POPAD
		JMP jmp_address
	}
}

void __declspec(naked) unpack_unit_data_psi_provider() { // 004E3182
	static const u32 jmp_address = 0x004E31A3;
	static Unit* unit;
	static u16 unit_id;
	__asm {
		MOV unit_id, CX
		MOV unit, ESI
		PUSHAD
	}
	if (hooks::can_make_psi_field(unit_id)) {
		if (unit->building.pylon_aura) {
			unit->building.pylon_aura = &spriteTable[(s32)unit->building.pylon_aura];
		}
	}
	__asm {
		POPAD
		JMP jmp_address
	}
}

void __declspec(naked) unit_has_power_wrapper() { // 004936B0
	static u32 result;
	static u16 unit_id;
	static u32 x;
	static u32 y;
	static u32 player;
	__asm {
		PUSH EBP
		MOV EBP, ESP
		PUSH EAX
		MOV EAX, [EBP + 0x8]
		MOV x, EAX
		MOV EAX, [EBP + 0xC]
		MOV y, EAX
		MOV EAX, [EBP + 0x10]
		MOV player, EAX
		POP EAX
		MOV unit_id, AX
		PUSHAD
	}
	result = hooks::unit_has_power(unit_id, x, y, player);
	__asm {
		POPAD
		MOV EAX, result
		MOV ESP, EBP
		POP EBP
		RETN 0xC
	}
}

void __declspec(naked) show_power_sprite_wrapper() { // 00493BF0
	static u32 result;
	static Unit* unit;
	__asm {
		MOV unit, ECX
		PUSHAD
	}
	result = hooks::show_power_sprite(unit);
	__asm {
		POPAD
		MOV EAX, result
		RETN
	}
}

namespace hooks
{
	void inject_psi_field_hooks()
	{
		jump_patch(show_all_psi_fields_on_select_wrapper,		0x004E6224, 0);
		jump_patch(hide_all_psi_fields_on_unselect_wrapper,		0x004E62BE, 0);
	//	jump_patch(pack_unit_data_psi_provider,					0x004E3935, 0);
	//	jump_patch(unpack_unit_data_psi_provider,				0x004E3182, 0);
		jump_patch(unit_has_power_wrapper,						0x004936B0, 1);
		jump_patch(show_power_sprite_wrapper,					0x00493BF0, 1);
		jump_patch(create_pylon_aura,							0x00493640, 2);
	}
}
