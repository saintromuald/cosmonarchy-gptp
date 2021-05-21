#include "ai_orders.h"
#include <hook_tools.h>
#include "SCBW/api.h"
namespace
{

	void __declspec(naked) AI_TrainingUnit_Wrapper() // 004A2830
	{
		static Unit* result;
		static Unit* parent;
		__asm {
			MOV result, EAX
			MOV parent, ECX
			PUSHAD
		}
		hooks::AI_TrainingUnit(result, parent);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_TrainingWorker_Wrapper() // 00435700
	{
		static char result;
		static Unit* child;
		static Unit* parent;
		__asm
		{
			MOV child, EAX
			MOV parent, ECX
			PUSHAD
		}
		result = hooks::AI_TrainingWorker(child, parent);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) AI_CreateNuke_Wrapper() // 0045B7A0
	{
		static Unit* silo;
		static Unit* result;
		__asm {
			MOV silo, EDI
			MOV result, EAX
			PUSHAD
		}
		hooks::AI_CreateNuke(silo, result);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_TrainingBroodling_Wrapper() // 0043E280
	{
		static Unit* parent;
		static Unit* result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV parent, EAX
			MOV EAX, [EBP + 0x0C]
			MOV result, EAX
			POP EAX
			PUSHAD
		}
		hooks::AI_TrainingBroodling(result, parent);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 8
		}
	}

	void __declspec(naked) AI_AddUnitAi_Wrapper() // 00433DD0
	{
		static u8 result;
		static Unit* unit;
		static AiTown* town;
		__asm {
			MOV unit, EBX
			MOV town, EDI
			PUSHAD
		}
		result = hooks::AI_AddUnitAI(unit, town);
		__asm {
			POPAD
			MOV result, AL
			RETN
		}
	}

	const u32 addUnitAiShortJump = 0x00433DED;
	void __declspec(naked) AI_AddUnitAi_FreeWorkerCheck_JmpWrapper()
	{
		static WorkerAiArray* free;
		__asm {
			MOV free, EDX
			PUSHAD
		}
		if ((u32)free <= 0x100000)
		{
			scbw::print("Corrupted pointer in AddUnitAi!");
			__asm {
				POPAD
				POP ESI
				RETN
			}
		}
		else
		{
			__asm {
				POPAD
				MOV EAX, [EDX + 0x5DC0]
				JMP addUnitAiShortJump
			}
		}

	}

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
		hooks::AI_UnitWasHit(self, attacker, mainTargetReactions, dontCallHelp);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 8
		}
	}

	void __declspec(naked) AI_IterateBuildingAI_Wrapper() // 00435770
	{
		static Unit* result;
		static Unit* parent;
		__asm {
			MOV result, EAX
			MOV parent, ECX
			PUSHAD
		}
		hooks::AI_IterateBuildingAI(result, parent);
		__asm {
			POPAD
			RETN
		}
	}

	// Attack extender

	void __declspec(naked) AI_TrainAttackForce_Wrapper()
	{ // 0043E670
		static AiRegion* region;
		static u32 result;
		__asm {
			MOV region, EAX
			PUSHAD
		}
		result = hooks::AI_TrainAttackForce(region);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) AI_GroupingForAttack_Wrapper()
	{ // 0043EEC0
		static AiRegion* region;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV region, EAX
			POP EAX
			PUSHAD
		}
		hooks::AI_GroupingForAttack(region);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) AI_GetMissingAttackForceUnits_Wrapper()
	{ // 00438050
		static u32 result;
		static AiRegion* region;
		static u32* unitIds;
		static u32 isCampaign;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV region, EAX
			MOV EAX, [EBP + 0x0C]
			MOV unitIds, EAX
			MOV EAX, [EBP + 0x10]
			MOV isCampaign, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::AI_GetMissingAttackForceUnits(region, unitIds, isCampaign);
		__asm {
			POPAD
			MOV result, EAX
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) AI_AddToAttack_Wrapper()
	{ // 00447230
		static u32 result;
		static u32 player;
		static u32 unitId;
		__asm {
			MOV result, EAX
			MOV player, ECX
			MOV unitId, EDI
			PUSHAD
		}
		hooks::AI_AddToAttack(result, player, unitId);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_AttackManager_Wrapper()
	{ // 0043ABB0
		static u32 result;
		static u32 player;
		static s32 x;
		static s32 y;
		static u32 alwaysOverwriteLastAttack;
		static u32 allowFallbackAirOnly;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV player, EAX
			MOV EAX, [EBP + 0x0C]
			MOV x, EAX
			MOV EAX, [EBP + 0x10]
			MOV y, EAX
			MOV EAX, [EBP + 0x14]
			MOV alwaysOverwriteLastAttack, EAX
			MOV EAX, [EBP + 0x18]
			MOV allowFallbackAirOnly, EAX
			POP EAX
			PUSHAD
		}
		result = hooks::AI_AttackManager(player, x, y, alwaysOverwriteLastAttack, allowFallbackAirOnly);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x14
		}
	}

	void __declspec(naked) AI_HasUnitsForAttackForce_Wrapper()
	{ // 00437F20
		static s32 result;
		static u32 player;
		__asm {
			MOV player, EDX
			PUSHAD
		}
		result = hooks::AI_HasUnitsForAttackForce(player);
		__asm {
			POPAD
			MOV result, EAX
			RETN
		}
	}

	void __declspec(naked) AI_orderUnitCloaking_Wrapper() // 0043B970
	{
		static Unit* unit;
		__asm {
			MOV unit, ESI
			PUSHAD
		}
		hooks::AI_orderUnitCloaking(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) deleteWorkerAI_Wrapper() // 00404470
	{
		static WorkerAi* result;
		static WorkerAi* workerAi;
		static WorkerAiArray* workerAiList;
		__asm
		{
			MOV workerAi, EAX
			MOV workerAiList, EDX
			PUSHAD
		}
		result = hooks::deleteWorkerAI(workerAi, workerAiList);
		__asm
		{
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
		hooks::AI_SendSuicide(argType, player);
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
			v7->town->in_battle = 1;
		}
		__asm
		{
			RETN
		}
	}
}

namespace hooks
{
	void injectAIOrdersHooks()
	{
		jmpPatch(AI_TrainingUnit_Wrapper,					0x004A2830, 2);
//		jmpPatch(AI_TrainingWorker_Wrapper,					0x00435700, 0);
//		jmpPatch(AI_CreateNuke_Wrapper,						0x0045B7A0, 0);
		jmpPatch(AI_TrainingBroodling_Wrapper,				0x0043E280, 1);
//		jmpPatch(AI_AddUnitAi_Wrapper,						0x00433DD0, 2);
		jmpPatch(AI_AddUnitAi_FreeWorkerCheck_JmpWrapper,	0x00433DE7, 1);
//		jmpPatch(AI_BuildTechUpgrade_Wrapper,				0x00431F90, 2);
//		jmpPatch(AI_UnitWasHit_Wrapper,						0x0043F320, 1);
//		jmpPatch(AI_IterateBuildingAI_Wrapper,				0x00435770, 1);

		/*
		jmpPatch(AI_TrainAttackForce_Wrapper,				0x0043E670, 4);
		jmpPatch(AI_GroupingForAttack_Wrapper,				0x0043EEC0, 4);
		jmpPatch(AI_GetMissingAttackForceUnits_Wrapper,		0x00438050, 2);
		jmpPatch(AI_AddToAttack_Wrapper,					0x00447230, 1);
		jmpPatch(AI_AttackManager_Wrapper,					0x0043ABB0, 4);
		jmpPatch(AI_HasUnitsForAttackForce_Wrapper,			0x00437F20, 4);
		*/

		jmpPatch(AI_orderUnitCloaking_Wrapper,	0x0043B970, 1);
//		jmpPatch(deleteWorkerAI_Wrapper,		0x00404470, 0);
		jmpPatch(AI_SendSuicide_Wrapper,		0x0043E050, 1);

		callPatch(AI_ReactToHit_TownNullCheck,	0x0043F388, 5);
	}
}