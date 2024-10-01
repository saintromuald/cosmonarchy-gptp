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

	void __declspec(naked) play_sound_from_unit_wrapper() { // 0048F440
		static SoundId::Enum sound_id;
		static Unit* unit;
		static b32 play;
		static s32 min_volume;
		__asm {
			PUSH EBP
			MOV EBP, ESP
			PUSH EAX
			MOV EAX, [EBP + 0x08]
			MOV play, EAX
			MOV EAX, [EBP + 0x0C]
			MOV min_volume, EAX
			POP EAX
			MOV unit, EAX
			MOV sound_id, EBX
			PUSHAD
		}
		hooks::play_sound_from_unit(sound_id, unit, (bool)play, min_volume);
		__asm {
			POPAD
			MOV EAX, 0
			MOV ESP, EBP
			POP EBP
			RETN 0x08
		}
	}

	void __declspec(naked) play_sound_at_pos_wrapper() { // 0048F440
		static SoundId::Enum sound_id;
		static Point16 pos;
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
}

namespace hooks {
	void inject_sfx_hooks() {
		jump_patch(get_unit_response_id_wrapper,			0x0048EA20, 1);
		jump_patch(play_yes_unit_response_wrapper,			0x0048F4D0, 3);
		jump_patch(get_random_yes_unit_response_wrapper,	0x0048EAB0, 0);
		jump_patch(play_unit_ready_sound_wrapper,			0x0048F440, 3);
		jump_patch(play_sound_from_unit_wrapper,			0x0048ED50, 0);
		jump_patch(play_sound_at_pos_wrapper,				0x0048EC10, 0);
	}
}