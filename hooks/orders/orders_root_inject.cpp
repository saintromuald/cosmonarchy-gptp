#include "orders_root.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) orders_id_cases_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_id_cases(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) secondary_orders_root_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::secondary_orders_root(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) main_orders_root_wrapper() {
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::main_orders_root(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) do_next_queued_order_wrapper() { // 00475000
		static Unit* unit;
		__asm {
			MOV unit, ECX
			PUSHAD
		}
		hooks::do_next_queued_order(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) orders_stay_in_range_wrapper() { // 0047C4F0
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_stay_in_range(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) partial_order_handling_wrapper() { // 004EBED0
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::partial_order_handling(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) order_to_move_to_target_wrapper() { // 004EB980
		static s32 result;
		static Unit* target;
		static Unit* unit;
		__asm {
			MOV target, EAX
			MOV unit, ECX
			PUSHAD
		}
		result = hooks::order_to_move_to_target(target, unit);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) orders_nuke_launch_wrapper() { // 00464730
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::orders_nuke_launch(unit);
		__asm {
			POPAD
			RETN
		}
	}

	// PATCHES \\

	const u32 perform_orders_E9E67 = 0x004E9E67;
	void __declspec(naked) perform_orders_patch_E9E61() {
		__asm {
			MOV		ESI, EAX
			MOV		byte ptr[ESI + 0x85], 0
			MOVZX	EAX, byte ptr[ESI + 0x4E] // unit->main_order_state
			JMP		perform_orders_E9E67
		}
	}

	const u32 setUnitStatTxtErrorMsg_8CCB9 = 0x0048CCB9;
	void __declspec(naked) set_unit_stat_txt_error_message_patch_8CCB4() {
		__asm {
			MOV		ECX, ds: [0x006D0F14] // g_is_in_replay
			TEST	ECX, ECX
			JNE		offset is_in_replay
			PUSH	218
			JMP		setUnitStatTxtErrorMsg_8CCB9

			is_in_replay:
			RET
		}
	}
}

namespace hooks {
	void inject_orders_root_hooks() {
	//	jump_patch(orders_id_cases_wrapper,			0x004EBED0,	5);
	//	jump_patch(secondary_orders_root_wrapper,	0x004EC170,	1);
		jump_patch(main_orders_root_wrapper,		0x004EC4D0,	3);
		jump_patch(do_next_queued_order_wrapper,	0x00475000,	1);
		jump_patch(orders_stay_in_range_wrapper,	0x0047C4F0,	0);
		jump_patch(partial_order_handling_wrapper,	0x004EBED0,	5);
		jump_patch(order_to_move_to_target_wrapper,	0x004EB980,	1);
		jump_patch(orders_nuke_launch_wrapper,		0x00464730, 1);

		jump_patch(perform_orders_patch_E9E61,					0x004E9E61, 1);	
		jump_patch(set_unit_stat_txt_error_message_patch_8CCB4,	0x0048CCB4);	// Prevents "unit's waypoint list is full" in replays. -DF
	}
}