#include "cloak_tech.h"
#include "../SCBW/scbwdata.h"
#include "../SCBW/api.h"
#include "../hook_tools.h"
#include "globals.h"
/*

There are 9 places where StarCraft checks unit ID for cloaking.

* 0x00429210: BTNSCOND_IsCloaked  (Used by Infested Kerrigan, Infested Duran, and Mixed Cloakable Group)
* 0x004292C0: BTNSCOND_CanCloak   (Used by Infested Kerrigan, Infested Duran, and Mixed Cloakable Group)
* 0x00491B30: ApplyCloakingOrder  (Used by all spell-cloaking units)
* 0x004C0720: CMDRECV_Cloak       (Used by all spell-cloaking units)
* 0x0043B970: ai_order_unit_cloaking(Used by AI-controlled units)
* 0x004C0660: CMDRECV_Decloak     (Used by all spell-cloaking units)
* 0x00491A50: getCloakingTech     (Used by "Create Unit With Properties" in StarEdit + create_unit opcode in aiscript.bin)
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
	void __declspec(naked) cloakingTechWrapper_IsCloaked()
	{
		static b32 return_value;
		static u32 playerId;
		static Unit* unit;	// unused in original function
		static u16 reqVar;	// unused in original function
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV playerId, EDX
			MOV EBX, [EBP + 0x08]
			MOV unit, EBX
			MOV reqVar, CX
			PUSHAD
		}
		return_value = hooks::BTNSCOND_IsCloaked(unit, playerId, reqVar);
		__asm {
			POPAD
			MOV EAX, return_value
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	//0x004292C0
	void __declspec(naked) cloakingTechWrapper_CanCloak()
	{
		static b32 return_value;
		static u32 playerId;
		static Unit* unit;	// unused in original function
		static u16 reqVar;	// unused in original function
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV playerId, EDX
			MOV EBX, [EBP + 0x08]
			MOV unit, EBX
			MOV reqVar, CX
			PUSHAD
		}
		return_value = hooks::BTNSCOND_CanCloak(unit, playerId, reqVar);
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
	void applyCloakingOrderHook(Unit* unit)
	{
		UnitExt* unitExt = globals.get_unit_ext(unit);
		if (unit->status & UnitStatus::RequiresDetection)
		{
			return;
		}

		// original code is hardcoded instead of using a function like getCloakingTech
		const u16 energyCost = techdata_dat::EnergyCost[hooks::getCloakingTech(unit)] * 256;

		if (scbw::isCheatEnabled(CheatFlags::TheGathering) || unit->energy >= energyCost)
		{
			if (!scbw::isCheatEnabled(CheatFlags::TheGathering))
			{
				bool spend = true;
				if (unitExt->values[ValueAICloakGimmick] == 1)
				{
					spend = false;
				}
				if (unit->unit_ai != nullptr)
				{
					unitExt->values[ValueAICloakGimmick] = 1;
				}
				if (spend)
				{
					unit->spendUnitEnergy(energyCost);
				}
			}
			//original code is hardcoded instead of using a function like setSecondaryOrder
			if (unit->secondaryOrderId != OrderId::Cloak)
			{
				unit->setSecondaryOrder(OrderId::Cloak);
			}
			else if (unit->unit_ai != nullptr)
			{
				unitExt->values[ValueAICloakGimmick] = 0;
			}
		}
	}

	void __cdecl cloakingTechWrapper_CMDRECV_Cloak()
	{
		*selectionIndexStart = 0;
		while (Unit* unit = getActivePlayerNextSelection())
		{
			if (unit->canUseTech(hooks::getCloakingTech(unit), *ACTIVE_NATION_ID) == 1)
				applyCloakingOrderHook(unit);
		}
	}

	const u32 Func_ApplyCloak = 0x00491B30;
	void applyCloakingOrderFunc(Unit* unit)
	{
		__asm {
			PUSHAD
			MOV EAX, unit
			CALL Func_ApplyCloak
			POPAD
		}
	}

	void __declspec(naked) cloakingTechWrapper_AI_cloakUnit()
	{
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		if (!(unit->status & UnitStatus::RequiresDetection))
		{
			if (unit->canUseTech(hooks::getCloakingTech(unit), unit->player) == 1)
			{
				applyCloakingOrderHook(unit);
				//applyCloakingOrderFunc(unit);

			}
		}
		__asm {
			POPAD
			RETN
		}
	}

	void __cdecl cloakingTechWrapper_CMDRECV_Decloak()
	{
		*selectionIndexStart = 0;
		while (Unit* unit = getActivePlayerNextSelection())
		{
			if (unit->canUseTech(hooks::getCloakingTech(unit), *ACTIVE_NATION_ID) == 1)
				unit->setSecondaryOrder(OrderId::Decloak);
		}
	}

	//-------- Other wrappers --------//

	void __declspec(naked) getCloakingTechWrapper()
	{
		static Unit* unit;
		static u8 result;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::getCloakingTech(unit);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) cloakingTech_ApplyCloakingOrder()
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		applyCloakingOrderHook(unit);
		__asm {
			POPAD
			RETN
		}
	}

	b32 __cdecl currentUnitSelectionCanCloakWrapper()
	{
		RaceId::Enum race;
		for (s32 i = 0; i < SELECTION_ARRAY_LENGTH; ++i)
		{
			Unit* unit = clientSelectionGroup->unit[i];
			if (unit != nullptr)
			{
				if (scbw::isCheatEnabled(CheatFlags::TheGathering)
					|| unit->energy >= techdata_dat::EnergyCost[hooks::getCloakingTech(unit)] * 256)
				{
					return true;
				}
			}
		}
		race = (*activePortraitUnit)->getRace();
		scbw::showErrorMessageWithSfx((*activePortraitUnit)->player, 864 + race, 156 + race);
		return false;
	}
}

namespace hooks
{
	void injectCloakingTechHooks()
	{
//		jump_patch(cloakingTechWrapper_IsCloaked,       0x00429210, 1);
//		jump_patch(cloakingTechWrapper_CanCloak,        0x004292C0, 1);
		jump_patch(cloakingTechWrapper_CMDRECV_Cloak,	0x004C0720, 3);
		jump_patch(cloakingTechWrapper_AI_cloakUnit,	0x0043B970, 1);
//		jump_patch(cloakingTech_ApplyCloakingOrder,		0x00491B30, 5);
//		jump_patch(cloakingTechWrapper_CMDRECV_Decloak, 0x004C0660, 3);
//		jump_patch(getCloakingTechWrapper,              0x00491A50, 3);
//		jump_patch(currentUnitSelectionCanCloakWrapper, 0x00423540, 1);
	}
}