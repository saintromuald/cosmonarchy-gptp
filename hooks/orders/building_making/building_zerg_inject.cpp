#include "hooks\orders\building_making\building_zerg.h"
#include "hooks\orders\unit_making\unit_morph.h"
#include <hook_tools.h>
#include "BW\structures\Unit.h"
#include <BW/api.h>
#include <algorithm>

namespace
{
	void __declspec(naked) orders_is_morphing_wrapper()
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

	void __declspec(naked) orders_zerg_build_self_wrapper()
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

	void __declspec(naked) cancel_zerg_building_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			PUSHAD
		}
		hooks::cancel_zerg_building(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	const u32 begin_gas_building_6790F = 0x0046790F;
	void __declspec(naked) begin_gas_building_patch() {
		__asm
		{
			MOV		EDX, [ESI + 0xC]	// unit->sprite
			MOV		DL, [EDX + 0xA]		// unit->sprite->player
			MOV		ECX, EBX
			JMP		begin_gas_building_6790F
		}
	}
}

namespace
{
	//-------- isMorphingBuilding() --------//

	void __declspec(naked) is_remorphing_building_wrapper()
	{
		static Unit* unit;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EAX
		}
		if (!(unit->status & UnitStatus::Completed) && hooks::is_morphed_building(unit->build_queue[unit->build_queue_slot]))
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
	void __declspec(naked) is_morphed_building_wrapper_ai_morph_behavior()
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
	void __declspec(naked) is_morphed_building_wrapper_console_wireframe_update()
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
	void __declspec(naked) is_morphed_building_wrapper_zerg_build_self_get_id()
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

	void __declspec(naked) is_morphed_building_wrapper_zerg_build_self_complete()
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
	void zerg_build_self_update_timer_and_hitpoints(Unit* unit, u16 unit_id)
	{
		bool isOperationCwalEnabled = bw::is_cheat_enabled(CheatFlags::OperationCwal);
		if (unit->remaining_build_time)
		{
			if (isOperationCwalEnabled)
			{
				unit->remaining_build_time -= std::min<u16>(unit->remaining_build_time, 16);
			}
			else
			{
				unit->remaining_build_time--;
			}
		}
		if (!hooks::is_morphed_building(unit_id))
		{
			u32 hpGain = unit->build_repair_hp_gain;
			if (isOperationCwalEnabled)
			{
				hpGain *= 16;
			}
			unit->set_hitpoints(unit->hitpoints + hpGain);
		}
	}

	void __declspec(naked) is_morphed_building_wrapper_zerg_build_self_set_timer()
	{
		static Unit* unit;
		static u16 unit_id;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EDI
			MOV unit_id, BX
		}
		zerg_build_self_update_timer_and_hitpoints(unit, unit_id);
		__asm {
			POPAD
			POP EDI
			POP ESI
			POP EBX
			RETN
		}
	}

	//-------- cancel_zerg_building --------//

	const u32 Hook_IsMorphedBuilding_CancelZergBuildingContinue = 0x0045DA93;
	const u32 Func_CancelZergBuildingRemorph = 0x0045D410;
	void __declspec(naked) is_morphed_building_wrapper_cancel_zerg_building()
	{
		static Unit* unit;
		__asm {
			PUSHAD
			MOV EBP, ESP
			MOV unit, EBX
		}
		if (!(unit->status & UnitStatus::Completed) && hooks::is_morphed_building(unit->build_queue[unit->build_queue_slot]))
		{
			__asm {
				POPAD
				MOV EAX, EBX
				CALL Func_CancelZergBuildingRemorph
				POP EBX
				MOV ESP, EBP
				POP EBP
				RETN 0x04
			}
		}
		else
		{
			__asm {
				POPAD
				JMP Hook_IsMorphedBuilding_CancelZergBuildingContinue
			}
		}
	}

	//-------- AI_GetWaitBuildUnitCount --------//

	// Inject @ 0x00433286
	const u32 Hook_AI_GetWaitBuildUnitCount_Return = 0x004333D3;
	void __declspec(naked) get_morph_building_id_count_wrapper_ai_get_wait_build_unit_count()
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
	void __declspec(naked) get_morph_building_id_count_type_ai_get_unit_count()
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
	void __declspec(naked) morph_special_state_setup_wrapper()
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
	void __declspec(naked) get_morph_building_id_count_wrapper_ai_manage_bases()
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

	void __declspec(naked) drone_morph_standard_wrapper() { // 0045D890
		static Unit* unit;
		static u32 id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV id, EAX
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		hooks::drone_morph_standard(unit, id);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) begin_gas_building_wrapper() { // 004678A0
		static Unit* result;
		static Unit* unit;
		static u32 id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV id, EAX
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::begin_gas_building(unit, id);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) orders_drone_build_wrapper() { // 0045E090
		static Unit* builder;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV builder, EAX
			POP EAX
			PUSHAD
		}
		hooks::orders_drone_build(builder);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) orders_drone_land_wrapper() { // 004E9AA0
		static Unit* builder;
		__asm {
			MOV builder, EAX
			PUSHAD
		}
		hooks::orders_drone_land(builder);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_morph2_wrapper() // 0045D0D0
	{
		static Unit* unit;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			PUSHAD
		}
		hooks::orders_morph2(unit);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) droleth_build_excisant_wrapper() // 0045D790
	{
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::droleth_build_excisant(unit);
		__asm {
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void inject_building_zerg_hooks()
	{
		jump_patch(orders_is_morphing_wrapper,											0x0045CD00, 2);
		jump_patch(orders_zerg_build_self_wrapper,										0x0045D500, 0);
		jump_patch(cancel_zerg_building_wrapper,										0x0045DA40, 1);
		jump_patch(begin_gas_building_patch,											0x0046790A);

		jump_patch(is_remorphing_building_wrapper,										0x0045CD00);
		jump_patch(is_morphed_building_wrapper_ai_morph_behavior,						0x0043505F);
		jump_patch(is_morphed_building_wrapper_console_wireframe_update,				0x00456911);
		jump_patch(is_morphed_building_wrapper_zerg_build_self_get_id,					0x0045D505);
		call_patch(is_morphed_building_wrapper_zerg_build_self_complete,				0x0045D65D);
		jump_patch(is_morphed_building_wrapper_zerg_build_self_set_timer,				0x0045D56C);
		jump_patch(is_morphed_building_wrapper_cancel_zerg_building,					0x0045DA4A);
		jump_patch(get_morph_building_id_count_wrapper_ai_get_wait_build_unit_count,	0x00433286);
		jump_patch(get_morph_building_id_count_type_ai_get_unit_count,					0x004334D9);
		jump_patch(get_morph_building_id_count_wrapper_ai_manage_bases,					0x00436439);
		jump_patch(morph_special_state_setup_wrapper,									0x0045D2A1);
		jump_patch(drone_morph_standard_wrapper,										0x0045D890, 0);
		jump_patch(begin_gas_building_wrapper,											0x004678A0, 0);
	//	jump_patch(orders_drone_build_wrapper,											0x0045E090, 0);
	//	jump_patch(orders_drone_land_wrapper,											0x004E9AA0, 1);
		jump_patch(orders_morph2_wrapper,												0x0045D0D0, 2);
		jump_patch(droleth_build_excisant_wrapper,										0x0045D790, 5);
	}
}