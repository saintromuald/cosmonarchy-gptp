#include "cloak_tech.h"
#include "../SCBW/scbwdata.h"
#include "../SCBW/api.h"
#include "../hook_tools.h"
#include "globals.h"

/*
There are 9 places where StarCraft checks unit ID for cloaking.

* 0x00429210: btnscond_is_cloaked  (Used by Infested Kerrigan, Infested Duran, and Mixed Cloakable Group)
* 0x004292C0: btnscond_can_cloak   (Used by Infested Kerrigan, Infested Duran, and Mixed Cloakable Group)
* 0x00491B30: ApplyCloakingOrder  (Used by all spell-cloaking units)
* 0x004C0720: CMDRECV_Cloak       (Used by all spell-cloaking units)
* 0x0043B970: ai_order_unit_cloaking(Used by AI-controlled units)
* 0x004C0660: CMDRECV_Decloak     (Used by all spell-cloaking units)
* 0x00491A50: get_cloaking_tech     (Used by "Create Unit With Properties" in StarEdit + create_unit opcode in aiscript.bin)
* 0x00491A90: getCloakingEnergyConsumption (Used by energy regen; This is editable from the energy hook stuff)
* 0x00423540: currentUnitSelectionCanCloak(Called by CMDACT_Cloak for all units; also checks energy amount)

Note that Wraiths and Ghosts use BTNSCOND_CanCloak_0 and BTNSCOND_IsCloaked_0,
which does NOT check unit IDs at all. (FireGraft has it wrong when it says that
Wraiths use "Can Cloak" and Infested Kerrigan uses "Can Cloak - Mixed")
*/

namespace
{
	//-------- Button conditions --------//

	//0x00429210
	void __declspec(naked) cloaking_tech_wrapper_is_cloaked()
	{
		static b32 return_value;
		static u32 player;
		static Unit* unit;			// unused in original function
		static u16 requirement_var;	// unused in original function
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV player, EDX
			MOV EBX, [EBP + 0x08]
			MOV unit, EBX
			MOV requirement_var, CX
			PUSHAD
		}
		return_value = hooks::btnscond_is_cloaked(unit, player, requirement_var);
		__asm {
			POPAD
			MOV EAX, return_value
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	//0x004292C0
	void __declspec(naked) cloaking_tech_wrapper_can_cloak()
	{
		static b32 return_value;
		static u32 player;
		static Unit* unit;			// unused in original function
		static u16 requirement_var;	// unused in original function
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV player, EDX
			MOV EBX, [EBP + 0x08]
			MOV unit, EBX
			MOV requirement_var, CX
			PUSHAD
		}
		return_value = hooks::btnscond_can_cloak(unit, player, requirement_var);
		__asm {
			POPAD
			MOV EAX, return_value
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	//-------- Actual cloak orders --------//

	//equivalent to 00491B30 ApplyCloakingOrder, not injected
	void apply_cloaking_order_hook(Unit* unit)
	{
		UnitExt* unit_ext = get_unit_ext(unit);
		if (unit->status & UnitStatus::RequiresDetection)
		{
			return;
		}

		// original code is hardcoded instead of using a function like get_cloaking_tech
		const u16 energyCost = techdata_dat::energy_cost[hooks::get_cloaking_tech(unit)] * 256;

		if (scbw::is_cheat_enabled(CheatFlags::TheGathering) || unit->energy >= energyCost)
		{
			if (!scbw::is_cheat_enabled(CheatFlags::TheGathering))
			{
				bool spend = true;
				if (unit_ext->values[ValueAICloakGimmick] == 1)
				{
					spend = false;
				}
				if (unit->unit_ai != nullptr)
				{
					unit_ext->values[ValueAICloakGimmick] = 1;
				}
				if (spend)
				{
					unit->spend_unit_energy(energyCost);
				}
			}
			// original code is hardcoded instead of using a function like set_secondary_order
			if (unit->secondary_order_id != OrderId::Cloak)
			{
				unit->set_secondary_order(OrderId::Cloak);
			}
			else if (unit->unit_ai != nullptr)
			{
				unit_ext->values[ValueAICloakGimmick] = 0;
			}
		}
	}

	void __cdecl cloaking_tech_wrapper_cmdrecv_cloak()
	{
		*g_unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection())
		{
			if (unit->can_use_tech(hooks::get_cloaking_tech(unit), *g_active_nation_id) == 1)
			{
				apply_cloaking_order_hook(unit);
			}
		}
	}

	const u32 func_apply_cloak = 0x00491B30;
	void apply_cloaking_order_func(Unit* unit)
	{
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL func_apply_cloak
			POPAD
		}
	}

	void __declspec(naked) cloaking_tech_wrapper_ai_cloak_unit()
	{
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		if (!(unit->status & UnitStatus::RequiresDetection))
		{
			if (unit->can_use_tech(hooks::get_cloaking_tech(unit), unit->player) == 1)
			{
				apply_cloaking_order_hook(unit);
			}
		}
		__asm {
			POPAD
			RETN
		}
	}

	void __cdecl cloaking_tech_wrapper_cmdrecv_decloak()
	{
		*g_unit_iterator_pos = 0;
		while (Unit* unit = get_active_player_next_selection())
		{
			if (unit->can_use_tech(hooks::get_cloaking_tech(unit), *g_active_nation_id) == 1)
			{
				unit->set_secondary_order(OrderId::Decloak);
			}
		}
	}

	//-------- Other wrappers --------//

	void __declspec(naked) get_cloaking_tech_wrapper()
	{
		static Unit* unit;
		static u8 result;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::get_cloaking_tech(unit);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) cloaking_tech_apply_cloaking_orders()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		apply_cloaking_order_hook(unit);
		__asm {
			POPAD
			RETN
		}
	}

	b32 __cdecl current_unit_selection_can_cloak_wrapper()
	{
		RaceId::Enum race;
		for (s32 i = 0; i < SelectionCap; ++i)
		{
			Unit* unit = g_client_selection_group->unit[i];
			if (unit != nullptr)
			{
				if (scbw::is_cheat_enabled(CheatFlags::TheGathering)
					|| unit->energy >= techdata_dat::energy_cost[hooks::get_cloaking_tech(unit)] * 256)
				{
					return true;
				}
			}
		}
		race = (*g_active_portrait_unit)->get_race();
		scbw::show_error_message_with_sfx((*g_active_portrait_unit)->player, 864 + race, 156 + race);
		return false;
	}
}

namespace hooks
{
	void inject_cloaking_tech_hooks()
	{
//		jump_patch(cloaking_tech_wrapper_is_cloaked,			0x00429210, 1);
//		jump_patch(cloaking_tech_wrapper_can_cloak,				0x004292C0, 1);
		jump_patch(cloaking_tech_wrapper_cmdrecv_cloak,			0x004C0720, 3);
		jump_patch(cloaking_tech_wrapper_ai_cloak_unit,			0x0043B970, 1);
//		jump_patch(cloaking_tech_apply_cloaking_orders,			0x00491B30, 5);
//		jump_patch(cloaking_tech_wrapper_cmdrecv_decloak,		0x004C0660, 3);
		jump_patch(get_cloaking_tech_wrapper,					0x00491A50, 3);
//		jump_patch(current_unit_selection_can_cloak_wrapper,	0x00423540, 1);
	}
}