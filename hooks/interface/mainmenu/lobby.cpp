#include "lobby.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace hooks
{
	s32 initGame() // 004EE110
	{
		if (*multiplayer_mode)
		{
			return 0;
		}

		if (*custom_singleplayer)
		{
			if (game_data->victory_condition || game_data->starting_units || game_data->tournament_mode_enabled)
			{
				helper::switchAllAvailableComputerSlotsToOpen();
				player_table[*local_nation_id].race = selected_singleplayer_race[0];
				for (s32 player = 0; player < 8; player++)
				{
					if (selected_singleplayer_race[player + 1] != RaceId::None)
					{
						helper::setLastOpenSlotToComputer(selected_singleplayer_race[player + 1]);
					}
				}
			}

			// Convert open slots to computers (singleplayer)
			for (s32 player = 0; player < 8; player++)
			{
				if (player_table[player].type == PlayerType::OpenSlot)
				{
					player_table[player].type = PlayerType::Computer;
				}
			}
		}
		return 1;
	}

	s32 startGame(void* lobbyState, s32 player) // 00472060
	{
		lobbyState = game_state;
		u8 bootReason = 4;

		// Boot (incoming?) players if lobby is not idle
		if (*game_state != 4)
		{
			return helper::bootReason(bootReason);
		}

		// ?
		s32 result = player;
		if (player)
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
		if (*bw_is_in_replay)
		{
			for (s32 i = 0; i < 12; i++)
			{
				playerReplayWatchers[i] = player_table[i];
			}
		}

		// Start countdown
		*game_state = 5;
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
		*elapsed_time_modified = 0;
		*countdownTimeRemaining = result;

		// Convert open slots to computers (multiplayer)
		for (s32 i = 0; i < 8; i++)
		{
			if (player_table[i].type == PlayerType::OpenSlot)
			{
				player_table[i].type = PlayerType::Computer;
			}
		}

		return result;
	}
}