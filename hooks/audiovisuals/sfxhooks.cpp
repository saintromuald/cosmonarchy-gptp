#include "sfxhooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <hooks/byte_operations_utils.h>

namespace hooks
{
	// Hooked to allow more unit ids to have annoyed lines.
	// Updated 22 July 2022 to remove annoyed lines. -Pr0nogo
	u32 get_unit_response_id(s32 unit_id, s32 last_sound_id) // 0048EA20
	{
		/*
		if (!(units_dat::group_flags[unit_id].is_building))	// original check was unit_id <= DisruptionWeb, now checks for existance of annoyed lines
		{
			u32 sound = scbw::get_unit_data(UnitsDatEntries::SndPssFirst, unit_id);
			if (sound)
			{
				if (last_sound_id <= scbw::get_unit_data(UnitsDatEntries::SndPssLast, unit_id) - sound + 1)
				{
					if (last_sound_id)
					{
						return sound + last_sound_id - 1;
					}
				}
				else
				{
					*selection_sound_counter = 0;
				}
			}
		}
		*/

		u32 what_start = scbw::get_unit_data(UnitsDatEntries::SndWhtFirst, unit_id);
		u32 what_end = scbw::get_unit_data(UnitsDatEntries::SndWhtLast, unit_id);
		u32 result = 0;
		if (what_start == what_end)
		{
			*g_last_random_sfx_id = scbw::get_unit_data(UnitsDatEntries::SndWhtFirst, unit_id);
			result = what_start;
		}
		else
		{
			u32 rand = what_start + (helper::rand_func() >> 8) % (what_end - what_start + 1);
			if (rand == *g_last_random_sfx_id && ++rand > what_end)
			{
				rand = what_start;
			}
			*g_last_random_sfx_id = rand;
			result = rand;
		}
		return result;
	}
	
	// Hooked to allow for aggressive order unit responses. -Pr0nogo
	void play_yes_unit_response(Unit* self) // 0048F4D0
	{
		if (!*use_quiet_sounds)
		{
			*g_place_building_parent_unit = 0;
			if (!*g_is_in_replay && self->player == *g_local_nation_id)
			{
				if (!self->is_frozen())
				{
					u32 last_rand = *g_last_random_sfx_id;
					u32 rand_response = get_random_yes_unit_response(self->id, self);
					bool can_play = (g_options_list->flags >> 1) & 1;
					if (helper::play_sound_from_unit(rand_response, self, can_play, 99))
					{
						helper::do_unit_event_notification(self, 17, sfx_dat::PortraitDuration[rand_response] + 1000 * (*(u32*)&g_sound_duration[rand_response] / 0x3E8u), 0, rand_response);
					}
					else
					{
						*g_last_random_sfx_id = last_rand;
					}
				}
			}
		}
	}

	// Hooked to allow more unit ids to process passive order unit responses.
	// Updated 22 July 2022 with a lame implementation of aggressive order unit responsese. -Pr0nogo
	s32 get_random_yes_unit_response(s32 unit_id, Unit* unit) // 0048EAB0
	{
		s32 result = 0;
		// Technically this will not work if a unit has attack sounds but no order passive sounds... -Pr0nogo
		u32 unit_data = scbw::get_unit_data(UnitsDatEntries::SndYesFirst, unit_id);
		if (!(units_dat::group_flags[unit_id].is_building) && unit_data)
		{
			if (unit->order_target.unit && !scbw::is_allied_to(unit->player, unit->order_target.unit->player)) {
				switch (unit_id)
				{
					case UnitId::ProtossHierophant:
					{
						result = scbw::rand_between(SoundId::HierophantOrderAggressive01, SoundId::HierophantOrderAggressive04);
					} break;
					default:
					{
						result = helper::get_random_sfx_between((s32)scbw::get_unit_data(UnitsDatEntries::SndYesLast, unit_id), unit_data);
					} break;
				}
			}
			else {
				result = helper::get_random_sfx_between((s32)scbw::get_unit_data(UnitsDatEntries::SndYesLast, unit_id), unit_data);
			}
		}
		return result;
	}
}