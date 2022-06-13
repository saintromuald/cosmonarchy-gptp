#include "supply_hooks.h"
#include <hook_tools.h>
#include "SCBW/api.h"

namespace
{
	void __declspec(naked) get_supplies_available_wrapper()
	{
		static u8 player;
		static s32 race;
		static u32 result;
		__asm
		{
			MOV race, ECX
			MOV player, AL
			PUSHAD
		}
		result = hooks::get_supplies_available(player, race);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) get_supplies_remaining_wrapper()
	{
		static u8 player;
		static u16 unit_id;
		static s32 result;
		__asm
		{
			MOV unit_id, DI
			MOV player, BL
			PUSHAD
		}
		result = hooks::get_supplies_remaining(player, unit_id);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) get_supplies_max_wrapper()
	{
		static u16 unit_id;
		static u8 player;
		static s32 result;
		__asm
		{
			MOV player, CL
			MOV unit_id, AX
			PUSHAD
		}
		result = hooks::get_supplies_max(unit_id, player);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) get_supplies_used_wrapper()
	{
		static s32 race;
		static u8 player;
		static s32 result;
		__asm
		{
			MOV player, CL
			MOV race, EAX
			PUSHAD
		}
		result = hooks::get_supplies_used(race, player);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) initialize_unit_counts_wrapper()
	{
		static char result;
		__asm
		{
			PUSHAD
		}
		result = hooks::initialize_unit_counts();
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) stat_res_main_proc_wrapper()
	{
		static BinDlg* dialog;
		__asm
		{
			MOV dialog, ECX
			PUSHAD
		}
		hooks::stat_res_main_proc(dialog);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) get_supplies_temp_wrapper()
	{
		static s32 result;
		__asm
		{
			PUSHAD
		}
		result = hooks::get_supplies_temp();
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) redraw_supply_info_wrapper()
	{
		static bool result;
		__asm
		{
			PUSHAD
		}
		result = hooks::redraw_supply_info();
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) draw_supply_unit_status_wrapper()
	{
		static BinDlg* dialog;
		__asm
		{
			MOV dialog, EAX
			PUSHAD
		}
		hooks::draw_supply_unit_status(dialog);
		__asm
		{
			POPAD
			RETN
		}
	}

	b32 __stdcall has_supplies_for_unit_wrapper(u8 player, u16 unit_id, b32 canShowErrorMessage)
	{
		return 1;
		if (hooks::has_supplies_for_unit(player, unit_id, canShowErrorMessage != 0))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	void __declspec(naked) increment_unit_scores_wrapper() // 00488BF0
	{
		static Unit* unit;
		static s32 count;
		static s16 result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV count, EAX
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		hooks::increment_unit_scores(unit, count);
		__asm {
			POPAD
			MOV EAX, 0
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) increment_unit_scores_ex_wrapper() // 00488D50
	{
		static s32 count;
		static Unit* unit;
		static s32 updateMorphMergeUnitScore;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV count, ECX
			MOV unit, EDI
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV updateMorphMergeUnitScore, EAX
			POP EAX
			PUSHAD
		}
		hooks::increment_unit_scores_ex(count, unit, updateMorphMergeUnitScore);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void inject_supply_hooks()
	{
		// disable red supply text
		u8 data[] = {
			0x33, 0xC0 // xor eax, eax
		};
		memory_patch(0x004E53FC, data, sizeof(data));

		jump_patch(get_supplies_available_wrapper,		0x00488900, 1);
		jump_patch(get_supplies_remaining_wrapper,		0x004889D0, 1);
		jump_patch(get_supplies_max_wrapper,			0x00488990, 4);
		jump_patch(get_supplies_used_wrapper,			0x004888C0, 1);
		jump_patch(initialize_unit_counts_wrapper,		0x00488F90, 0);
		jump_patch(stat_res_main_proc_wrapper,			0x004E5640, 4);
		jump_patch(get_supplies_temp_wrapper,			0x004246D0, 1);
		jump_patch(redraw_supply_info_wrapper,			0x00425230, 4);
		jump_patch(draw_supply_unit_status_wrapper,		0x00427540, 1); // Currently broken, supply used in unit status display won't show properly for zerg and protoss units
		jump_patch(has_supplies_for_unit_wrapper,		0x0042CF70, 1);
		jump_patch(increment_unit_scores_wrapper,		0x00488BF0, 2);
		jump_patch(increment_unit_scores_ex_wrapper,	0x00488D50, 1);
	}
}