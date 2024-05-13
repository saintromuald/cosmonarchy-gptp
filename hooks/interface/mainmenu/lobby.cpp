#include "lobby.h"
#include <BW/api.h>
#include <BW/structures.h>
#include <BW/bwdata.h>
#include "time.h"

namespace hooks
{
	s32 init_game() // 004EE110
	{
		if (*g_multiplayer_mode) {
			return 0;
		}

		if (*g_custom_singleplayer) {
			if (game_data->victory_condition || game_data->starting_units || game_data->use_standard_unit_stats)
			{
				*g_initial_random_seed = time(0);
				g_last_random_number = *g_initial_random_seed;
				srand(*g_initial_random_seed);

				helper::switch_all_available_computer_slots_to_open();
				g_player_table[*g_local_nation_id].race = g_selected_singleplayer_race[0];
				for (s32 player = 0; player < 8; player++) {
					if (g_selected_singleplayer_race[player + 1] != RaceId::None)
					{
						helper::set_last_open_slot_to_computer(g_selected_singleplayer_race[player + 1]);
					}
				}
			}

			// Convert open slots to computers (singleplayer)
			for (s32 player = 0; player < 8; player++) {
				if (g_player_table[player].type == PlayerType::OpenSlot) {
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
		if (*g_game_state != 4) {
			return helper::boot_reason(boot_reason);
		}

		// ?
		s32 result = player;
		if (player) {
			return result;
		}

		// Lard
		if (!helper::is_authorized_map(lobby_state)) {
			boot_reason = 9;
			return helper::boot_reason(boot_reason);
		}

		// Replay-specific
		if (g_is_in_replay) {
			for (s32 i = 0; i < 12; i++) {
				g_player_replay_watchers[i] = g_player_table[i];
			}
		}

		// Start countdown
		*g_game_state = 5;
		*g_countdown_timer_interval = 24;
		if (2 * *g_provider_latency_calls + 4 >= 24) {
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
		/*
		for (s32 i = 0; i < 8; i++)
		{
			if (g_player_table[i].type == PlayerType::OpenSlot)
			{
				g_player_table[i].type = PlayerType::Computer;
			}
		}
		*/

		return result;
	}

	// Hooked to triage and hopefully fix singleplayer randomness. -Pr0nogo
	// Warning: not sure if Player in GPTP is the same as PlayerStruct in IDA!
	Player* init_singleplayer_custom(BinDlg* dialog) // 004AD850
	{
		if (dialog->control_type) {
			dialog = dialog->parent;
		}
		BinDlg* child_dlg = dialog->child_dialog;
		BinDlg* dropdown = 0;
		if (child_dlg) {
			while (child_dlg->index != 28) {
				child_dlg = child_dlg->next;
				if (!child_dlg) {
					// originally GOTO label 6
					dropdown = 0;
					break;
				}
			}
			dropdown = child_dlg;
		} else { // originally label 6
			dropdown = 0;
		}
		Player* result = (Player*)helper::is_melee_game(*g_selected_game_type, *g_selected_game_type_param, *g_selected_game_type); // first arg was SBYTE1(*g_selected_game_type)
		if (result) {
			g_selected_singleplayer_race[1] = 0x7070707; // these were both written
			g_selected_singleplayer_race[5] = 0x7070707; // to using *(_DWORD *)&
			g_custom_singleplayer[0] = 1;
			if (dropdown->initial_version_0x46.text_pos.x) {
				if (dropdown->initial_version_0x46.text_pos.y != -1) {
					g_selected_singleplayer_race[0] = dropdown->child_dialog->bounds.x * dropdown->initial_version_0x46.text_pos.y; // I don't think this is correct
				}
			}
			result = (Player*)dropdown;
			if (dropdown) {
				char* race = &g_selected_singleplayer_race[1];
				do {
					if (result->name[13] & 8) {
						if (result[1].name[23]) {
							u8 race_something = result[2].id;
							if (race_something != -1) {
								*race++ = result[1].name[19] + 4 * race_something; // result[1].name[19] was originally prefixed with *(_BYTE*)(*(_DWORD*)&
							}
						}
					}
					result = (Player*)result->id;
				} while (result);
			}
		} else if (dropdown->flags & 8) {
			if (dropdown->initial_version_0x46.text_pos.x && dropdown->initial_version_0x46.text_pos.y != -1) {
				g_player_table[*g_local_nation_id].race = dropdown->child_dialog->bounds.x * dropdown->initial_version_0x46.text_pos.y; // I don't think this is correct
				g_is_in_game_loop = 1;
				result = helper::randomize_player_slot();
				g_is_in_game_loop = 0;
			} else {
				g_player_table[*g_local_nation_id].race = 6;
				g_is_in_game_loop = 1;
				result = helper::randomize_player_slot();
				g_is_in_game_loop = 0;
			}
		}
		return result;
	}

	// Hooked to play a sound on player join. -Pr0nogo
	void on_player_join(u32 player_id, BinDlg* dialog, u32 data_1, u32 data_2) // 00470720
	{
		if (!player_id) {
			player_id = *(u32*)(data_2 + 1);
			if (player_id < PLAYABLE_PLAYER_COUNT && player_id != *g_local_storm_id) {
				helper::play_sound_global(SoundId::Lobby_PlayerJoin);

				char player_name[28];
				player_id = helper::snet_get_player_name(player_id, (int)player_name, 25);
				if (player_id) {
					player_id = player_name[0];
					if (player_name[0]) {
						const char* string = g_network_table_DF->get_string(74);
						char final_string[148];
						sprintf_s(final_string, 145, string, player_name);
						helper::print_lobby_string(8, (int)final_string);
						player_id = helper::is_multiplayer_mode_bnet();
						if (player_id) {
							sprintf_s(final_string, 145, "/astat %s", player_name);
							player_id = helper::snet_send_server_chat_command((u32)final_string);
						}
					}
				}
			}
		}
	}
}