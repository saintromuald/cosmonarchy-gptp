#include "ai_orders.h"
#include <hook_tools.h>
#include "SCBW/api.h"

namespace
{
	void __declspec(naked) ai_unit_was_hit_wrapper() // 0043F320
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
			MOV mainTargetReactions, EAX
			MOV EAX, [EBP + 0x0C]
			MOV dontCallHelp, EAX
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

	void __declspec(naked) ai_order_unit_cloaking_wrapper() // 0043B970
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

	void __declspec(naked) ai_send_suicide_wrapper() // 0043E050
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

	void __declspec(naked) ai_react_to_hit_town_nullcheck()
	{
		static BuildingAi* v7;
		__asm
		{
			MOV v7, EAX
		}
		if (v7->town)
		{
			v7->town->in_battle = 1;
		}
		__asm
		{
			RETN
		}
	}

	void __declspec(naked) ai_recalculate_region_strength_wrapper()
	{
		static AiRegion* region;
		__asm
		{
			MOV region, ESI
			PUSHAD
		}
		hooks::ai_recalculate_region_strength(region);
		__asm
		{
			POPAD
			MOV EAX, 0
			RETN
		}
	}

	void __declspec(naked) get_closest_enemy_structure_wrapper() // 004A0F90
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

	void __declspec(naked) get_region_id_from_unit_wrapper() // 0049CB40
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

	void __declspec(naked) get_best_worker_for_repair_proc_wrapper() // 004402B0
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

	void __declspec(naked) ai_military_should_wait_for_group_wrapper() // 0043AA10
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

	void __declspec(naked) delete_ai_town_if_needed_wrapper() // 004347E0
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

	void __declspec(naked) ai_can_mine_extra_wrapper() // 00447A70
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

	void __declspec(naked) ai_is_building_safe_wrapper() // 00438910
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

	void __declspec(naked) ai_are_on_connected_regions_wrapper() // 00438E10
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

	void __declspec(naked) get_bunker_placement_position_wrapper() // 004442E0
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

	void __declspec(naked) ai_get_build_location_wrapper() // 004461B0
	{
		static u32 unit_id;
		static char* placement_data;
		static u32 player;
		static Point16 desired_pos;
		static s32 max_range;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			MOV EAX, [EBP + 0x0C]
			MOV desired_pos, EAX
			MOV EAX, [EBP + 0x10]
			MOV max_range, EAX
			POP EAX
			MOV placement_data, ESI
			MOV unit_id, EBX
			PUSHAD
		}
		hooks::ai_get_build_location(unit_id, placement_data, player, desired_pos, max_range);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) ai_check_military_grouping_wrapper() // 00439550
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

	void __declspec(naked) ai_get_vespene_type_wrapper() { // 004326E0
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

	void __declspec(naked) ai_worker_controller_wrapper() { // 00435210
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

	void __declspec(naked) ai_reassign_worker_resources_wrapper() { // 00435900
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

	void __declspec(naked) ai_auto_harvest_wrapper() { // 00433B90
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

	void __declspec(naked) ai_find_best_repair_unit_wrapper() { // 00443160
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

	void __declspec(naked) ai_worker_repair_controller_wrapper() { // 00433C70
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
		result = hooks::ai_worker_repair_controller(repairer, town);
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

	void __declspec(naked) ai_is_unit_id_unit_race_wrapper() { // 00432480
		static u32 result;
		static u16 unit_id;
		static Unit* unit;
		__asm {
			MOV unit_id, AX
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::ai_is_unit_id_unit_race(unit_id, unit);
		__asm {
			MOV EAX, result
			POPAD
			RETN
		}
	}

	void __declspec(naked) player_has_transport_wrapper() { // 00437250
		static bool result;
		static u32 player_id;
		__asm {
			MOV player_id, ECX
			PUSHAD
		}
		result = hooks::player_has_transport(player_id);
		__asm {
			POPAD
			MOVZX AL, result
			RETN
		}
	}

	void __declspec(naked) request_military_to_wrapper() { // 0043E760
		static u32 result;
		static u16* defense_ptr;
		static AiRegion* ai_region;
		static u32 b_against_air;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV b_against_air, EAX
			POP EAX
			MOV ai_region, EBX
			MOV defense_ptr, EAX
			PUSHAD
		}
		result = hooks::request_military_to(defense_ptr, ai_region, b_against_air);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	void __declspec(naked) ai_can_travel_to_region_wrapper() { // 0043A790
		static s32 result;
		static AiRegion* source;
		static AiRegion* destination;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV source, EAX
			MOV EAX, [EBP + 0x0C]
			MOV destination, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::ai_can_travel_to_region(source, destination);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}
}

namespace hooks
{
	void inject_ai_orders_hooks()
	{
		jump_patch(ai_unit_was_hit_wrapper,						0x0043F320, 1);

		jump_patch(ai_order_unit_cloaking_wrapper,				0x0043B970, 1);
		jump_patch(ai_send_suicide_wrapper,						0x0043E050, 1);
		jump_patch(ai_recalculate_region_strength_wrapper,		0x0043A390, 1);
		jump_patch(get_closest_enemy_structure_wrapper,			0x004A0F90, 1);
		jump_patch(get_region_id_from_unit_wrapper,				0x0049CB40, 2);
		jump_patch(get_best_worker_for_repair_proc_wrapper,		0x004402B0, 1);

		jump_patch(ai_military_should_wait_for_group_wrapper,	0x0043AA10, 5);
	//	jump_patch(delete_ai_town_if_needed_wrapper,			0x004347E0, 3);
	//	jump_patch(ai_can_mine_extra_wrapper,					0x00447A70, 1);
		jump_patch(ai_is_building_safe_wrapper,					0x00438910, 2);
		jump_patch(ai_are_on_connected_regions_wrapper,			0x00438E10, 1);

	//	jump_patch(get_bunker_placement_position_wrapper,		0x004442E0, 4);
	//	jump_patch(ai_get_build_location_wrapper,				0x004461B0, 1);	// TODO - fix crash in ai_manage_building_placement
		jump_patch(ai_check_military_grouping_wrapper,			0x00439550, 1);
		jump_patch(hooks::ai_remove_different_height,			0x004436C0, 3);
	//	jump_patch(attack_recent_attacker_wrapper,				0x004770E0, 2);

		jump_patch(ai_get_vespene_type_wrapper,					0x004326E0, 2);
		jump_patch(get_ai_refinery_wrapper,						0x00432B90, 0);
		jump_patch(ai_worker_controller_wrapper,				0x00435210, 0);
		jump_patch(ai_reassign_worker_resources_wrapper,		0x00435900, 1);
		jump_patch(ai_auto_harvest_wrapper,						0x00433B90, 1);
		jump_patch(ai_get_build_order_id_wrapper,				0x00434C31, 0);

		jump_patch(ai_find_best_repair_unit_wrapper,			0x00443160, 1);
	//	jump_patch(ai_worker_repair_controller_wrapper,			0x00433C70, 1);
		jump_patch(ai_expand_wrapper,							0x0045B530, 1);
		jump_patch(ai_remove_addon_placements,					0x00443CC0, 3);
		jump_patch(ai_manage_military_controller_wrapper,		0x0043D910, 1);
		jump_patch(ai_assign_region_to_slowest_unit_wrapper,	0x00436F70, 2);
	//	jump_patch(ai_respond_to_cloaked_threat_wrapper,		0x0043C580,	1);
		jump_patch(ai_try_return_home_wrapper,					0x00462EA0, 1);
		jump_patch(ai_is_unit_id_unit_race_wrapper, 			0x00432480, 4);

		jump_patch(player_has_transport_wrapper, 				0x00437250, 1);
		jump_patch(request_military_to_wrapper,					0x0043E760, 0);

		jump_patch(ai_can_travel_to_region_wrapper,				0x0043A790, 1);

		call_patch(ai_react_to_hit_town_nullcheck,				0x0043F388, 5);

		// Disable worker and Zethrokor defense gimmick
		nops(0x0043F85E, 2);
		nops(0x0043F864, 2);

		// Update gas prios
		u32 p105 = 105;
		memory_patch(0x004336E0 + 1, p105);
		u32 p104 = 104;
		memory_patch(0x004336A3 + 1, p104);

		// Make melee AI convert non building/worker regions state 6 back to 0 instead of 5
		nops(0x0043E542, 2);

		// Do not clear grouping region on attack_clear opcode (because attack_do is garbage and doesn't guarantee that and we don't want the grouping regions to be removed too quickly!!!)
		nops(0x0045BCD6, 1);
		nops(0x0045BCD7, 7);
		nops(0x0045BCE4, 5);
	}
}