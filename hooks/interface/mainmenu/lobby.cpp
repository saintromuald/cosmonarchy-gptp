#include "lobby.h"

#include "BW/ui.h"
#include "BW/api.h"
#include "BW/structures.h"
#include "BW/bwdata.h"
#include "hooks/race_randomization.h"
#include "storm/storm.h"
#include "config.h"

#include <time.h>

namespace hooks {
bool init_game() { // 004EE110
    if (g_multiplayer_mode) {
        return 0;
    }

    if (g_custom_singleplayer) {
        if (game_data->victory_condition != VictoryCondition::Default || game_data->starting_units != StartingUnits::MapUnits || game_data->use_standard_unit_stats) {
            *g_initial_random_seed = time(0);
            g_last_random_number = *g_initial_random_seed;
            srand(*g_initial_random_seed);

            helper::switch_all_available_computer_slots_to_open();
            g_player_table[g_local_nation_id].race = g_selected_races[0];

            for (auto i = 1; i < 8; i++) {
                if (g_selected_races[i] != RaceId::None) {
                    helper::set_last_open_slot_to_computer(g_selected_races[i]);
                }
            }
        }

        for (auto& player : g_player_table) {
            if (player.type == PlayerType::OpenSlot) player.type = PlayerType::Computer;
        }
    }
    return 1;
}

void start_game(s32 player) { // 00472060
    u8 boot_reason = 4;

    // Boot (incoming?) players if lobby is not idle
    if (g_game_state != GameState::InLobby) {
        helper::boot_reason(boot_reason);
        return;
    }

    // I believe player is named wrong -Jesse
    if (player) {
        return;
    }

    if (!helper::is_authorized_map()) {
        boot_reason = 9;
        helper::boot_reason(boot_reason);
        return;
    }

    if (g_is_in_replay) {
        for (s32 i = 0; i < 12; i++) {
            g_player_replay_watchers[i] = g_player_table[i];
        }
    }

    // Start countdown
    g_game_state = GameState::GameStarting;
    g_countdown_timer_interval = 24;
    if ((2 * g_provider_turns_in_transit) + 4 >= 24) {
        g_countdown_timer_interval = 2 * g_provider_turns_in_transit + 4;
    }

    // Set timers
    helper::set_timer_func();
    s32 tick_count = helper::get_tick_count(); // CRASH
    g_countdown_timer_tick_count = tick_count;
    g_countdown_time_remaining = g_countdown_timer_tick_count - 120000;
    g_countdown_timer_tick_count_2 = tick_count;

    g_is_in_game = true;
    g_elapsed_time_modified = 0;

    return;
}

// Hooked to triage and hopefully fix singleplayer randomness. -Pr0nogo
// Refactored -Jesse
void init_singleplayer_custom(BinDlg* dialog) { // 004AD850
    if (dialog->control_type != DialogControlType::DialogBoxDC) {
        dialog = dialog->parent;
    }

    auto children = get_children_by_index(dialog, GluCustomControls::PlayerRace1);
    if (children.empty()) return;
    BinDlg* player_race_combo = children[0];



    auto melee = bw::is_melee_game(*g_selected_game_type, *g_selected_game_type_param);
    if (melee) {
        for (auto& race : g_selected_races | std::views::drop(1)) {
            race = RaceId::None;
        }

        g_custom_singleplayer = true;
        if (player_race_combo->ComboBox0x46.item_count) {
            auto selection_index = player_race_combo->ComboBox0x46.selection_index;
            if (selection_index != -1) {
                g_selected_races[0] = (RaceId::Enum)player_race_combo->combobox_output_array_u32[selection_index];
            }
        }

        auto index = 1;
        for (auto cpu_race_combo = player_race_combo->next; cpu_race_combo; cpu_race_combo = cpu_race_combo->next) {
            auto selection_index = cpu_race_combo->ComboBox0x46.selection_index;
            if ((cpu_race_combo->flags & 8) && cpu_race_combo->ComboBox0x46.item_count && selection_index != -1) {
                g_selected_races[index] = (RaceId::Enum)cpu_race_combo->combobox_output_array_u32[selection_index];
            }
            index++;
        }
    } else if (player_race_combo->flags & 8) {
        auto selection_index = player_race_combo->ComboBox0x46.selection_index;
        if (player_race_combo->ComboBox0x46.item_count && selection_index != -1) {
            g_player_table[g_local_nation_id].race = (RaceId::Enum)player_race_combo->combobox_output_array_u32[selection_index];
            g_is_in_game_loop = 1;
            auto& randomizer = RaceRandomizer::instance();
            randomizer.process_randomization();
            g_is_in_game_loop = 0;
        } else {
            g_player_table[g_local_nation_id].race = RaceId::Random;
            g_is_in_game_loop = 1;
            auto& randomizer = RaceRandomizer::instance();
            randomizer.process_randomization();
            g_is_in_game_loop = 0;
        }
    }
}

static void flash_window() {
    FLASHWINFO flash_info{
        .cbSize = sizeof(flash_info),
        .hwnd = g_main_window,
        .dwFlags = FLASHW_CAPTION | FLASHW_TRAY | FLASHW_TIMERNOFG,
    };
    FlashWindowEx(&flash_info);
}

// Hooked to play a sound on player join. -Pr0nogo
void on_player_join(u32 player_id, BinDlg* dialog, u32 data_1, u32 data_2) // 00470720
{
    if (!player_id) {
        player_id = *(u32*)(data_2 + 1);
        if (player_id < PLAYABLE_PLAYER_COUNT && player_id != *g_local_storm_id) {
            helper::play_sound_global(SoundId::Lobby_PlayerJoin);
            if (g_config.flash_window_lobby_join) flash_window();

                char player_name[28];
                player_id = SNetGetPlayerName(player_id, player_name, 25);
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
                            player_id = SNetSendServerChatCommand(final_string);
                        }
                    }
                }
            }
        }
    }
}