#include "ai_orders.h"
#include <hook_tools.h>
#include "SCBW/api.h"

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

	void __declspec(naked) AI_OrderUnitCloaking_Wrapper() // 0043B970
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

	void __declspec(naked) AI_RecalculateRegionStrength_Wrapper()
	{
		static u32 result;
		static AiRegion* region;
		__asm
		{
			MOV region, ESI
			PUSHAD
		}
		result = hooks::AI_RecalculateRegionStrength(region);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
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
		result = hooks::AI_progressBuild(player, minerals, gas);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			MOV EAX, result
			RETN 0x0C
		}
	}

	void __declspec(naked) getClosestEnemyStructure_Wrapper() // 004A0F90
	{
		static s32 x;
		static s32 y;
		static s32 playerId;
		static Unit* result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV playerId, EAX
			POP EAX
			MOV y, ECX
			MOV x, EAX
			PUSHAD
		}
		result = hooks::getClosestEnemyStructure(x, y, playerId);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x4
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

	void __declspec(naked) getRegionIdFromUnit_Wrapper() // 0049CB40
	{
		static Unit* unit;
		static s16 result;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::getRegionIdFromUnit(unit);
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
		result = hooks::getBestSCVForRepairProc(repairman, repairTarget);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) AI_MilitaryShouldWaitForGroup_Wrapper() // 0043AA10
	{
		static b32 result;
		static Unit* unit;
		__asm
		{
			MOV unit, EBX
			PUSHAD
		}
		result = hooks::AI_MilitaryShouldWaitForGroup(unit);
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
		result = hooks::deleteAITownIfNeeded(town);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) Ai_CanMineExtra_Wrapper() // 00447A70
	{
		static Unit* unit;
		static bool result;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::Ai_CanMineExtra(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}
}

namespace hooks
{
	void injectAIOrdersHooks()
	{
//		jmpPatch(AI_BuildTechUpgrade_Wrapper,				0x00431F90, 2);
//		jmpPatch(AI_UnitWasHit_Wrapper,						0x0043F320, 1);

		jmpPatch(AI_OrderUnitCloaking_Wrapper,				0x0043B970, 1);
		jmpPatch(AI_SendSuicide_Wrapper,					0x0043E050, 1);
		jmpPatch(AI_RecalculateRegionStrength_Wrapper,		0x0043A390, 1);
		jmpPatch(AI_ProgressBuild_Wrapper,					0x00434480, 1);
		jmpPatch(getClosestEnemyStructure_Wrapper,			0x004A0F90, 1);
//		jmpPatch(AIManageBuildingPlacement_Wrapper,			0x004465C0, 3);
		jmpPatch(getRegionIdFromUnit_Wrapper,				0x0049CB40, 2);

		jmpPatch(getBestSCVForRepairProc_Wrapper,			0x004402B0, 1);
		jmpPatch(AI_MilitaryShouldWaitForGroup_Wrapper,		0x0043AA10, 5);
		jmpPatch(deleteAITownIfNeeded_Wrapper,				0x004347E0, 3);
//		jmpPatch(Ai_CanMineExtra_Wrapper,					0x00447A70, 1);

		callPatch(AI_ReactToHit_TownNullCheck,				0x0043F388, 5);
	}
}