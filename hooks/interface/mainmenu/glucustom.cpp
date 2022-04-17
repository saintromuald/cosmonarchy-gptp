#include "glucustom.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace hooks
{
	void displayEstablishingShot() // 004D9200, hooked by X405
	{
		// This is the original original function which loads the establishing shots in campaigns:
		if (!*multiPlayerMode && !(*CHEAT_STATE & CheatFlags::NoGlues) && *campaignIndex)
		{
			for (s32 i = 0; i < 65; i++)
			{
				if (campaignMissions[i].missionIndex == *campaignIndex)
				{
					helper::loadInitCreditsBIN(campaignMissions[i].missionName);
				}
			}
		}

		// This addition enables establishing shots in custom scenarios:
		else if (*campaignIndex == 0 && CurrentMapFileName)
		{
			helper::SFileOpenArchive(CurrentMapFileName, 0, 0, mapArchiveHandle);
			void* handle;
			bool establishingShotExists = helper::SFileOpenFileEx(*mapArchiveHandle, "rez\\est.txt", 0, &handle);
			if (handle)
			{
				helper::SFileCloseFile(handle);
			}

			if (establishingShotExists)
			{
				helper::loadInitCreditsBIN("est");
			}

			if (*mapArchiveHandle)
			{
				helper::SFileCloseArchive(*mapArchiveHandle);
			}
		}
	}
}