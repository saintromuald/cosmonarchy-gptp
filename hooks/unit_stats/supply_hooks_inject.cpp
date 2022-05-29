#include "supply_hooks.h"
#include <hook_tools.h>
#include "SCBW/api.h"

namespace
{
	void __declspec(naked) getSuppliesAvailable_Wrapper()
	{
		static u8 playerId;
		static s32 race;
		static u32 result;
		__asm
		{
			MOV race, ECX
			MOV playerId, AL
			PUSHAD
		}
		result = hooks::getSuppliesAvailable(playerId, race);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) getSuppliesRemaining_Wrapper()
	{
		static u8 playerId;
		static u16 unitId;
		static s32 result;
		__asm
		{
			MOV unitId, DI
			MOV playerId, BL
			PUSHAD
		}
		result = hooks::getSuppliesRemaining(playerId, unitId);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) getSuppliesMax_Wrapper()
	{
		static u16 unitId;
		static u8 playerId;
		static s32 result;
		__asm
		{
			MOV playerId, CL
			MOV unitId, AX
			PUSHAD
		}
		result = hooks::getSuppliesMax(unitId, playerId);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) getSuppliesUsed_Wrapper()
	{
		static s32 race;
		static u8 playerId;
		static s32 result;
		__asm
		{
			MOV playerId, CL
			MOV race, EAX
			PUSHAD
		}
		result = hooks::getSuppliesUsed(race, playerId);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) initializeUnitCounts_Wrapper()
	{
		static char result;
		__asm
		{
			PUSHAD
		}
		result = hooks::initializeUnitCounts();
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) StatRes_MainProc_Wrapper()
	{
		static BinDlg* dialog;
		__asm
		{
			MOV dialog, ECX
			PUSHAD
		}
		hooks::StatRes_MainProc(dialog);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) getSuppliesTemp_Wrapper()
	{
		static s32 result;
		__asm
		{
			PUSHAD
		}
		result = hooks::getSuppliesTemp();
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) redrawSupplyInfo_Wrapper()
	{
		static bool result;
		__asm
		{
			PUSHAD
		}
		result = hooks::redrawSupplyInfo();
		__asm
		{
			POPAD
			MOV AL, result
			RETN
		}
	}

	void __declspec(naked) drawSupplyUnitStatus_Wrapper()
	{
		static BinDlg* dialog;
		__asm
		{
			MOV dialog, EAX
			PUSHAD
		}
		hooks::drawSupplyUnitStatus(dialog);
		__asm
		{
			POPAD
			RETN
		}
	}

	b32 __stdcall hasSuppliesForUnitWrapper(u8 playerId, u16 unitId, b32 canShowErrorMessage)
	{
		if (hooks::hasSuppliesForUnitHook(playerId, unitId, canShowErrorMessage != 0))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	void __declspec(naked) incrementUnitScores_Wrapper() // 00488BF0
	{
		static Unit* unit;
		static s32 count;
		static s16 result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV count, EAX
			POP EAX
			MOV unit, EAX
			PUSHAD
		}
		hooks::incrementUnitScores(unit, count);
		__asm {
			POPAD
			MOV EAX, 0
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) incrementUnitScoresEx_Wrapper() // 00488D50
	{
		static s32 count;
		static Unit* unit;
		static s32 updateMorphMergeUnitScore;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			MOV count, ECX
			MOV unit, EDI
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV updateMorphMergeUnitScore, EAX
			POP EAX
			PUSHAD
		}
		hooks::increment_unit_scores_ex(count, unit, updateMorphMergeUnitScore);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void injectSupplyHooks()
	{
		// disable red supply text
		u8 data[] = {
			0x33, 0xC0 // xor eax, eax
		};
		memoryPatch(0x004E53FC, data, sizeof(data));

		jump_patch(getSuppliesAvailable_Wrapper,	0x00488900, 1);
		jump_patch(getSuppliesRemaining_Wrapper,	0x004889D0, 1);
		jump_patch(getSuppliesMax_Wrapper,		0x00488990, 4);
		jump_patch(getSuppliesUsed_Wrapper,		0x004888C0, 1);
		jump_patch(initializeUnitCounts_Wrapper,	0x00488F90, 0);
		jump_patch(StatRes_MainProc_Wrapper,		0x004E5640, 4);
		jump_patch(getSuppliesTemp_Wrapper,		0x004246D0, 1);
		jump_patch(redrawSupplyInfo_Wrapper,		0x00425230, 4);
		jump_patch(drawSupplyUnitStatus_Wrapper,	0x00427540, 1); // Currently broken, supply used in unit status display won't show properly for zerg and protoss units
		jump_patch(hasSuppliesForUnitWrapper,		0x0042CF70);
		jump_patch(incrementUnitScores_Wrapper,	0x00488BF0, 2);
		jump_patch(incrementUnitScoresEx_Wrapper,	0x00488D50, 1);
	}
}