#include "trigger_hooks.h"
#include "hook_tools.h"

// HOOKS \\

static bool execute_trigger_condition(TriggerCondition* condition) {
	static b32 result;
	const void* address = g_trigger_condition_list[condition->condition_type];
	__asm {
		PUSHAD
		MOV ECX, condition
		CALL address
		MOV result, EAX
		POPAD
	}
	return result;
}

// Hooked to check custom conditions. -Pr0nogo
s32 execute_trigger_conditions(TriggerVector* trigger_vector) { // 00489200
	for (u32 i = 0; i < 64; ++i) {
		TriggerAction* action = &trigger_vector->trigger.actions[i];
		if (!(action->flags & 2)) {
			if (action->action_type == 47) {
				if (!parse_comment_condition(action)) {
					return false;
				}
			}
		}
	}

	for (u32 i = 0; i < 16; ++i) {
		TriggerCondition* condition = &trigger_vector->trigger.conditions[i];
		if (!(condition->flags & 2)) {
			if (!condition->condition_type) {
				return true;
			}

			if (!execute_trigger_condition(condition)) {
				return false;
			}
		}
	}
	return true;
}

// HOOK INJECTION \\

void __declspec(naked) execute_trigger_conditions_wrapper() { // 00489200
	static b32 result;
	static TriggerVector* trigger_vector;
	__asm {
		MOV trigger_vector, EAX
		PUSHAD
	}
	result = execute_trigger_conditions(trigger_vector);
	__asm {
		POPAD
		MOV EAX, result
		RETN
	}
}

void inject_trigger_system_hooks() {
	jump_patch(execute_trigger_conditions_wrapper, 0x00489200, 1);
}