#include "lobby_command_receive.h"

#include "BW/bwdata.h"
#include "BW/enumerations.h"
#include "BW/Storm.h"
#include "hooks/interface/mainmenu/lobby.h"
#include "BW/api.h"
#include "hooks/race_randomization.h"

#include <ranges>
#include <cstdint>
#include <span>
#include <variant>
#include <memory>
#include <functional>

static bool is_all_cpu_force(u8 force) {
    for (const auto& player : g_player_table) {
        if (player.force == force) {
            // If we find a human player in this force, it's not a computer-only force
            if (player.type != PlayerType::Computer &&
                player.type != PlayerType::ComputerSlot) {
                return false;
            }
        }
    }
    return true;
}

static bool mode_allows_change(u8 player_index) {
    if (g_game_data.victory_condition != VictoryCondition::Default) return true;
    if (g_game_data.starting_units) return true;
    if (g_game_data.tournament_mode_enabled) return true;
    if (g_player_force[player_index]) return true;
    return false;
}

static bool same_force_as_host(u8 player_index) {
    if (g_player_table[0].force == g_player_table[player_index].force) return true;
    return false;
}

static bool can_change_race(RaceId::Enum race, u8 player_index, u32 sender) {
    if (player_index >= 8) return false;
    if (sender >= 8) return false;
    if (!is_valid_race(race)) return false;
    if (load_game_file_handle->_Placeholder) return false;
    if (!mode_allows_change(player_index)) return false;

    auto multiplayer_and_teams = g_multiplayer_mode && g_game_data.teams_enabled;

    if (!g_player_table[sender].actions) {
        if (multiplayer_and_teams && !same_force_as_host(player_index)
            && !is_all_cpu_force(g_player_table[player_index].force)) {
            return false;
        } else if (!multiplayer_and_teams && player_index != sender
            && g_player_table[player_index].type != PlayerType::ComputerSlot) {
            return false;
        }
        return true;
    }

    if (multiplayer_and_teams && same_force_as_host(player_index)) return true;
    if (!multiplayer_and_teams && player_index == sender) return true;
    return false;
}

void broadcast_player_update(Player& player) {
    if (player.id >= 8) return;
    if (!g_lobby_commands_player_update[player.id]) {
        g_lobby_commands_player_update[player.id] = (Command_PlayerUpdate*)SMemAlloc(sizeof(Command_PlayerUpdate), __FILE__, __LINE__, 0);
    }
    g_lobby_commands_player_update[player.id]->command = GameCommand_PlayerUpdate;
    g_lobby_commands_player_update[player.id]->player_index = player.id;
    g_lobby_commands_player_update[player.id]->player_actions_lowest_byte = player.actions;
    g_lobby_commands_player_update[player.id]->type = player.type;
    g_lobby_commands_player_update[player.id]->race = static_cast<RaceId::Enum>(player.race);
    g_lobby_commands_player_update[player.id]->force = player.force;
}

template <typename T>
static void send_and_free(T*& command) {
    if (command) {
        helper::queue_game_command(reinterpret_cast<u8*>(command), sizeof(*command));
        SMemFree(command, __FILE__, __LINE__, 0);
        command = nullptr;
    }
}

void queue_lobby_commands() {
    if (g_game_state == GameState::FinalCommands) {
        g_game_state = GameState::WaitingForSaveData;
    } else if (std::to_underlying(g_game_state) > 6) {
        return;
    }

    auto& race_randomizer = RaceRandomizer::instance();
    
    send_and_free(g_lobby_command_start_countdown);
    send_and_free(g_lobby_command_transfer_status);
    for (auto& command : g_lobby_commands_player_update) {
        if (command) {
            command->random_flags = std::to_underlying(race_randomizer.player_race_flags(command->player_index));
        }
        send_and_free(command);
    }
    for (auto& command : g_lobby_commands_player_presence) {
        send_and_free(command);
    }

    if (g_lobby_command_change_race) {
        g_lobby_command_change_race->random_flags = std::to_underlying(race_randomizer.race_flags());
    }
    send_and_free(g_lobby_command_change_race);
    send_and_free(g_lobby_command_join_force);
    send_and_free(g_lobby_command_move_to_slot);
    send_and_free(g_lobby_command_set_slot_state);
    send_and_free(g_lobby_command_update_force);
    send_and_free(g_lobby_command_start_game_sync);
    send_and_free(g_lobby_command_make_public);

    if (g_lobby_command_request_join) {
        g_lobby_command_request_join->random_flags = std::to_underlying(race_randomizer.race_flags());
    }
    send_and_free(g_lobby_command_request_join);
}

static void process_change_race_command(const Command_ChangeRace* command, u32 sender) {
    if (!g_is_host) return;
    if (g_game_state != GameState::InLobby) return;

    const auto index = command->player_index == 8 ? g_player_id_table[sender] : command->player_index;

    if (!can_change_race(command->race, index, g_player_id_table[sender])) return;

    auto& race_randomizer = RaceRandomizer::instance();
    race_randomizer.set_player_race_flags(index, command->random_flags);

    auto force = g_player_table[index].force;
    auto multiplayer_and_teams = g_multiplayer_mode && g_game_data.teams_enabled;
    if (multiplayer_and_teams && is_all_cpu_force(force)) {
        for (auto& player : g_player_table | std::views::filter([force](const auto& p) {
            return p.force == force;
            })) {
            player.race = command->race;
            broadcast_player_update(player);
        }
    } else {
        g_player_table[index].race = command->race;
        broadcast_player_update(g_player_table[index]);
    }
    g_chat_update_needed = true;
}

static void process_player_update_command(const Command_PlayerUpdate* command, u32 sender) {
    if (sender != 0) return;
    if (g_is_host) return;
    if (command->player_index >= 8) return;
    u8 actions_byte = command->player_actions_lowest_byte;
    if (actions_byte > 8 && actions_byte != 0xff) return;

    u32 player_actions = (actions_byte == 0xFF) ? 0xFFFFFFFF : static_cast<u32>(actions_byte);

    char name_buffer[25];
    auto got_player_name = SNetGetPlayerName(player_actions, name_buffer, 25);
    if (command->type == PlayerType::Human && !got_player_name) return;
    if (g_game_state == GameState::Unk_3) {
        g_game_state = GameState::InLobby;
    } else if (g_game_state == GameState::Unk_1 || g_game_state == GameState::Unk_2) return;

    Player& player = g_player_table[command->player_index];
    player = {
        .id = command->player_index,
        .actions = player_actions,
        .type = command->type,
        .race = command->race,
        .force = command->force
    };

    if (player.type == PlayerType::Human && got_player_name) {
        safe_copy(player.name, name_buffer);
    }
    helper::update_nation_array(g_player_id);
    g_chat_update_needed = true;

    auto& race_randomizer = RaceRandomizer::instance();
    race_randomizer.set_player_race_flags(command->player_index, command->random_flags);
}

CommandResult CommandResult::success(size_t bytes) {
    return {Status::Success, bytes, ""};
}

CommandResult CommandResult::insufficient_data() {
    return {Status::InsufficientData, 0, "Insufficient packet data"};
}

CommandResult CommandResult::invalid_command() {
    return {};
}

CommandResult CommandResult::abort(const std::string& reason) {
    return {};
}

CommandInfo::CommandInfo(size_t size, CommandProcessor proc, std::string n)
    : required_size(size), processor(std::move(proc)), name(std::move(n)) {
}

LobbyCommandProcessor::LobbyCommandProcessor() {
    register_handlers();
}

template <typename T>
static void original_handler(T* command, u32 sender) {
    auto size = sizeof(T);
    helper::lobby_recv_original(size, sender, (void*)command);
}

void LobbyCommandProcessor::register_handlers() {
    m_handlers.emplace(GameCommand_PlayerUpdate, CommandInfo{
        sizeof(Command_PlayerUpdate),
        [](std::span<const u8> data, u32 sender_id) -> CommandResult {
            auto* command = reinterpret_cast<const Command_PlayerUpdate*>(data.data());
            process_player_update_command(command, sender_id);
            return CommandResult::success(sizeof(Command_PlayerUpdate));
        },
        "PlayerUpdate"
    });
    m_handlers.emplace(GameCommand_JoinRequest, CommandInfo{
        sizeof(Command_PlayerJoinRequest),
        [](std::span<const u8> data, u32 sender_id) -> CommandResult {
            auto* command = reinterpret_cast<const Command_PlayerJoinRequest*>(data.data());
            
            auto& race_randomizer = RaceRandomizer::instance();
            race_randomizer.set_player_race_flags(sender_id, command->random_flags);

            // Only pass the original struct to lobby_recv_original
            auto original_size = 18;
            helper::lobby_recv_original(original_size, sender_id, (void*)command);
            return CommandResult::success(sizeof(Command_PlayerJoinRequest));
        },
        "JoinRequest"
    });
    m_handlers.emplace(GameCommand_ChangeRace, CommandInfo{
        sizeof(Command_ChangeRace),
        [](std::span<const u8> data, u32 sender_id) -> CommandResult {
            auto* command = reinterpret_cast<const Command_ChangeRace*>(data.data());
            process_change_race_command(command, sender_id);
            return CommandResult::success(sizeof(Command_ChangeRace));
        },
        "ChangeRace"
    });
}

void LobbyCommandProcessor::process_command_data(std::span<const u8> command_data, u32 sender_id) {
    auto data = command_data;

    while (!data.empty()) {
        auto result = process_single_command(data, sender_id);

        switch (result.status) {
            case CommandResult::Status::Success: {
                data = data.subspan(result.bytes_consumed);
            } break;
            case CommandResult::Status::InsufficientData:
            case CommandResult::Status::InvalidCommand:
            case CommandResult::Status::Abort: return;
        }
    }
}

static u32 get_command_size(u8 command_id) {
    switch (command_id) {
        case GameCommand_Sequence: return sizeof(Command_Sequence);
        case GameCommand_StartCountdown: return sizeof(Command_StartCountdown);
        case GameCommand_MapFileTransferStatus: return sizeof(Command_FileTransferStatus);
        case GameCommand_PlayerUpdate: return sizeof(Command_PlayerUpdate);
        case GameCommand_PlayerPresence: return sizeof(Command_PlayerPresence);
        case GameCommand_JoinRequest: return sizeof(Command_PlayerJoinRequest);
        case GameCommand_ChangeRace: return sizeof(Command_ChangeRace);
        case GameCommand_JoinForce: return sizeof(Command_JoinForce);
        case GameCommand_MoveToSlot: return sizeof(Command_MoveToSlot);
        case GameCommand_SetSlotState: return sizeof(Command_SetSlotState);
        case GameCommand_UpdatePlayerForce: return sizeof(Command_UpdatePlayerForce);
        case GameCommand_StartGameSync: return sizeof(Command_StartGameSync);
        case GameCommand_UnknownLobby: return sizeof(Command_UnknownLobby);
        case GameCommand_PlayerJoined: return sizeof(Command_PlayerJoined);
        case GameCommand_MakePublic: return sizeof(Command_MakePublic);;
    }
    return 0;
}

CommandResult LobbyCommandProcessor::process_single_command(std::span<const u8> data, u32 sender_id) {
    if (data.empty()) return CommandResult::insufficient_data();

    u8 command_id = data[0];

    auto handler_it = m_handlers.find(command_id);
    if (handler_it != m_handlers.end()) {
        auto& handler = handler_it->second;
        if (data.size() < handler.required_size) {
            return CommandResult::insufficient_data();
        }
        return handler.processor(data, sender_id);
    }

    // Handle unregistered commands by forwarding to original handler
    auto command_size = get_command_size(command_id);
    if (!command_size) {
        return CommandResult::invalid_command();
    }

    if (data.size() < command_size) {
        return CommandResult::insufficient_data();
    }

    helper::lobby_recv_original(command_size, sender_id, (void*)data.data());
    return CommandResult::success(command_size);
}