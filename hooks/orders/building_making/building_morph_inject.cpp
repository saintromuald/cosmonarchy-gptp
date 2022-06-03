#include "hooks\orders\building_making\building_morph.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include <hook_tools.h>
#include "SCBW\structures\Unit.h"
#include <SCBW/api.h>
#include <algorithm>

namespace
{
	void __declspec(naked) orders_isMorphingWrapper()
	{
		static Unit* building;
		static b32 result;
		__asm {
			MOV building, EAX
			PUSHAD
		}
		result = hooks::is_morphing(building) ? 1 : 0;
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) orders_ZergBuildSelfWrapper()
	{
		static Unit* building;
		__asm {
			MOV building, EAX
			PUSHAD
		}
		hooks::orders_zerg_build_self(building);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ZergPlaceBuildingWrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		hooks::zerg_place_building(unit);
		__asm {
			POPAD
			MOv ESP, EBp
			POP EBP
			RETN 4
		}
	}
}

namespace
{
	//-------- isMorphingBuilding() --------//

	void __declspec(naked) isRemorphingBuildingWrapper()
	{
		static Unit* unit;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EAX
		}
		if (!(unit->status & UnitStatus::Completed)
			&& hooks::is_morphed_building(unit->build_queue[unit->build_queue_slot]))
		{
			__asm {
				POPAD
				MOV EAX, 1
				RETN
			}
		}
		else
		{
			__asm {
				POPAD
				XOR EAX, EAX; // Set EAX to 0
				RETN
			}
		}
	}

	//-------- ai_morph_behavior() --------//

	const u32 Hook_AI_MorphBehavior_IsMorphedBuildingYes = 0x0043508B;
	void __declspec(naked) isMorphedBuildingWrapper_AI_MorphBehavior()
	{
		static u16 buildUnitId;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV buildUnitId, DI
		}
		if (hooks::is_morphed_building(buildUnitId))
		{
			__asm {
				POPAD
				JMP Hook_AI_MorphBehavior_IsMorphedBuildingYes
			}
		}
		else
		{  // Code at 0x00435122
			__asm {
				POPAD
				POP EDI
				XOR EAX, EAX
				POP ESI
				POP EBP
				RETN 8
			}
		}
	}

	//-------- statdata_UnitWireframeUpdate() --------//

	const u32 Hook_IsMorphedBuilding_ConsoleWireframeUpdateBack = 0x00456955;
	void __declspec(naked) isMorphedBuildingWrapper_ConsoleWireFrameUpdate()
	{
		static Unit* unit;
		static u16 buildUnitId;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, ESI
		}
		buildUnitId = unit->build_queue[unit->build_queue_slot];
		if (unit->status & UnitStatus::Completed
			|| !hooks::is_morphed_building(buildUnitId))
		{
			buildUnitId = unit->id;
		}
		__asm {
			POPAD
			MOVZX ECX, buildUnitId
			JMP Hook_IsMorphedBuilding_ConsoleWireframeUpdateBack
		}
	}

	//-------- orders_zerg_build_self() --------//

	const u32 Hook_isMorphedBuilding_ZergBuildSelf_GetTypeBack = 0x0045D53F;
	void __declspec(naked) isMorphedBuildingWrapper_ZergBuildSelf_GetType()
	{
		static Unit* unit;
		static u16 buildUnitId;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EDI
		}
		buildUnitId = unit->build_queue[unit->build_queue_slot];
		if (!hooks::is_morphed_building(buildUnitId))
		{
			buildUnitId = unit->id;
		}
		__asm {
			POPAD
			MOVZX EBX, buildUnitId
			JMP Hook_isMorphedBuilding_ZergBuildSelf_GetTypeBack
		}
	}

	void __declspec(naked) isMorphedBuildingWrapper_ZergBuildSelf_Complete()
	{
		static u16 unit_id;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit_id, AX
		}
		if (hooks::is_morphed_building(unit_id))
		{
			__asm {
				POPAD
				MOV EAX, 1
				RETN
			}
		}
		else
		{
			__asm {
				POPAD
				XOR EAX, EAX;
				RETN
			}
		}
	}

	// This was moved out of the naked function to prevent stack corruption bug
	void zergBuildSelf_UpdateBuildTimerAndHp(Unit* unit, u16 unit_id)
	{
		bool isOperationCwalEnabled = scbw::is_cheat_enabled(CheatFlags::OperationCwal);
		if (unit->remaining_build_time)
		{
			if (isOperationCwalEnabled)
				unit->remaining_build_time -= std::min<u16>(unit->remaining_build_time, 16);
			else
				unit->remaining_build_time--;
		}
		if (!hooks::is_morphed_building(unit_id))
		{
			u32 hpGain = unit->build_repair_hp_gain;
			if (isOperationCwalEnabled)
				hpGain *= 16;
			unit->setHp(unit->hitpoints + hpGain);
		}
	}

	void __declspec(naked) isMorphedBuildingWrapper_ZergBuildSelf_SetTimer()
	{
		static Unit* unit;
		static u16 unit_id;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EDI
			MOV unit_id, BX
		}
		zergBuildSelf_UpdateBuildTimerAndHp(unit, unit_id);
		__asm {
			POPAD
			POP EDI
			POP ESI
			POP EBX
			RETN
		}
	}

	//-------- cancelZergBuilding --------//

	const u32 Hook_IsMorphedBuilding_CancelZergBuildingContinue = 0x0045DA93;
	const u32 Func_CancelZergBuildingRemorph = 0x0045D410;
	void __declspec(naked) isMorphedBuildingWrapper_CancelZergBuilding()
	{
		static Unit* unit;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EBX
		}
		if (!(unit->status & UnitStatus::Completed)
			&& hooks::is_morphed_building(unit->build_queue[unit->build_queue_slot]))
		{
			//scbw::print("Morph detected");
			__asm {
				POPAD
				MOV EAX, EBX
				CALL Func_CancelZergBuildingRemorph
				POP EBX
				MOV ESP, EBP
				POP EBP
				RETN 4
			}
		}
		else
		{
			//scbw::print("Morph is not detected");
			__asm {
				POPAD
				JMP Hook_IsMorphedBuilding_CancelZergBuildingContinue
			}
		}
	}

	//-------- AI_GetWaitBuildUnitCount --------//

	// Inject @ 0x00433286
	const u32 Hook_AI_GetWaitBuildUnitCount_Return = 0x004333D3;
	void __declspec(naked) getMorphBuildingTypeCountWrapper_AI_GetWaitBuildUnitCount()
	{
		static Unit* unit;
		static u16 unit_id;
		static b32 ignoreIncomplete;
		static u32 unitCount;
		__asm {
			PUSHAD
			MOV ignoreIncomplete, EDX
			MOV unit_id, DI
			MOV unit, ESI
		}
		unitCount = hooks::get_morph_building_id_count(unit, unit_id, ignoreIncomplete != 0);
		__asm {
			POPAD
			MOV EAX, unitCount
			JMP Hook_AI_GetWaitBuildUnitCount_Return
		}
	}

	//-------- AI_GetUnitCount --------//

	// Inject @ 0x004334D9
	const u32 Hook_AI_GetUnitCount_Return = 0x00433588;
	void __declspec(naked) getMorphBuildingTypeCountWrapper_AI_GetUnitCount()
	{
		static Unit* unit;
		static u16 unit_id;
		static u32 unitCount;
		__asm {
			PUSHAD
			MOV unit_id, DI
			MOV unit, ESI
			MOV unitCount, EBX
		}
		unitCount += hooks::get_morph_building_id_count(unit, unit_id, false);
		__asm {
			POPAD
			MOV EBX, unitCount
			JMP Hook_AI_GetUnitCount_Return
		}
	}

	//-------- AI_ManageBases --------//

	// Inject @ 0x00436439

	const u32 Hook_MorphSpecialStateContinue = 0x0045D2B1;
	void __declspec(naked) MorphSpecialStateSetup_Wrapper()
	{
		static Sprite* sprite;
		__asm {
			PUSHAD
			MOV sprite, ECX
		}
		hooks::set_special_state_1(sprite);
		__asm {
			POPAD
			JMP Hook_MorphSpecialStateContinue
		}
	}

	const u32 Hook_AI_ManageBases_Return = 0x004364F7;
	void __declspec(naked) getMorphBuildingTypeCountWrapper_AI_ManageBases()
	{
		static Unit* unit;
		static u16 unit_id;
		static u32 unitCount;
		__asm {
			PUSHAD
			MOV EAX, [EBP - 16]
			MOV unit_id, AX
			MOV unit, ESI
			MOV unitCount, EBX
		}
		unitCount += hooks::get_morph_building_id_count(unit, unit_id, false);
		__asm {
			POPAD
			MOV EBX, unitCount
			JMP Hook_AI_ManageBases_Return
		}
	}
}

extern const u32 Func_IsRemorphingBuilding;
namespace hooks
{
	void injectBuildingMorphHooks()
	{
		jump_patch(orders_isMorphingWrapper,									0x0045CD00, 2);
		jump_patch(orders_ZergBuildSelfWrapper,									0x0045D500, 0);
		jump_patch(ZergPlaceBuildingWrapper,									0x0045DA40, 1);
		jump_patch(isRemorphingBuildingWrapper,																																															Func_IsRemorphingBuilding);
		jump_patch(isMorphedBuildingWrapper_AI_MorphBehavior,					0x0043505F);
		jump_patch(isMorphedBuildingWrapper_ConsoleWireFrameUpdate,				0x00456911);
		jump_patch(isMorphedBuildingWrapper_ZergBuildSelf_GetType,				0x0045D505);
		call_patch(isMorphedBuildingWrapper_ZergBuildSelf_Complete,				0x0045D65D);
		jump_patch(isMorphedBuildingWrapper_ZergBuildSelf_SetTimer,				0x0045D56C);
		jump_patch(isMorphedBuildingWrapper_CancelZergBuilding,					0x0045DA4A);
		jump_patch(getMorphBuildingTypeCountWrapper_AI_GetWaitBuildUnitCount,	0x00433286);
		jump_patch(getMorphBuildingTypeCountWrapper_AI_GetUnitCount,			0x004334D9);
		jump_patch(getMorphBuildingTypeCountWrapper_AI_ManageBases,				0x00436439);
		jump_patch(MorphSpecialStateSetup_Wrapper,								0x0045D2A1);
	}
}