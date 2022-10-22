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
		scbw::adjust_addon_y(*place_building_unit_id, *place_building_base_type, &addonOffsetY);
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
		scbw::adjust_addon_x(*place_building_unit_id, *place_building_base_type, &addonOffsetX);
		__asm
		{
			POPAD
			MOV AX, addonOffsetX
			JMP updatePlacementWrapperJmp2
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

	void placementSimpleWrappers()
	{
		jump_patch(update_placement_simple_wrapper,		0x0048DD42, 2);
		jump_patch(update_placement_simple_wrapper_2,	0x0048DD61, 2);
	}

	const u32 orders_PlaceAddon_E6B91 = 0x004E6B91;
	void __declspec(naked) orders_PlaceAddon_patch() {
		__asm {
			PUSH	EAX
			MOV		EAX, [EDI + 0xC]//unit->sprite
			MOV		AL, [EAX + 0xA]//unit->sprite->player
			PUSH	ECX
			MOV		ECX, [EBX + 0xC]//addon->sprite
			MOV[ECX + 0xA], AL//addon->sprite->player = unit->sprite->player
			POP		ECX
			POP		EAX
			MOV[EBX + 0x80], EDI
			JMP		orders_PlaceAddon_E6B91
		}
	}

	void __declspec(naked) completeAddon_patch() {
		__asm {
			MOV		EDX, [EDI + 0xC]
			MOVZX	EDX, byte ptr[EDX + 0xA]
			MOV		ECX, ESI
			RET
		}
	}
}

namespace hooks
{
	void inject_building_terran_hook()
	{
		jump_patch(orders_terran_build_self_wrapper,		0x00467760, 2);
		jump_patch(orders_scv_build_2_wrapper,				0x00467A70, 1);
		jump_patch(orders_scv_build_wrapper,				0x00467FD0, 0);
	//	jump_patch(refresh_screen_wrapper,					0x0048DDC0, 4);
	//	jump_patch(btnsact_place_addon_wrapper,				0x00423D10, 4);
		jump_patch(play_building_complete_sound_wrapper,	0x0048F310, 3);

	//	placementSimpleWrappers(); // bugged
		jump_patch(orders_PlaceAddon_patch,					0x004E6B8B, 1);
		call_patch(completeAddon_patch,						0x00463D69, 0);
	}
}
