#include <BW/api.h>
#include "sfxhooks.h"
#include <hook_tools.h>

namespace {
	void __declspec(naked) get_unit_response_id_wrapper() { // 0048EA20
		static s32 unit_id;
		static s32 last_sound_id;
		static s32 result;
		__asm {
			MOV unit_id, EAX
			MOV last_sound_id, EDX
			PUSHAD
		}
		result = hooks::get_unit_response_id(unit_id, last_sound_id);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) play_yes_unit_response_wrapper() { // 0048F4D0
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::play_yes_unit_response(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) get_random_yes_unit_response_wrapper() { // 0048EAB0
		static u32 unit_id;
		static u32 result;
		__asm {
			MOV unit_id, ECX
			PUSHAD
		}
		result = hooks::get_random_yes_unit_response_local(unit_id);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) play_unit_ready_sound_wrapper() { // 0048F440
		static Unit* unit;
		__asm {
			MOV unit, EAX
			PUSHAD
		}
		hooks::play_unit_ready_sound(unit);
		__asm {
			POPAD
			RETN
		}
	}

	void __declspec(naked) play_sound_from_unit_wrapper() { // 0048ED50
		static SoundId::Enum sound_id;
		static Unit* unit;
		static b32 play;
		static s32 min_volume;
		static bool result;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV play, EAX
			MOV EAX, [EBP + 0x0C]
			MOV min_volume, EAX
			POP EAX
			MOV unit, ESI
			MOV sound_id, EBX
			PUSHAD
		}
		result = hooks::play_sound_from_unit(sound_id, unit, (bool)play, min_volume);
		__asm {
			POPAD
			MOVZX EAX, result
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) play_sound_at_pos_wrapper() { // 0048EC10
		static SoundId::Enum sound_id;
		static v2u16 pos;
		static b32 play;
		static s32 min_volume;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV pos, EAX
			MOV EAX, [EBP + 0x0C]
			MOV play, EAX
			MOV EAX, [EBP + 0x10]
			MOV min_volume, EAX
			POP EAX
			MOV sound_id, EBX
			PUSHAD
		}
		hooks::play_sound_at_pos(sound_id, pos, (bool)play, min_volume);
		__asm {
			POPAD
			MOV ESP, EBP
			POP EBP
			RETN 0x0C
		}
	}

	// 0048E8D0 / 0048E850 wrappers: marshal register arguments from sound entry points (and 004E5D50).
	void __declspec(naked) get_sfx_volume_from_screen_distance_wrapper() { // 0048E8D0
		static s32 sound_x;
		static s32 sound_y;
		static s32 result;
		__asm {
			MOV sound_x, ECX
			MOV sound_y, EAX
			PUSHAD
		}
		result = hooks::get_sfx_volume_from_screen_distance(sound_x, sound_y);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}

	void __declspec(naked) get_sfx_pan_from_screen_distance_wrapper() { // 0048E850
		static s32 sound_x;
		static s32 result;
		__asm {
			MOV sound_x, EAX
			PUSHAD
		}
		result = hooks::get_sfx_pan_from_screen_distance(sound_x);
		__asm {
			POPAD
			MOV EAX, result
			RETN
		}
	}
}

namespace hooks {
	void inject_sfx_hooks() {
		jump_patch(get_unit_response_id_wrapper,			0x0048EA20, 1);
		jump_patch(play_yes_unit_response_wrapper,			0x0048F4D0, 3);
		jump_patch(get_random_yes_unit_response_wrapper,	0x0048EAB0, 0);
		jump_patch(play_unit_ready_sound_wrapper,			0x0048F440, 3);
		jump_patch(play_sound_from_unit_wrapper,			0x0048ED50, 0);
		jump_patch(play_sound_at_pos_wrapper,				0x0048EC10, 0);
		jump_patch(reset_race_audio_state,					0x0048FB40);
		jump_patch(get_sfx_volume_from_screen_distance_wrapper,	0x0048E8D0, 1);
		jump_patch(get_sfx_pan_from_screen_distance_wrapper,	0x0048E850, 1);
	}
}