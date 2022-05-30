#include "hook_tools.h"
#include "SCBW/api.h"
#include "ai_requests.h"

namespace
{
	void __declspec(naked) AI_BuildTechUpgrade_Wrapper() // 00431F90
	{
		static AiTown* town;
		static s32 method;
		static s32 amount;
		static s32 id;
		static s32 priority;
		static s32 need;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV method, EAX
			MOV EAX, [EBP + 0x0C]
			MOV amount, EAX
			MOV EAX, [EBP + 0x10]
			MOV id, EAX
			MOV EAX, [EBP + 0x14]
			MOV priority, EAX
			MOV EAX, [EBP + 0x18]
			MOV need, EAX
			POP EAX
			MOV town, ESI
			PUSHAD
		}
		hooks::ai_build_tech_upgrade(town, method, amount, id, priority, need);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 20
		}
	}

	void __declspec(naked) AI_ProgressBuild_Wrapper() // 00434480
	{
		static s32 player;
		static s32* minerals;
		static s32* gas;
		static AiTown* result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			MOV EAX, [EBP + 0x0C]
			MOV minerals, EAX
			MOV EAX, [EBP + 0x10]
			MOV gas, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::ai_progress_build(player, minerals, gas);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			MOV EAX, result
			RETN 0x0C
		}
	}

	void __declspec(naked) AIManageBuildingPlacement_Wrapper() // 004465C0
	{
		static Unit* builder;
		static u32 unitId;
		static Point16 tryPosition;
		static Point16* outPosition;
		static s32 areaTiles;
		static s32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unitId, EAX
			MOV EAX, [EBP + 0x0C]
			MOV tryPosition, EAX
			MOV EAX, [EBP + 0x10]
			MOV outPosition, EAX
			MOV EAX, [EBP + 0x14]
			MOV areaTiles, EAX
			POP EAX
			MOV builder, ECX
			PUSHAD
		}
		result = hooks::ai_manage_building_placement(builder, unitId, tryPosition, outPosition, areaTiles);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x10
		}
	}

	void __declspec(naked) AI_PerformTask_Wrapper() // 004361A0
	{
		static Unit* unit;
		static char result;
		__asm
		{
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::ai_perform_task(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) AI_PlanDoMergeMorph_Wrapper() // 00435F10
	{
		static char result;
		static s32 player;
		static Unit* unit;
		static s32 boolSomething;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			MOV EAX, [EBP + 0x0C]
			MOV unit, EAX
			MOV EAX, [EBP + 0x10]
			MOV boolSomething, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::ai_plan_do_merge_morph(player, unit, boolSomething);
		__asm
		{
			POPAD
			MOV AL, result
			MOV ESP, EBP
			POP EBP
			RETN 0xC
		}
	}

	void __declspec(naked) ai_train_defensive_military_Wrapper() { // 0043F990
		static u8 player_id;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player_id, AL
			POP EAX
			PUSHAD
		}
		hooks::ai_train_defensive_military(player_id);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	void __declspec(naked) AI_Build_Wrapper() { // 004323D0
		static u32 unit_id;
		static bool need;
		static u32 count;
		static AiTown* town;
		static u8 priority;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV priority, AL
			POP EAX
			MOV town, ESI
			MOV count, EAX
			MOV need, DL
			MOV unit_id, ECX
			PUSHAD
		}
		hooks::ai_build(unit_id, need, count, town, priority);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	void __declspec(naked) AI_build_gas_building_Wrapper() { // 00433660
		static AiTown* town;
		__asm {
			MOV town, EAX
			PUSHAD
		}
		hooks::ai_build_refinery(town);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_request_guard_wrapper() { // 00448630
		static GuardAi* guard_ai;
		static u32 player_id;
		__asm {
			MOV player_id, ECX
			MOV guard_ai, EAX
			PUSHAD
		}
		hooks::ai_request_guard(guard_ai, player_id);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_region_train_attack_force_wrapper() { // 0043E670
		static AiRegion* region;
		__asm {
			MOV region, EAX
			PUSHAD
		}
		hooks::ai_region_train_attack_force(region);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_add_town_spending_request_wrapper() { // 00448400
		static u32 player;
		static u32 priority;
		static u32 unit_id;
		static AiRegion* region;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x8]
			MOV region, EAX
			POP EAX
			MOV player, EAX
			MOV priority, EDI
			MOV unit_id, ESI
			PUSHAD
		}
		hooks::ai_add_town_spending_request(player, priority, unit_id, region);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x4
		}
	}

	static u32 ai_player_need_priority(u32 unit_id) {
		if (scbw::unit_id_is_tech_tier_structure(unit_id)) {
			return 120;
		}
		if (scbw::unit_id_is_tech(unit_id)) {
			return 100;
		}
		return 80;
	}


	void __declspec(naked) ai_player_need_priority_wrapper() { // 0045BE1F
		static const u32 jmp_address = 0x0045BE26;
		static u32 priority;
		static u32 unit_id;
		__asm {
			MOV unit_id, ECX
			PUSHAD
		}
		priority = ai_player_need_priority(unit_id);
		_asm {
			POPAD
			PUSH priority
			MOV EDX, 1
			JMP jmp_address
		}
	}
}

namespace hooks
{
	void injectAIRequestsHooks()
	{
//		jump_patch(AI_BuildTechUpgrade_Wrapper,				0x00431F90, 2);
		jump_patch(AI_ProgressBuild_Wrapper,				0x00434480, 1);
//		jump_patch(AIManageBuildingPlacement_Wrapper,		0x004465C0, 3);
		jump_patch(AI_PerformTask_Wrapper,					0x004361A0, 0);
//		jump_patch(AI_PlanDoMergeMorph_Wrapper,				0x00435F10, 1);
//		jump_patch(ai_train_defensive_military_Wrapper,		0x0043F990, 1);
		jump_patch(AI_Build_Wrapper,						0x004323D0, 1);
		jump_patch(AI_build_gas_building_Wrapper,			0x00433660, 1);

		jump_patch(ai_request_guard_wrapper,				0x00448630, 0);
		jump_patch(ai_region_train_attack_force_wrapper,	0x0043E670,	4);
		jump_patch(ai_add_town_spending_request_wrapper,    0x00448400, 0);

		jump_patch(ai_player_need_priority_wrapper,			0x0045BE1F, 2);
	}
}