#include "sfxhooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <hooks/byte_operations_utils.h>

namespace hooks
{
	u32 get_unit_response_id(s32 unit_id, s32 last_sound_id) // 0048EA20
	{
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
		u32 whatStart = scbw::get_unit_data(UnitsDatEntries::SndWhtFirst, unit_id);
		u32 whatEnd = scbw::get_unit_data(UnitsDatEntries::SndWhtLast, unit_id);
		s32 result;
		if (whatStart == whatEnd)
		{
			*last_random_sfx = scbw::get_unit_data(UnitsDatEntries::SndWhtFirst, unit_id);
			result = whatStart;
		}
		else
		{
			u32 rand = whatStart + (helper::rand_func() >> 8) % (whatEnd - whatStart + 1);
			if (rand == *last_random_sfx && ++rand > whatEnd)
				rand = whatStart;
			*last_random_sfx = rand;
			result = rand;
		}
		return result;
	}

	s32 get_random_yes_unit_response(s32 unit_id) // 0048EAB0
	{
		s32 result = 0;
		u32 unit_data = scbw::get_unit_data(UnitsDatEntries::SndYesFirst, unit_id);
		if (!(units_dat::group_flags[unit_id].is_building) && unit_data)
		{
			result = helper::get_rand_sfx_between((s32)scbw::get_unit_data(UnitsDatEntries::SndYesLast, unit_id), unit_data);
		}
		return result;
	}
}