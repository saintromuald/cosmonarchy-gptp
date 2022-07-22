#include "trigger_hooks.h"
#include "trigger_system.h"
#include "trigger_sound.h"
#include "hook_tools.h"
#include "hooks/limits/sfx_extender.h"

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
	Sprite* sprite = image->parent_sprite;
	SpriteExt* sprite_ext = get_sprite_ext(image->parent_sprite);
	bool replace = false;
	for (u32 i = 0; i < sound_replacements_count; ++i) {
		SoundReplacement* sound_replacement = &sound_replacements[i];

		Unit* unit = sprite_ext->owner;
		if (unit && sprite->main_graphic == image &&
			(sound_replacement->any_sound || sound_replacement->old_sound_id == sound_id) &&
			(sound_replacement->any_animation || sound_replacement->animation == image->animation) &&
			unit->id < UnitId::Count && sound_replacement->unit_ids.set[unit->id] &&
			unit->player < PlayerLimit && sound_replacement->players.set[unit->player]) {

			play_sound(sound_replacement->new_sound, get_sound_volume_at_pos(sprite->position), helper::get_sfx_pan_from_screen_distance(sprite->position.x));

			if (sound_replacement->replace) {
				replace = true;
			}
		}
	}

	if (!replace) {
		helper::play_sound_at_pos(sound_id, sprite->position, 1, 0);
	}
}

// Hooked to extend sound channels. -Pr0nogo
s32 play_transmission_from_location(u32 sound_id, s32 volume, s32 pan, Unit* unit) { // 004BBFD0
	if (!g_direct_sound_interface->lpVtbl || !g_options_list->sfx) {
		return false;
	}
	
	if (SFXDAT_SoundFile[sound_id]) {
		char buffer[256] = {};
		snprintf(buffer, 256, "sound\\%s", SFXDAT_SoundFile[sound_id]);
		Sound* sound = get_sound(buffer);
		if (sound) {
			u32 flags = SFXDAT_Flags[sound_id];
			if (flags & DatSoundFlags_OneAtATime) {
				DWORD status = 0;
				if (sound->ds_buffer) {
					sound->ds_buffer->GetStatus(&status);
					if (status == DSBSTATUS_PLAYING) {
						return false;
					}
				}
			}

			if (flags & DatSoundFlags_UnitResponse && unit) {
				for (u32 i = 0; i < MaxDupChannels; ++i) {
					DupChannel* dup_channel = &dup_channels[i];
					if (!dup_channels[i].ds_buffer) {
						continue;
					}
					if (dup_channels[i].unit_id != unit->id) {
						continue;
					}
					DWORD status = 0;
					dup_channels[i].ds_buffer->GetStatus(&status);
					if (status == DSBSTATUS_PLAYING) {
						return false;
					}
				}
			}

			s32 unit_id = -1;
			if (flags & DatSoundFlags_UnitResponse && unit) {
				unit_id = unit->id;
			}
			if (play_sound(sound, volume, pan, unit_id)) {
				return true;
			}
		}
	}
	return false;
}

void free_sound_buffer(u32 sound_id) {
	SoundData* sound_data = &SFXDAT_SoundData[sound_id];
	IDirectSoundBuffer* ds_buffer = sound_data->ds_buffer;
	if (!ds_buffer) {
		return;
	}
	for (u32 i = 0; i < 8; ++i) {
		if (g_sound_channels[i].ds_buffer == ds_buffer) {
			g_sound_channels[i].ds_buffer = 0;
		}
	}
	ds_buffer->Release();
	sound_data->ds_buffer = 0;
	if (SFXDAT_Flags[sound_id] & DatSoundFlags_PermanentlyLoaded) {
		return;
	}
	*g_current_sound_memory -= sound_data->wav_data_size;
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

void __declspec(naked) play_transmission_from_location_wrapper() { // 004BBFD0
	static s32 result;
	static u32 sound_id;
	static s32 volume;
	static s32 pan;
	static Unit* unit;
	__asm {
		PUSH EBP
		MOV EBP, ESP
		PUSH EAX
		MOV EAX, [EBP + 0x08]
		MOV volume, EAX
		MOV EAX, [EBP + 0x0C]
		MOV pan, EAX
		MOV EAX, [EBP + 0x10]
		MOV unit, EAX
		POP EAX
		MOV sound_id, EAX
		PUSHAD
	}
	result = play_transmission_from_location(sound_id, volume, pan, unit);
	__asm {
		POPAD
		MOV EAX, result
		MOV ESP, EBP
		POP EBP
		RETN 0xC
	}
}

void __declspec(naked) free_sound_buffer_wrapper() { // 004BB8E0
	static u32 sound_id;
	__asm {
		MOV sound_id, EBX
		PUSHAD
	}
	free_sound_buffer(sound_id);
	__asm {
		POPAD
		RETN
	}
}

void inject_trigger_system_hooks() {
	jump_patch(execute_trigger_conditions_wrapper,			0x00489200, 1);
	jump_patch(iscript_play_sound_wrapper,					0x004D6610, 2);
	jump_patch(play_transmission_from_location_wrapper,		0x004BBFD0, 0);
	jump_patch(free_sound_buffer_wrapper,					0x004BB8E0, 1);
}