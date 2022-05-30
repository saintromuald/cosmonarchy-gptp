#include "hook_tools.h"
#include "SCBW/api.h"
#include "attack_extender.h"

namespace
{
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

	void __declspec(naked) AI_GroupingForAttack_Wrapper() // 0043EEC0
	{
		static AiRegion* region;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV region, EAX
			POP EAX
			PUSHAD
		}
		hooks::AI_GroupingForAttack(region);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) AI_GetMissingAttackForceUnits_Wrapper() // 00438050
	{
		static u32 result;
		static AiRegion* region;
		static u32* unitIds;
		static u32 isCampaign;
		__asm
		{
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
		__asm
		{
			POPAD
			MOV result, EAX
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	void __declspec(naked) AI_AddToAttack_Wrapper() // 00447230
	{
		static u32 result;
		static u32 player;
		static u32 unitId;
		__asm
		{
			MOV result, EAX
			MOV player, ECX
			MOV unitId, EDI
			PUSHAD
		}
		hooks::AI_AddToAttack(result, player, unitId);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_AttackManager_Wrapper() // 0043ABB0
	{
		static u32 result;
		static u32 player;
		static s32 x;
		static s32 y;
		static u32 alwaysOverwriteLastAttack;
		static u32 allowFallbackAirOnly;
		__asm
		{
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
		result = hooks::ai_attack_manager(player, x, y, alwaysOverwriteLastAttack, allowFallbackAirOnly);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x14
		}
	}

	void __declspec(naked) AI_HasUnitsForAttackForce_Wrapper() // 00437F20
	{
		static s32 result;
		static u32 player;
		__asm
		{
			MOV player, EDX
			PUSHAD
		}
		result = hooks::AI_HasUnitsForAttackForce(player);
		__asm
		{
			POPAD
			MOV result, EAX
			RETN
		}
	}

	void __declspec(naked) AI_RemoveFromAttack_Wrapper() // 00447040
	{
		static s32 unitId;
		static s32 player;
		__asm
		{
			MOV unitId, EAX
			MOV player, EDX
			PUSHAD
		}
		hooks::AI_RemoveFromAttack(unitId, player);
		__asm
		{
			POPAD
			RETN
		}
	}
}

namespace hooks
{
	void injectAttackExtenderHooks()
	{
		/*
		jump_patch(AI_TrainAttackForce_Wrapper,				0x0043E670, 4);
		jump_patch(AI_GroupingForAttack_Wrapper,				0x0043EEC0, 4);
		jump_patch(AI_GetMissingAttackForceUnits_Wrapper,		0x00438050, 2);
		jump_patch(AI_AddToAttack_Wrapper,					0x00447230, 1);
		jump_patch(AI_AttackManager_Wrapper,					0x0043ABB0, 4);
		jump_patch(AI_HasUnitsForAttackForce_Wrapper,			0x00437F20, 4);
		jump_patch(AI_RemoveFromAttack_Wrapper,				0x00447040, 0)
		*/
	}
}