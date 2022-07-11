#include "trigger_hooks.h"
#include "trigger_system.h"
#include "trigger_sound.h"
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

void iscript_play_sound(Image* image, u32 sound_id) { // 004D6610
	bool replace = false;
	for (u32 i = 0; i < sound_replacements_count; ++i) {
		SoundReplacement* sound_replacement = &sound_replacements[i];
		if ((sound_replacement->any_sound || sound_replacement->old_sound_id == sound_id) &&
			sound_replacement->image_id  == image->id &&
			sound_replacement->animation == image->animation &&
			sound_replacement->player	 == image->parent_sprite->player) {

			play_sound(sound_replacement->new_sound, get_sound_volume_at_pos(image->parent_sprite->position));

			if (sound_replacement->replace) {
				replace = true;
			}
		}
	}

	if (!replace) {
		helper::play_sound_at_pos(sound_id, image->parent_sprite->position, 1, 0);
	}
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

void __declspec(naked) iscript_play_sound_wrapper() { // 004D6610
	static Image* image;
	static u32 sound_id;
	__asm {
		MOV sound_id, EBX
		MOV image, EAX
		PUSHAD
	}
	iscript_play_sound(image, sound_id);
	__asm {
		POPAD
		RETN
	}
}

void inject_trigger_system_hooks() {
	jump_patch(execute_trigger_conditions_wrapper,	0x00489200, 1);
	jump_patch(iscript_play_sound_wrapper,			0x004D6610, 2);
}