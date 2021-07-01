#include "hook_tools.h"
#include "SCBW/api.h"
#include "unit_ai.h"

namespace
{
	void __declspec(naked) AI_TrainingUnit_Wrapper() // 004A2830
	{
		static Unit* result;
		static Unit* parent;
		__asm
		{
			MOV result, EAX
			MOV parent, ECX
			PUSHAD
		}
		hooks::AI_TrainingUnit(result, parent);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_CreateNuke_Wrapper() // 0045B7A0
	{
		static Unit* silo;
		static Unit* result;
		__asm
		{
			MOV silo, EDI
			MOV result, EAX
			PUSHAD
		}
		hooks::AI_CreateNuke(silo, result);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_TrainingSimpleMilitary_Wrapper() // 0043E280
	{
		static Unit* parent;
		static Unit* result;
		__asm
		{
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
		hooks::AI_TrainingSimpleMilitary(result, parent);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 8
		}
	}

	void __declspec(naked) AI_TrainingWorker_Wrapper() // 00435700
	{
		static Unit* newUnit;
		static Unit* otherUnit;
		__asm
		{
			MOV newUnit, EAX
			MOV otherUnit, ECX
			PUSHAD
		}
		hooks::AI_TrainingWorker(newUnit, otherUnit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_TrainingNormal_Wrapper() // 00435DB0
	{
		static Unit* newUnit;
		static Unit* parentUnit;
		__asm
		{
			MOV parentUnit, ECX
			MOV newUnit, EAX
			PUSHAD
		}
		hooks::AI_TrainingNormal(newUnit, parentUnit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_AddWorkerStructureAI_Wrapper() // 00433DD0
	{
		static u8 result;
		static Unit* unit;
		static AiTown* town;
		__asm
		{
			MOV unit, EBX
			MOV town, EDI
			PUSHAD
		}
		result = hooks::AI_AddWorkerStructureAI(unit, town);
		__asm
		{
			POPAD
			MOV result, AL
			RETN
		}
	}

	const u32 addUnitAiShortJump = 0x00433DED;
	void __declspec(naked) AI_AddUnitAi_FreeWorkerCheck_JmpWrapper()
	{
		static WorkerAiArray* free;
		__asm
		{
			MOV free, EDX
			PUSHAD
		}
		if ((u32)free <= 0x100000)
		{
			scbw::print("Corrupted pointer in AddUnitAi!");
			__asm
			{
				POPAD
				POP ESI
				RETN
			}
		}
		else
		{
			__asm
			{
				POPAD
				MOV EAX, [EDX + 0x5DC0]
				JMP addUnitAiShortJump
			}
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

	void __declspec(naked) deleteWorkerAI_Wrapper() // 00404470
	{
		static WorkerAi* workerAi;
		static WorkerAiArray** workerAiArray;
		__asm
		{
			MOV workerAi, EAX
			MOV workerAiArray, EDX
			PUSHAD
		}
		hooks::deleteWorkerAI(workerAi, *workerAiArray);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) deleteBuildingAI_Wrapper() // 00404500
	{
		static BuildingAi* buildingAi;
		static BuildingAiArray** buildingAiArray;
		__asm
		{
			MOV buildingAi, EAX
			MOV buildingAiArray, EDX
			PUSHAD
		}
		hooks::deleteBuildingAI(buildingAi, *buildingAiArray);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) deleteWorkerAI_nullcheck() // 00404470
	{
		static const u32 back_jump = 0x00404475;
		static const u32 cmp_jump = 0x0040447A;
		static WorkerAi* ai;
		static WorkerAiArray** aiArray;
		__asm
		{
			MOV ai, EAX
			MOV aiArray, EDX
			PUSHAD
		}
		if (!ai)
		{
			__asm
			{
				POPAD
				MOV EAX, 0
				RETN
			}
		}
		if ((*aiArray)->worker_ais[0].link.next == ai)
		{
			__asm
			{
				POPAD
				JMP cmp_jump
			}
		}
		__asm
		{
			POPAD
			JMP back_jump
		}
	}

	void __declspec(naked) deleteBuildingAI_nullcheck() // 00404500
	{
		static const u32 back_jump = 0x00404505;
		static const u32 cmp_jump = 0x0040450A;
		static BuildingAi* ai;
		static BuildingAiArray** aiArray;
		__asm
		{
			MOV ai, EAX
			MOV aiArray, EDX
			PUSHAD
		}
		if (!ai)
		{
			__asm
			{
				POPAD
				MOV EAX, 0
				RETN
			}
		}
		if ((*aiArray)->building_ais[0].link.next == ai)
		{
			__asm
			{
				POPAD
				JMP cmp_jump
			}
		}
		__asm
		{
			POPAD
			JMP back_jump
		}
	}

	void __declspec(naked) AI_RemoveUnitAI_Wrapper() // 004A1E50
	{
		static Unit* unit;
		static UnitAi* unitAI;
		__asm
		{
			MOV unitAI, EDX
			MOV unit, ECX
			PUSHAD
		}
		hooks::AI_RemoveUnitAI(unit, unitAI);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) AI_RemoveWorkerOrBuildingAI_Wrapper() // 00434C90
	{
		static s32 result;
		static Unit* unit;
		static UnitAi* unitAI;
		__asm
		{
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV unitAI, EAX
			POP EAX
			MOV unit, EDI
			PUSHAD
		}
		result = hooks::AI_RemoveWorkerOrBuildingAI(unit, unitAI);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}
}

namespace hooks
{
	void injectUnitAIHooks()
	{
		jmpPatch(AI_TrainingUnit_Wrapper,					0x004A2830, 2);
//		jmpPatch(AI_CreateNuke_Wrapper,						0x0045B7A0, 0);
		jmpPatch(AI_TrainingSimpleMilitary_Wrapper,			0x0043E280, 1);
		jmpPatch(AI_TrainingWorker_Wrapper,					0x00435700, 0);
//		jmpPatch(AI_TrainingNormal_Wrapper,					0x00435DB0, 0);
//		jmpPatch(AI_AddWorkerStructureAI_Wrapper,			0x00433DD0, 2);
		jmpPatch(AI_AddUnitAi_FreeWorkerCheck_JmpWrapper,	0x00433DE7, 1);
//		jmpPatch(AI_IterateBuildingAI_Wrapper,				0x00435770, 1);

//		jmpPatch(deleteWorkerAI_Wrapper,					0x00404470, 0);
//		jmpPatch(deleteBuildingAI_Wrapper,					0x00404500, 0);
//		jmpPatch(deleteWorkerAI_nullcheck,					0x00404470, 0);
//		jmpPatch(deleteBuildingAI_nullcheck,				0x00404500, 0);
		jmpPatch(AI_RemoveUnitAI_Wrapper,					0x004A1E50, 0);
		jmpPatch(AI_RemoveWorkerOrBuildingAI_Wrapper,		0x00434C90, 0);
	}
}