#include "hook_tools.h"
#include "BW/api.h"
#include "unit_ai.h"

namespace {
	void __declspec(naked) ai_training_unit_wrapper() { // 004A2830
		static Unit* result;
		static Unit* parent;
		__asm {
			MOV result, EAX
			MOV parent, ECX
			PUSHAD
		}
		hooks::ai_training_unit(result, parent);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_create_nuke_wrapper() { // 0045B7A0
		static Unit* silo;
		static Unit* result;
		__asm {
			MOV silo, EDI
			MOV result, EAX
			PUSHAD
		}
		hooks::ai_create_nuke(silo, result);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_add_military_to_region_wrapper() { // 0043DA20
		static AiRegion* region;
		static Unit* unit;
		static u32 must_use_region;
		static void* debug_caller_address;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV must_use_region, EAX
			MOV EAX, [EBP + 0x04]
			MOV debug_caller_address, EAX
			POP EAX
			MOV unit, EBX
			MOV region, EAX
			PUSHAD
		}
		hooks::ai_add_military_to_region(region, unit, must_use_region, debug_caller_address);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) ai_training_simple_military_wrapper() { // 0043E280
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
		hooks::ai_training_simple_military(result, parent);
		__asm {
			POPAD
			MOV EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x8
		}
	}

	void __declspec(naked) ai_training_worker_wrapper() { // 00435700
		static Unit* new_unit;
		static Unit* other_unit;
		__asm {
			MOV other_unit, ECX
			MOV new_unit, EAX
			PUSHAD
		}
		hooks::ai_training_worker(new_unit, other_unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_training_normal_wrapper() { // 00435DB0
		static Unit* new_unit;
		static Unit* parent;
		__asm {
			MOV parent, ECX
			MOV new_unit, EAX
			PUSHAD
		}
		hooks::ai_training_normal(new_unit, parent);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) add_worker_structure_ai_wrapper() { // 00433DD0
		static void* result;
		static Unit* unit;
		static AiTown* town;
		__asm {
			MOV town, EDI
			MOV unit, EBX
			PUSHAD
		}
		result = hooks::add_worker_structure_ai(unit, town);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) iterate_building_ai_wrapper() { // 00435770
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

	void __declspec(naked) delete_worker_ai_wrapper() { // 00404470
		static WorkerAi* worker_ai;
		static WorkerAiList* worker_ai_list;
		__asm {
			MOV worker_ai, EAX
			MOV worker_ai_list, EDX
			PUSHAD
		}
		hooks::delete_worker_ai(worker_ai, worker_ai_list);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) delete_building_ai_wrapper() { // 00404500
		static BuildingAi* building_ai;
		static BuildingAiList* building_ai_list;
		__asm {
			MOV building_ai, EAX
			MOV building_ai_list, EDX
			PUSHAD
		}
		hooks::delete_building_ai(building_ai, building_ai_list);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) delete_military_ai_wrapper() { // 004371D0
		static MilitaryAi* military_ai;
		static AiRegion* ai_region;
		__asm {
			MOV military_ai, EAX
			MOV ai_region, EDX
			PUSHAD
		}
		hooks::delete_military_ai(military_ai, ai_region);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_remove_unit_ai_wrapper() { // 004A1E50
		static Unit* unit;
		static UnitAi* unit_ai;
		__asm {
			MOV unit_ai, EDX
			MOV unit, ECX
			PUSHAD
		}
		hooks::remove_unit_ai(unit, unit_ai);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_remove_worker_or_building_ai_wrapper() { // 00434C90
		static s32 result;
		static Unit* unit;
		static b32 always_building;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV always_building, EAX
			POP EAX
			MOV unit, EDI
			PUSHAD
		}
		hooks::remove_worker_or_building_ai(unit, always_building);
		__asm {
			POPAD
			MOV EAX, 0
			MOV ESP, EBP
			POP EBP
			RETN 4
		}
	}

	void __declspec(naked) ai_training_overlord_wrapper() { // 00435770
		static Unit* newUnit;
		static Unit* oldUnit;
		__asm {
			MOV newUnit, EAX
			MOV oldUnit, ECX
			PUSHAD
		}
		hooks::ai_training_overlord(newUnit, oldUnit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) ai_order_lurker_burrow_wrapper() { // 0043C230
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::ai_order_lurker_burrow(unit);
		__asm {
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

	void __declspec(naked) is_valid_for_wrapper() { // 0043B6D0
		static bool result;
		static u32 unit_id;
		static Unit* unit;
		static u32 must_be_active;
		__asm {

			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV must_be_active, EAX
			POP EAX
			MOV unit, ESI
			MOV unit_id, ECX
			PUSHAD
		}
		result = hooks::is_valid_for(unit_id, unit, must_be_active);
		__asm {
			POPAD
			MOVZX EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x04
		}
	}

	void __declspec(naked) ai_remove_cleric_wrapper() { // 004477C0
		static Unit* unit;
		__asm {
			MOV unit, EDX
			PUSHAD
		}
		hooks::ai_remove_cleric(unit);
		__asm {
			POPAD
			RETN
		}
	}

	// PATCHES \\
	
	const u32 addUnitAiShortJump = 0x00433DED;
	void __declspec(naked) add_unit_ai_free_worker_check_jump_wrapper() {
		static WorkerAiArray* free;
		__asm {
			MOV free, EDX
			PUSHAD
		}
		if ((u32)free <= 0x100000) {
			bw::print("Corrupted pointer in AddUnitAi!");
			__asm {
				POPAD
				POP ESI
				RETN
			}
		} else {
			__asm {
				POPAD
				MOV EAX, [EDX + 0x5DC0]
				JMP addUnitAiShortJump
			}
		}
	}
}

namespace hooks {
	void inject_unit_ai_hooks() {
		jump_patch(ai_training_unit_wrapper,					0x004A2830, 2);
	//	jump_patch(ai_create_nuke_wrapper,						0x0045B7A0, 0);
		jump_patch(ai_add_military_to_region_wrapper,			0x0043DA20, 1);
		jump_patch(ai_training_simple_military_wrapper,			0x0043E280, 1);
		jump_patch(ai_training_worker_wrapper,					0x00435700, 0);
	//	jump_patch(ai_training_normal_wrapper,					0x00435DB0, 0);
		jump_patch(add_worker_structure_ai_wrapper,				0x00433DD0, 2);
	//	jump_patch(iterate_building_ai_wrapper,					0x00435770, 1);

		jump_patch(delete_worker_ai_wrapper,					0x00404470, 0);
		jump_patch(delete_building_ai_wrapper,					0x00404500, 0);
		jump_patch(delete_military_ai_wrapper,					0x004371D0, 0);
		jump_patch(remove_from_ai_structs,						0x00439D60, 0);
		jump_patch(ai_remove_unit_ai_wrapper,					0x004A1E50, 0);
		jump_patch(ai_remove_worker_or_building_ai_wrapper,		0x00434C90, 0);
	//	jump_patch(ai_training_overlord_wrapper,				0x00435770, 1);
		jump_patch(ai_order_lurker_burrow_wrapper,				0x0043C230, 3);
	//	jump_patch(create_building_ai_wrapper,					0x004044C0,	4);
	//	jump_patch(is_valid_for_wrapper,						0x0043B6D0,	5);
		jump_patch(add_guard_ai_to_unit,						0x00462960,	2);
		jump_patch(ai_remove_cleric_wrapper,					0x004477C0,	0);

	//	jump_patch(hooks::find_nearest_military_or_guard_ai,	0x0043BAC0, 0);

	//	jump_patch(add_unit_ai_free_worker_check_jump_wrapper,	0x00433DE7, 1);
	}
}