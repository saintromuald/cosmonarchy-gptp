#include "sfxhooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <hooks/byte_operations_utils.h>

namespace hooks
{
	s32 getResponseSfx(s32 unit_id, s32 a2)
	{ // 0048EA20
		s32 sound = 0;			//v2
		s32 whatStart;			//v3
		s32 whatEnd;			//v4
		s32 rand;				//v6
		s32 result;

		if (!(units_dat::group_flags[unit_id].is_building)) // original check was unit_id <= 0x69, now checks for existance of annoyed lines
		{
			sound = units_dat::sound_annoyed_first[unit_id];
			if ((s16)sound)
			{
				sound = (u16)sound;
				if (a2 <= (u16)units_dat::sound_annoyed_last[unit_id] - (u16)sound + 1)
				{
					if (a2)
					{
						return sound + a2 - 1;
					}
				}
				else
				{
					*selection_sound_counter = 0;
				}
			}
		}
		whatStart = units_dat::sound_what_first[unit_id];
		whatEnd = units_dat::sound_what_last[unit_id];
		if (whatStart == whatEnd)
		{
			*last_random_sfx = units_dat::sound_what_first[unit_id];
			result = whatStart;
		}
		else
		{
			rand = whatStart + (helper::rand_func() >> 8) % (whatEnd - whatStart + 1);
			if (rand == *last_random_sfx && ++rand > whatEnd)
				rand = whatStart;
			*last_random_sfx = rand;
			result = rand;
		}
		return result;
	}

	s32 getRandYesSFX(s32 a1) // 0048EAB0
	{
		u16 v1; // ax
		s32 result; // eax

		if (!(units_dat::group_flags[a1].is_building) && (v1 = units_dat::sound_yes_first[a1]) != 0)
		{
			result = helper::randGetSFXBetween(units_dat::sound_yes_last[a1], (s32)v1);
		}
		else
		{
			result = 0;
		}
		return result;
	}
}