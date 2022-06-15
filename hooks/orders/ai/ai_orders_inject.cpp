#include "ai_orders.h"
#include <hook_tools.h>
#include "SCBW/api.h"

namespace
{
	void __declspec(naked) AI_UnitWasHit_Wrapper() // 0043F320
	{
		static Unit* self;
		static Unit* attacker;
		static s32 mainTargetReactions;
		static s32 dontCallHelp;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV dontCallHelp, EAX
			MOV EAX, [EBP + 0x0C]
			MOV mainTargetReactions, EAX
			POP EAX
			MOV attacker, EDX
			MOV self, ECX
			PUSHAD
		}
		hooks::ai_unit_was_hit(self, attacker, mainTargetReactions, dontCallHelp);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 8
		}
	}

	void __declspec(naked) AI_OrderUnitCloaking_Wrapper() // 0043B970
	{
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		hooks::ai_order_unit_cloaking(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_SendSuicide_Wrapper() // 0043E050
	{
		static char argType;
		static s32 player;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			POP EAX
			MOV argType, AL
			PUSHAD
		}
		hooks::ai_send_suicide(argType, player);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) AI_ReactToHit_TownNullCheck()
	{
		static BuildingAi* v7;
		__asm
		{
			MOV v7, EAX
		}
		if (v7->town)
		{
			v7->town->inBattle = 1;
		}
		__asm
		{
			RETN
		}
	}

	void __declspec(naked) AI_RecalculateRegionStrength_Wrapper()
	{
		static u32 result;
		static AiRegion* region;
		__asm
		{
			MOV region, ESI
			PUSHAD
		}
		result = hooks::ai_recalculate_region_strength(region);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) getClosestEnemyStructure_Wrapper() // 004A0F90
	{
		static s32 x;
		static s32 y;
		static s32 player;
		static Unit* result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			POP EAX
			MOV y, ECX
			MOV x, EAX
			PUSHAD
		}
		result = hooks::get_closest_enemy_structure(x, y, player);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	void __declspec(naked) getRegionIdFromUnit_Wrapper() // 0049CB40
	{
		static Unit* unit;
		static s16 result;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::get_region_id_from_unit(unit);
		__asm
		{
			POPAD
			MOV AX, result
			RETN
		}
	}

	void __declspec(naked) getBestSCVForRepairProc_Wrapper() // 004402B0
	{
		static bool result;
		static Unit* repairman;
		static Unit* repairTarget;
		__asm
		{
			MOV repairTarget, EDX
			MOV repairman, ECX
			PUSHAD
		}
		result = hooks::get_best_worker_for_repair_proc(repairman, repairTarget);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) AI_military_should_wait_for_group_wrapper() // 0043AA10
	{
		static b32 result;
		static Unit* unit;
		__asm
		{
			MOV unit, EBX
			PUSHAD
		}
		result = hooks::ai_military_should_wait_for_group(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) deleteAITownIfNeeded_Wrapper() // 004347E0
	{
		static AiTown* town;
		static AiTown* result;
		__asm
		{
			MOV town, EAX
			PUSHAD
		}
		result = hooks::delete_ai_town_if_needed(town);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) AI_CanMineExtra_Wrapper() // 00447A70
	{
		static Unit* unit;
		static bool result;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::ai_can_mine_extra(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) AI_is_building_safe_wrapper() // 00438910
	{
		static Unit* unit;
		static bool result;
		__asm
		{
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::ai_is_building_safe(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) AI_are_on_connected_regions_wrapper() // 00438E10
	{
		static Unit* unit1;
		static Unit* unit2;
		static bool result;
		__asm
		{
			MOV unit1, EAX
			MOV unit2, ECX
			PUSHAD
		}
		result = hooks::ai_are_on_connected_regions(unit1, unit2);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) getBunkerPlacementPosition_Wrapper() // 004442E0
	{
		static u32 placementId;
		static u32* regionId;
		static s32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV placementId, EAX
			MOV EAX, [EBP + 0x0C]
			MOV regionId, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::get_bunker_placement_position(placementId, regionId);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}

	void __declspec(naked) AI_GetBuildLocation_Wrapper() // 004461B0
	{
		static u32 unit_id;
		static char* placementData;
		static u32 player;
		static Point16 desiredPos;
		static s32 maxRange;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			MOV EAX, [EBP + 0x0C]
			MOV desiredPos, EAX
			MOV EAX, [EBP + 0x10]
			MOV maxRange, EAX
			POP EAX
			MOV placementData, ESI
			MOV unit_id, EBX
			PUSHAD
		}
		hooks::ai_get_build_location(unit_id, placementData, player, desiredPos, maxRange);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) AI_check_military_grouping_wrapper() // 00439550
	{
		static bool result;
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::ai_check_military_grouping(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) attack_recent_attacker_wrapper() { // 004770E0
		static s32 result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::attack_recent_attacker(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) AI_get_vespene_type_wrapper() { // 004326E0
		static u32 result;
		static Unit* unit;
		static u32 player_id;
		__asm {
			MOV player_id, EDX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::ai_get_vespene_type(unit, player_id, false);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) get_ai_refinery_wrapper() { // 00432B90
		static Unit* result;
		static AiTown* town;
		static u32 gas_state;
		static u32 index;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV gas_state, EAX
			MOV EAX, [EBP + 0xC]
			MOV index, EAX
			POP EAX
			MOV town, ESI
			PUSHAD
		}
		result = hooks::get_ai_refinery(town, gas_state, index, false);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}

	void __declspec(naked) AI_worker_controller_wrapper() { // 00435210
		static Unit* worker;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV worker, EAX
			POP EAX
			PUSHAD
		}
		hooks::ai_worker_controller(worker);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	void __declspec(naked) AI_reassign_worker_resources_wrapper() { // 00435900
		static char result;
		static AiTown* town;
		__asm {
			MOV town, EAX
			PUSHAD
		}
		result = hooks::ai_reassign_worker_resources(town);
		__asm {
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) AI_auto_harvest_wrapper() { // 00433B90
		static Unit* mineral;
		static Unit* worker;
		static Unit* gas;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV gas, EAX
			POP EAX
			MOV worker, ECX
			MOV mineral, EAX
			PUSHAD

		}
		hooks::ai_auto_harvest(mineral, worker, gas);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	void __declspec(naked) ai_get_build_order_id_wrapper() { // 00434C31
		static Unit* unit;
		static u32 unit_id;
		static u8 order_id;
		_asm {
			MOV unit_id, EBX
			MOV unit, EDI
			PUSHAD
		}
		order_id = hooks::ai_get_build_order_id(unit, unit_id);
		if (order_id != OrderId::None) {
			static u32 jmp_address = 0x00434C53;
			__asm {
				POPAD
				MOV AL, order_id
				JMP jmp_address
			}
		} else {
			static u32 jmp_address = 0x00434C80;
			__asm {
				POPAD
				JMP jmp_address
			}
		}
	}

	void __declspec(naked) AI_find_best_repair_unit_wrapper() { // 00443160
		static Unit* result;
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::ai_find_best_repair_unit(unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) AI_SCV_repair_controller_wrapper() { // 00433C70
		static s32 result;
		static Unit* repairer;
		static AiTown* town;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV repairer, EAX
			MOV EAX, [EBP + 0xC]
			MOV town, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::ai_scv_repair_controller(repairer, town);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}

	void __declspec(naked) ai_expand_wrapper() { // 0045B530
		static AiScript* aiscript;
		static u32 block;
		static u32 amount;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV block, EAX
			MOV EAX, [EBP + 0xC]
			MOV amount, EAX
			POP EAX
			MOV aiscript, ESI
			PUSHAD
		}
		hooks::ai_expand(aiscript, block, amount);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}

	void __declspec(naked) ai_manage_military_controller_wrapper() { // 0043D910
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::ai_manage_military_controller(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_assign_region_to_slowest_unit_wrapper() { // 00436F70
		static AiRegion* region;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV region, EAX
			POP EAX
			PUSHAD
		}
		hooks::ai_assign_region_to_slowest_unit(region);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	void __declspec(naked) ai_respond_to_cloaked_threat_wrapper() { // 0043C580
		static Unit* victim;
		static Unit* threat;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV threat, EAX
			POP EAX
			MOV victim, EAX
			PUSHAD
		}
		hooks::ai_respond_to_cloaked_threat(victim, threat);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) ai_try_return_home_wrapper() { // 00462EA0
		static u32 result;
		static Unit* unit;
		static u32 dont_issue_order;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV dont_issue_order, EAX
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::ai_try_return_home(unit, dont_issue_order);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}
}

namespace hooks
{
	void inject_ai_orders_hooks()
	{
//		jump_patch(AI_UnitWasHit_Wrapper,						0x0043F320, 1);

		jump_patch(AI_OrderUnitCloaking_Wrapper,				0x0043B970, 1);
		jump_patch(AI_SendSuicide_Wrapper,						0x0043E050, 1);
		jump_patch(AI_RecalculateRegionStrength_Wrapper,		0x0043A390, 1);
		jump_patch(getClosestEnemyStructure_Wrapper,			0x004A0F90, 1);
		jump_patch(getRegionIdFromUnit_Wrapper,					0x0049CB40, 2);
		jump_patch(getBestSCVForRepairProc_Wrapper,				0x004402B0, 1);

		jump_patch(AI_military_should_wait_for_group_wrapper,	0x0043AA10, 5);
//		jump_patch(deleteAITownIfNeeded_Wrapper,				0x004347E0, 3);
//		jump_patch(AI_CanMineExtra_Wrapper,						0x00447A70, 1);
		jump_patch(AI_is_building_safe_wrapper,					0x00438910, 2);
		jump_patch(AI_are_on_connected_regions_wrapper,			0x00438E10, 1);

//		jump_patch(getBunkerPlacementPosition_Wrapper,			0x004442E0, 4);
//		jump_patch(AI_GetBuildLocation_Wrapper,					0x004461B0, 1);
		jump_patch(AI_check_military_grouping_wrapper,			0x00439550, 1);
		jump_patch(hooks::ai_remove_different_height,			0x004436C0, 3);
//		jump_patch(attack_recent_attacker_wrapper,				0x004770E0, 2);

		jump_patch(AI_get_vespene_type_wrapper,					0x004326E0, 2);
		jump_patch(get_ai_refinery_wrapper,						0x00432B90, 0);
		jump_patch(AI_worker_controller_wrapper,				0x00435210, 0);
		jump_patch(AI_reassign_worker_resources_wrapper,		0x00435900, 1);
		jump_patch(AI_auto_harvest_wrapper,						0x00433B90, 1);
		jump_patch(ai_get_build_order_id_wrapper,				0x00434C31, 0);

		jump_patch(AI_find_best_repair_unit_wrapper,			0x00443160, 1);
	//	jump_patch(AI_SCV_repair_controller_wrapper,			0x00433C70, 1);
		jump_patch(ai_expand_wrapper,							0x0045B530, 1);
		jump_patch(ai_remove_addon_placements,					0x00443CC0, 3);
		jump_patch(ai_manage_military_controller_wrapper,		0x0043D910, 1);
		jump_patch(ai_assign_region_to_slowest_unit_wrapper,	0x00436F70, 2);
	//	jump_patch(ai_respond_to_cloaked_threat_wrapper,		0x0043C580,	1);
		jump_patch(ai_try_return_home_wrapper,					0x00462EA0, 1);

		call_patch(AI_ReactToHit_TownNullCheck,					0x0043F388, 5);

		// Disable worker and zethrokor defense gimmick
		nops(0x0043F85E, 2);
		nops(0x0043F864, 2);
	}
}