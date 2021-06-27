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
		hooks::AI_OrderUnitCloaking(unit);
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
		result = hooks::AI_RecalculateRegionStrength(region);
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

	void __declspec(naked) AI_CanMineExtra_Wrapper() // 00447A70
	{
		static Unit* unit;
		static bool result;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::AI_CanMineExtra(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) AI_IsBuildingSafe_Wrapper() // 00438910
	{
		static Unit* unit;
		static bool result;
		__asm
		{
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::AI_IsBuildingSafe(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) AI_AreOnConnectedRegions_Wrapper() // 00438E10
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
		result = hooks::AI_AreOnConnectedRegions(unit1, unit2);
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
		result = hooks::getBunkerPlacementPosition(placementId, regionId);
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
		static u32 unitId;
		static char* placementData;
		static u32 playerId;
		static Point16 desiredPos;
		static s32 maxRange;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV playerId, EAX
			MOV EAX, [EBP + 0x0C]
			MOV desiredPos, EAX
			MOV EAX, [EBP + 0x10]
			MOV maxRange, EAX
			POP EAX
			MOV placementData, ESI
			MOV unitId, EBX
			PUSHAD
		}
		hooks::AI_GetBuildLocation(unitId, placementData, playerId, desiredPos, maxRange);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}
}

namespace hooks
{
	void injectAIOrdersHooks()
	{
//		jmpPatch(AI_UnitWasHit_Wrapper,						0x0043F320, 1);

		jmpPatch(AI_OrderUnitCloaking_Wrapper,				0x0043B970, 1);
		jmpPatch(AI_SendSuicide_Wrapper,					0x0043E050, 1);
		jmpPatch(AI_RecalculateRegionStrength_Wrapper,		0x0043A390, 1);
		jmpPatch(getClosestEnemyStructure_Wrapper,			0x004A0F90, 1);
		jmpPatch(getRegionIdFromUnit_Wrapper,				0x0049CB40, 2);
		jmpPatch(getBestSCVForRepairProc_Wrapper,			0x004402B0, 1);

		jmpPatch(AI_MilitaryShouldWaitForGroup_Wrapper,		0x0043AA10, 5);
//		jmpPatch(deleteAITownIfNeeded_Wrapper,				0x004347E0, 3);
//		jmpPatch(AI_CanMineExtra_Wrapper,					0x00447A70, 1);
		jmpPatch(AI_IsBuildingSafe_Wrapper,					0x00438910, 2);
		jmpPatch(AI_AreOnConnectedRegions_Wrapper,			0x00438E10, 1);

//		jmpPatch(getBunkerPlacementPosition_Wrapper,		0x004442E0, 4);
//		jmpPatch(AI_GetBuildLocation_Wrapper,				0x004461B0, 1);

		callPatch(AI_ReactToHit_TownNullCheck,				0x0043F388, 5);
	}
}