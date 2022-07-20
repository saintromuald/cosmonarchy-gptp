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

//enum DatSoundFlags {
//	DatSoundFlags_PermanentlyLoaded    = 0x01,
//	DatSoundFlags_UnitResponse         = 0x02,
//	DatSoundFlags_SkipFreeing          = 0x04,
//	DatSoundFlags_BufferCreationQueued = 0x08,
//	DatSoundFlags_OneAtATime           = 0x10,
//	DatSoundFlags_NeverPreempt         = 0x20,
//};

// Hooked to extend sound channels. -Pr0nogo
s32 play_transmission_from_location(u32 sound_id, s32 volume, s32 pan, Unit* unit) { // 004BBFD0
	if (!g_direct_sound_interface->lpVtbl || !g_options_list->sfx) {
		return 0;
	}
	
	u32 flags = sfx_dat::Flags[sound_id];
	bool has_flag_10 = flags & 0x10;
	if (unit && flags & 2 || has_flag_10) {
		for (u32 i = 0; i < 8; ++i) {
			SoundChannel* sound_channel = &g_sound_channels[i];
			if (sound_channel->ds_buffer) {
				if (has_flag_10 || sound_channel->sound_id == sound_id || (sound_channel->flags & 2 && sound_channel->unit_id == unit->id)) {
					u32 status = 0;
					if (!(*(s32(__stdcall**)(void*, u32*))(*(s32*) sound_channel->ds_buffer + 36))(sound_channel->ds_buffer, &status)) { // GetStatus
						if (status == DSBSTATUS_PLAYING) {
							return 0;
						}
						if (sound_channel->release_when_not_used) {
							(*(void(__stdcall**)(void*))(*(s32*)sound_channel->ds_buffer + 8))(sound_channel->ds_buffer); // Release
						}
						sound_channel->ds_buffer = 0;
					}
				}
			}
		}
	}
	if (flags & 8) {
		return 0;
	}
	s32 unit_id = unit ? unit->id : -1;
	if (g_sound_data[sound_id].ds_buffer) {
		sfx_dat::Flags[sound_id] |= 4;
		return helper::play_loaded_sound((u32)&g_sound_data[16 * sound_id], volume, pan, sound_id, unit_id, flags, 0, sfx_dat::Priority[sound_id]);
	}
	helper::queue_sound(sound_id, volume, pan, unit_id);
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
		MOV unit, EAX
		MOV EAX, [EBP + 0x0C]
		MOV pan, EAX
		MOV EAX, [EBP + 0x10]
		MOV volume, EAX
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

// (u32 sound_id, s32 volume, s32 pan, Unit* unit) // 004BBFD0

void inject_trigger_system_hooks() {
	jump_patch(execute_trigger_conditions_wrapper,			0x00489200, 1);
	jump_patch(iscript_play_sound_wrapper,					0x004D6610, 2);
	jump_patch(play_transmission_from_location_wrapper,	0x004D6610, 0);
}