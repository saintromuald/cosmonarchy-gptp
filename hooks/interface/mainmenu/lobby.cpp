#include "lobby.h"
#include <SCBW/api.h>
#include <SCBW/structures.h>
#include <SCBW/scbwdata.h>

namespace hooks
{
	s32 init_game() // 004EE110
	{
		if (*multiplayer_mode)
		{
			return 0;
		}

		if (*custom_singleplayer)
		{
			if (game_data->victory_condition || game_data->starting_units || game_data->tournament_mode_enabled)
			{
				helper::switch_all_available_computer_slots_to_open();
				g_player_table[*g_local_nation_id].race = selected_singleplayer_race[0];
				for (s32 player = 0; player < 8; player++)
				{
					if (selected_singleplayer_race[player + 1] != RaceId::None)
					{
						helper::set_last_open_slot_to_computer(selected_singleplayer_race[player + 1]);
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

	s32 start_game(void* lobby_state, s32 player) // 00472060
	{
		lobby_state = g_game_state;
		u8 boot_reason = 4;

		// Boot (incoming?) players if lobby is not idle
		if (*g_game_state != 4)
		{
			return helper::boot_reason(boot_reason);
		}

		// ?
		s32 result = player;
		if (player)
		{
			return result;
		}

		// Lard
		if (!helper::is_authorized_map(lobby_state))
		{
			boot_reason = 9;
			return helper::boot_reason(boot_reason);
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
		helper::set_timer_func();
		s32 tick_count = helper::get_tick_count(); // CRASH
		*g_countdown_timer_tick_count = tick_count;
		*g_countdown_timer_tick_count_2 = tick_count;
		result = tick_count - 120000;

		*g_is_in_game = 1;
		*g_elapsed_time_modified = 0;
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