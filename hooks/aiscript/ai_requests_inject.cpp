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
		hooks::AI_BuildTechUpgrade(town, method, amount, id, priority, need);
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
		result = hooks::AI_ProgressBuild(player, minerals, gas);
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
		result = hooks::AIManageBuildingPlacement(builder, unitId, tryPosition, outPosition, areaTiles);
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
		result = hooks::AI_PerformTask(unit);
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
		result = hooks::AI_PlanDoMergeMorph(player, unit, boolSomething);
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
}

namespace hooks
{
	void injectAIRequestsHooks()
	{
//		jmpPatch(AI_BuildTechUpgrade_Wrapper,				0x00431F90, 2);
		jmpPatch(AI_ProgressBuild_Wrapper,					0x00434480, 1);
//		jmpPatch(AIManageBuildingPlacement_Wrapper,			0x004465C0, 3);
		jmpPatch(AI_PerformTask_Wrapper,					0x004361A0, 0);
//		jmpPatch(AI_PlanDoMergeMorph_Wrapper,				0x00435F10, 1);
//		jmpPatch(ai_train_defensive_military_Wrapper,		0x0043F990, 1);
	}
}