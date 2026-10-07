#pragma once
#include "BW/structures.h"

namespace hooks
{
	u32 get_unit_response_id(s32 a1, s32 last_sound_id);										// 0048EA20
	void play_yes_unit_response(Unit* self);													// 0048F4D0
	u32 get_random_yes_unit_response_local(u32 unit_id);										// 0048EAB0
	void play_unit_ready_sound(Unit* unit);														// 0048F440
	bool play_sound_from_unit(SoundId::Enum sound_id, Unit* unit, bool play, s32 min_volume);	// 0048ED50
	void play_sound_at_pos(SoundId::Enum sound_id, v2u16 pos, bool play, s32 min_volume);		// 0048EC10
	void __stdcall reset_race_audio_state(bool unused);											// 0048FB40
	s32 get_sfx_volume_from_screen_distance(s32 x, s32 y);										// 0048E8D0
	s32 get_sfx_pan_from_screen_distance(s32 x);												// 0048E850

	// Internal funcs:
	u32 get_random_yes_unit_response_internal(Unit* unit, UnitResponseContext context);
	void play_yes_unit_response_internal(Unit* self, UnitResponseContext context);

	void inject_sfx_hooks();
}