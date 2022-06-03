#include "target_related.h"
#include "hook_tools.h"
#include "SCBW/structures.h"

namespace
{
	void __declspec(naked) genericRightClickUnitProc2_Wrapper() // 00455E00
	{
		static s32 result;
		static Unit* unit;
		static s32 a2;
		static s16 a3;
		static Unit* target;
		static s16 unitID;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV a3, AX
			MOV EAX, [EBP + 0xC]
			MOV target, EAX
			MOV EAX, [EBP + 0x10]
			MOV unitID, AX
			POP EAX
			MOV a2, EDX
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::genericRightClickUnitProc2(unit, a2, a3, target, unitID);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0xC
		}
	}

	void __declspec(naked) CMDRECV_HoldPosition_Wrapper() // 004C20C0
	{
		static Unit* result;
		static s32 a1;
		__asm
		{
			MOV a1, EDI
			PUSHAD
		}
		result = hooks::CMDRECV_HoldPosition(a1);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unitCanSeeCloakedTarget_Wrapper() // 00401D60
	{
		static Unit* target;
		static Unit* unit;
		static u32 result;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			MOV target, EAX
			PUSHAD
		}
		result = hooks::unitCanSeeCloakedTargetHook(target, unit);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) isTargetVisible_Wrapper() // 004E5E30
	{
		static u32 result;
		static Unit* target;
		static Unit* unit;
		__asm
		{
			MOV target, EAX
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::isTargetVisibleHook(target, unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) isUnitVisible_Wrapper() // 004E5DB0
	{
		static u32 result;
		static Unit* unit;
		__asm
		{
			MOV unit, ESI
			PUSHAD
		}
		result = hooks::isUnitVisibleHook(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) checkUnitVisibility_Wrapper() // 00443390
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::checkUnitVisibilityHook(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) isUnitCloaking_Wrapper() // 004916E0
	{
		static s32 result;
		static Unit* unit;
		__asm
		{
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::isUnitCloakingHook(unit);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) unitCanAttackTarget_Wrapper() // 00476730
	{
		static u32 result;
		static Unit* target;
		static Unit* unit;
		static u32 a3;
		__asm
		{
			PUSH EBP
			PUSH EAX
			MOV EBP, ESP
			MOV EAX, [EBP + 0x08]
			MOV a3, EAX
			POP EAX
			MOV target, EBX
			MOV unit, ESI
			POP EBP
			PUSHAD
		}
		result = hooks::unit_can_attack_target(target, unit, a3);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN 4
		}
	}

	void __declspec(naked) orderAttackBehavior_Wrapper() // 00478540
	{
		static bool result;
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		result = hooks::orderAttackBehavior(unit);
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) AI_AttackUnit_Wrapper() // 0043FFD0
	{
		static Unit* unit;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unit, EAX
			POP EAX
			PUSHAD
		}
		hooks::AI_AttackUnit(unit);
		__asm
		{
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) return_interceptor_if_cannot_attack_wrapper() { // 00465810
		static Unit* hangar_unit;
		static s32 result;
		__asm {
			MOV hangar_unit, EAX
			PUSHAD
		}
		result = hooks::return_interceptor_if_cannot_attack(hangar_unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void injectTargetHooks()
	{
		// Removes can not attack status checks from some update functions, it was done by iquare - not exactly sure what was the intention. Currently it doesn't seem to do anything. -Veeq7
//		nops(0x004EB176, 8);
//		nops(0x004EB316, 2);

//		jump_patch(genericRightClickUnitProc2_Wrapper,			0x00455E00, 1);
//		jump_patch(genericRightClickUnitProc2_Wrapper,			0x004C20C0, 0);
//		jump_patch(unitCanSeeCloakedTarget_Wrapper,				0x00401D60, 4);
//		jump_patch(isTargetVisible_Wrapper,						0x004E5E30, 4);
//		jump_patch(isUnitVisible_Wrapper,						0x004E5DB0, 1);
		jump_patch(checkUnitVisibility_Wrapper,					0x00443390, 3);
//		jump_patch(isUnitCloaking_Wrapper,						0x004916E0, 0);
		jump_patch(unitCanAttackTarget_Wrapper,					0x00476730, 5);
//		jump_patch(orderAttackBehavior_Wrapper,					0x00478540, 0);
		jump_patch(AI_AttackUnit_Wrapper,						0x0043FFD0, 1);
		jump_patch(return_interceptor_if_cannot_attack_wrapper,	0x00465810, 1);
	}

	void injectTargetRelatedHooks()
	{
		injectTargetHooks();
	}
}