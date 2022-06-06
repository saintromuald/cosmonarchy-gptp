#include "lobby.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace hooks
{
	s32 initGame() // 004EE110
	{
		if (*multiPlayerMode)
		{
			return 0;
		}

		if (*customSingleplayer)
		{
			if (gameData->victoryCondition || gameData->startingUnits || gameData->tournamentModeEnabled)
			{
				helper::switchAllAvailableComputerSlotsToOpen();
				playerTable[*LOCAL_NATION_ID].race = selectedSingleplayerRace[0];
				for (s32 playerId = 0; playerId < 8; playerId++)
				{
					if (selectedSingleplayerRace[playerId + 1] != RaceId::None)
					{
						helper::setLastOpenSlotToComputer(selectedSingleplayerRace[playerId + 1]);
					}
				}
			}

			// Convert open slots to computers (singleplayer)
			for (s32 playerId = 0; playerId < 8; playerId++)
			{
				if (playerTable[playerId].type == PlayerType::OpenSlot)
				{
					playerTable[playerId].type = PlayerType::Computer;
				}
			}
		}
		return 1;
	}

	s32 startGame(void* lobbyState, s32 playerId) // 00472060
	{
		lobbyState = gameState;
		u8 bootReason = 4;

		// Boot (incoming?) players if lobby is not idle
		if (*gameState != 4)
		{
			return helper::bootReason(bootReason);
		}

		// ?
		s32 result = playerId;
		if (playerId)
		{
			return result;
		}

		// Lard
		if (!helper::isAuthedMap(lobbyState))
		{
			bootReason = 9;
			return helper::bootReason(bootReason);
		}

		// Replay-specific
		if (*is_in_replay)
		{
			for (s32 i = 0; i < 12; i++)
			{
				playerReplayWatchers[i] = playerTable[i];
			}
		}

		// Start countdown
		*gameState = 5;
		*countdownTimerInterval = 24;
		if (2 * *provider_LatencyCalls + 4 >= 24)
		{
			*countdownTimerInterval = 2 * *provider_LatencyCalls + 4;
		}

		// Set timers
		helper::setTimerFunc();
		s32 tickCount = helper::getTickCount(); // CRASH
		*countdownTimeTickCount = tickCount;
		*countdownTimeTickCount_0 = tickCount;
		result = tickCount - 120000;

		*isInGame = 1;
		*elapstedTimeModifier = 0;
		*countdownTimeRemaining = result;

		// Convert open slots to computers (multiplayer)
		for (s32 i = 0; i < 8; i++)
		{
			if (playerTable[i].type == PlayerType::OpenSlot)
			{
				playerTable[i].type = PlayerType::Computer;
			}
		}

		return result;
	}
}