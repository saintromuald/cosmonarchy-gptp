#include "sfxhooks.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>
#include <hooks/byte_operations_utils.h>

namespace hooks
{
	s32 getResponseSfx(s32 unitId, s32 a2)
	{ // 0048EA20
		s32 sound = 0;			//v2
		s32 whatStart;			//v3
		s32 whatEnd;			//v4
		s32 rand;				//v6
		s32 result;

		if (!(units_dat::GroupFlags[unitId].isBuilding))
		{		// original check was unitId <= 0x69, now checks for existance of annoyed lines
			sound = scbw::get_unit_data(UnitsDatEntries::SndPssFirst, unitId);
			if ((s16)sound)
			{
				sound = (u16)sound;
				if (a2 <= (u16)scbw::get_unit_data(UnitsDatEntries::SndPssLast, unitId) - (u16)sound + 1)
				{
					if (a2)
					{
						return sound + a2 - 1;
					}
				}
				else
				{
					*selectionSoundCounter = 0;
				}
			}
		}
		whatStart = scbw::get_unit_data(UnitsDatEntries::SndWhtFirst, unitId);
		whatEnd = (u16)scbw::get_unit_data(UnitsDatEntries::SndWhtLast, unitId);
		if (whatStart == whatEnd)
		{
			*lastRandSFX = scbw::get_unit_data(UnitsDatEntries::SndWhtFirst, unitId);
			result = whatStart;
		}
		else
		{
			rand = whatStart + (helper::rand_func() >> 8) % (whatEnd - whatStart + 1);
			if (rand == *lastRandSFX && ++rand > whatEnd)
				rand = whatStart;
			*lastRandSFX = rand;
			result = rand;
		}
		return result;
	}

	s32 getRandYesSFX(s32 a1)
	{ // 0048EAB0
		u16 v1; // ax
		s32 result; // eax

		if (!(units_dat::GroupFlags[a1].isBuilding) && (v1 = scbw::get_unit_data(UnitsDatEntries::SndYesFirst, a1)) != 0)
			result = helper::randGetSFXBetween((s32)scbw::get_unit_data(UnitsDatEntries::SndYesLast, a1), (s32)v1);
		else
			result = 0;
		return result;
	}
}