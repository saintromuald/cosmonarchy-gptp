#include "hook_tools.h"
#include "SCBW/api.h"
#include "unit_ai.h"

namespace
{
	void __declspec(naked) ai_training_unit_wrapper() // 004A2830
	{
		static Unit* result;
		static Unit* parent;
		__asm
		{
			MOV result, EAX
			MOV parent, ECX
			PUSHAD
		}
		hooks::ai_training_unit(result, parent);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_create_nuke_wrapper() // 0045B7A0
	{
		static Unit* silo;
		static Unit* result;
		__asm
		{
			MOV silo, EDI
			MOV result, EAX
			PUSHAD
		}
		hooks::ai_create_nuke(silo, result);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_add_military_to_region_wrapper() { // 0043DA20
		static AiRegion* region;
		static Unit* unit;
		static u32 must_use_region;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV must_use_region, EAX
			POP EAX
			MOV unit, EBX
			MOV region, EAX
			PUSHAD
		}
		hooks::ai_add_military_to_region(region, unit, must_use_region);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) ai_training_simple_military_wrapper() // 0043E280
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
		hooks::ai_training_simple_military(result, parent);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}

	void __declspec(naked) ai_training_worker_wrapper() // 00435700
	{
		static Unit* newUnit;
		static Unit* otherUnit;
		__asm
		{
			MOV otherUnit, ECX
			MOV newUnit, EAX
			PUSHAD
		}
		hooks::ai_training_worker(newUnit, otherUnit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_training_normal_wrapper() // 00435DB0
	{
		static Unit* newUnit;
		static Unit* parentUnit;
		__asm
		{
			MOV parentUnit, ECX
			MOV newUnit, EAX
			PUSHAD
		}
		hooks::ai_training_normal(newUnit, parentUnit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) add_worker_structure_ai_wrapper() // 00433DD0
	{
		static void* result;
		static Unit* unit;
		static AiTown* town;
		__asm
		{
			MOV town, EDI
			MOV unit, EBX
			PUSHAD
		}
		result = hooks::add_worker_structure_ai(unit, town);
		__asm
		{
			POPAD
			MOV EAX, result
			RETN
		}
	}

	const u32 addUnitAiShortJump = 0x00433DED;
	void __declspec(naked) add_unit_ai_free_worker_check_jump_wrapper()
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

	void __declspec(naked) iterate_building_ai_wrapper() // 00435770
	{
		static Unit* result;
		static Unit* parent;
		__asm {
			MOV result, EAX
			MOV parent, ECX
			PUSHAD
		}
		hooks::iterate_building_ai(result, parent);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) delete_worker_ai_wrapper() // 00404470
	{
		static WorkerAi* workerAi;
		static WorkerAiArray** workerAiArray;
		__asm
		{
			MOV workerAi, EAX
			MOV workerAiArray, EDX
			PUSHAD
		}
		hooks::delete_worker_ai(workerAi, *workerAiArray);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) delete_building_ai_wrapper() // 00404500
	{
		static BuildingAi* buildingAi;
		static BuildingAiArray** buildingAiArray;
		__asm
		{
			MOV buildingAi, EAX
			MOV buildingAiArray, EDX
			PUSHAD
		}
		hooks::delete_building_ai(buildingAi, *buildingAiArray);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) delete_worker_ai_nullcheck() // 00404470
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

	void __declspec(naked) delete_building_ai_nullcheck() // 00404500
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

	void __declspec(naked) ai_remove_unit_ai_wrapper() // 004A1E50
	{
		static Unit* unit;
		static UnitAi* unitAI;
		__asm
		{
			MOV unitAI, EDX
			MOV unit, ECX
			PUSHAD
		}
		hooks::remove_unit_ai(unit, unitAI);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_remove_worker_or_building_ai_wrapper() // 00434C90
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
		result = hooks::remove_worker_or_building_ai(unit, unitAI);
		__asm
		{
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) ai_training_overlord_wrapper() // 00435770
	{
		static Unit* newUnit;
		static Unit* oldUnit;
		__asm
		{
			MOV newUnit, EAX
			MOV oldUnit, ECX
			PUSHAD
		}
		hooks::ai_training_overlord(newUnit, oldUnit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_order_lurker_burrow_wrapper() // 0043C230
	{
		static Unit* unit;
		__asm
		{
			MOV unit, EAX
			PUSHAD
		}
		hooks::ai_order_lurker_burrow(unit);
		__asm
		{
			POPAD
			RETN
		}
	}

	void __declspec(naked) create_building_ai_wrapper() { // 004044C0
		static BuildingAiArray* ai_array;
		static BuildingAi* result;
		__asm {
			MOV ai_array, ESI
			PUSHAD
		}
		result = hooks::create_building_ai(ai_array);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks
{
	void inject_unit_ai_hooks()
	{
		jump_patch(ai_training_unit_wrapper,					0x004A2830, 2);
	//	jump_patch(ai_create_nuke_wrapper,						0x0045B7A0, 0);
		jump_patch(ai_add_military_to_region_wrapper,			0x0043DA20, 1);
		jump_patch(ai_training_simple_military_wrapper,			0x0043E280, 1);
		jump_patch(ai_training_worker_wrapper,					0x00435700, 0);
	//	jump_patch(ai_training_normal_wrapper,					0x00435DB0, 0);
		jump_patch(add_worker_structure_ai_wrapper,				0x00433DD0, 2);
	//	jump_patch(add_unit_ai_free_worker_check_jump_wrapper,	0x00433DE7, 1);
	//	jump_patch(iterate_building_ai_wrapper,					0x00435770, 1);

	//	jump_patch(delete_worker_ai_wrapper,					0x00404470, 0);
	//	jump_patch(delete_building_ai_wrapper,					0x00404500, 0);
	//	jump_patch(delete_worker_ai_nullcheck,					0x00404470, 0);
	//	jump_patch(delete_building_ai_nullcheck,				0x00404500, 0);
		jump_patch(ai_remove_unit_ai_wrapper,					0x004A1E50, 0);
		jump_patch(ai_remove_worker_or_building_ai_wrapper,		0x00434C90, 0);
	//	jump_patch(ai_training_overlord_wrapper,				0x00435770, 1);
		jump_patch(ai_order_lurker_burrow_wrapper,				0x0043C230, 3);
	//	jump_patch(create_building_ai_wrapper,					0x004044C0,	4);

	//	jump_patch(hooks::find_nearest_military_or_guard_ai,	0x0043BAC0, 0);
	}
}