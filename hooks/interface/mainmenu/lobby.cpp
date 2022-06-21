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
				g_player_table[*g_local_nation_id].race = selected_singleplayer_race[0];
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
				if (g_player_table[player].type == PlayerType::OpenSlot)
				{
					g_player_table[player].type = PlayerType::Computer;
				}
			}
		}
		return 1;
	}

	s32 startGame(void* lobbyState, s32 player) // 00472060
	{
		lobbyState = g_game_state;
		u8 bootReason = 4;

		// Boot (incoming?) players if lobby is not idle
		if (*g_game_state != 4)
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
		if (*g_is_in_replay)
		{
			for (s32 i = 0; i < 12; i++)
			{
				g_player_replay_watchers[i] = g_player_table[i];
			}
		}

		// Start countdown
		*g_game_state = 5;
		*g_countdown_timer_interval = 24;
		if (2 * *g_provider_latency_calls + 4 >= 24)
		{
			*g_countdown_timer_interval = 2 * *g_provider_latency_calls + 4;
		}

		// Set timers
		helper::setTimerFunc();
		s32 tickCount = helper::getTickCount(); // CRASH
		*g_countdown_timer_tick_count = tickCount;
		*g_countdown_timer_tick_count_2 = tickCount;
		result = tickCount - 120000;

		*g_is_in_game = 1;
		*elapsed_time_modified = 0;
		*g_countdown_time_remaining = result;

		// Convert open slots to computers (multiplayer)
		for (s32 i = 0; i < 8; i++)
		{
			if (g_player_table[i].type == PlayerType::OpenSlot)
			{
				g_player_table[i].type = PlayerType::Computer;
			}
		}

		return result;
	}
}