#include "fraud_launcher.h"
#include <iostream>
#include <fstream>
#include <format>
#include "logger.h"

bool FraudLauncherClient::connect() {
    m_connected = false;
    if (!WaitNamedPipeA(m_pipe_to_server_name, NMPWAIT_WAIT_FOREVER)) {
        return false;
    }
    m_pipe_to_server = CreateFileA(m_pipe_to_server_name, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (m_pipe_to_server == INVALID_HANDLE_VALUE) {
        return false;
    }
    if (!WaitNamedPipeA(m_pipe_from_server_name, NMPWAIT_WAIT_FOREVER)) {
        return false;
    }
    m_pipe_from_server = CreateFileA(m_pipe_from_server_name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (m_pipe_from_server == INVALID_HANDLE_VALUE) {
        return false;
    }

    m_connected = true;
    return true;
}

void FraudLauncherClient::disconnect() {
    m_running = false;
    m_connected = false;
    if (m_pipe_to_server != INVALID_HANDLE_VALUE) {
        CloseHandle(m_pipe_to_server);
        m_pipe_to_server = INVALID_HANDLE_VALUE;
    }
    if (m_pipe_from_server_name != INVALID_HANDLE_VALUE) {
        CloseHandle(m_pipe_from_server);
        m_pipe_from_server = INVALID_HANDLE_VALUE;
    }
}

bool FraudLauncherClient::send(std::string& message) {
    if (!m_connected || m_pipe_to_server == INVALID_HANDLE_VALUE) return false;
    u32 length = message.length();
    DWORD bytes_written = 0;

    // Send message size
    auto success = WriteFile(m_pipe_to_server, &length, sizeof(u32), &bytes_written, NULL);
    if (!success || bytes_written != sizeof(u32)) return false;
    success = WriteFile(m_pipe_to_server, message.c_str(), length, &bytes_written, NULL);
    if (!success || bytes_written != length) return false;
    
    return true;
}

bool FraudLauncherClient::handle_messages() {
    // TODO: have a separate listener thread feeding messages into a queue and managing connection status
    // This should go in a separate method - this is just for proof of concept
    if (m_connected == false || m_pipe_from_server == INVALID_HANDLE_VALUE) return false;

    DWORD bytes_read = 0;
    u32 message_length = 0;
    if (!ReadFile(m_pipe_from_server, &message_length, sizeof(message_length), &bytes_read, NULL)) {
        DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            m_connected = false;
            return false;
        } else {
            // TODO: Handle other errors
            return false;
        }
    }
    if (bytes_read != sizeof(message_length)) return false;
        
    char buffer[256]{}; // TODO: Probably a std::vector would be better since we're using nlohmann

    if (!ReadFile(m_pipe_from_server, &buffer, message_length, &bytes_read, NULL)) {
        DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            m_connected = false;
            return false;
        } else {
            // TODO: Handle other errors
            return false;
        }
    }
    if (bytes_read != message_length) return false;

    std::string_view message{buffer};
    if (message == "sp") {
        example_create_singleplayer();
    } else if (message == "mp") {
        example_create_lobby();
    } else if (message == "join") {
        example_join_lobby();
    }

    return true;
}

// This function is for documentation only
void example_providers() {
    auto& provider_manager = snp::ProviderManager::instance();

    // Use this list to provide the user options for connecting to choose from
    auto& provider_list = provider_manager.available_providers();

    // When a provider is selected, initialize it
    auto result = provider_manager.initialize_provider('CNLK');

    // CrownLink allows you to also register a callback for when the server connection status changes
    // It will run immediately when registered to give you the current status
    provider_manager.register_crownlink_status_callback([](snp::CrownLinkStatus status) {
        // Send to Fraud Launcher
    }, false, false);

    // When leaving multiplayer mode (e.g. entering a singleplayer game) destroy the provider to reduce load on the server
    provider_manager.destroy_provider();
}

// This function is for documentation only
void example_characters() {
    auto& character_manager = CharacterManager::instance();

    // Use this to list the .spc or .mpc files
    // First update the lists then get the list for the current g_multiplayer_mode
    character_manager.recalculate_lists();
    character_manager.current_list();

    // Listing the characters may not be necessary depending on implementation
    // We don't really use the character files, we just need to make sure one exists
    // If a character file doesn't exist at the end of a match the game will crash trying to write to it
    
    // Instead we can just call this function to set g_player_name and ensure a spc or mpc file exists
    character_manager.set_player_name("jesse");

    // If we do want to explore utilizing the spc/mpc files for storing data there are functions that can be used for direct access
    character_manager.save_character_file("jesse");
    character_manager.delete_character_file("jesse");
}

// This function is for documentation only
void example_create_singleplayer() {
    CharacterManager::instance().set_player_name("test_single");
    auto& game_runner = GameRunner::instance();

    // Player slots are reset to RaceId::Random / PlayerType::OpenSlot after each call to create_singleplayer_game
    auto& slots = game_runner.player_slots();
    slots[0].race = RaceId::Protoss; // Human is always index 0
    slots[2].type = PlayerType::ClosedSlot; // You can close any slot except 0

    const char* melee_map = R"(C:\Cosmonarchy\Starcraft\Maps\CMBW-Root\CMBW-maps\melee\1v1\Class 1 - Empire\(2)Acid Test 1.2.scx)";
    const char* scenario_path = R"(C:\Cosmonarchy\Starcraft\Maps\CMBW-Root\CMBW-maps\campaign\Tutorials\1. Further Notice.scx)";
    const char* replay_path = R"(C:\Cosmonarchy\Starcraft\Maps\replays\release_sp_replay.rep)";
    
    auto result = game_runner.create_singleplayer_game(melee_map, GameType::Unranked, 1);
    // auto result = game_runner.create_singleplayer_game(scenario_path, GameType::Scenario);
    // auto result = game_runner.create_singleplayer_game(replay_path);
    return;
}

// This function is for documentation only
void example_create_lobby() {
    CharacterManager::instance().set_player_name("test_host");

    auto& provider_manager = snp::ProviderManager::instance();
    provider_manager.initialize_provider('CNLK');
    provider_manager.set_turns_per_second(snp::TurnsPerSecond::Medium);

    const char* melee_map = R"(C:\Cosmonarchy\Starcraft\Maps\CMBW-Root\CMBW-maps\melee\1v1\Class 1 - Empire\(2)Acid Test 1.2.scx)";
    const char* scenario_path = R"(C:\Cosmonarchy\Starcraft\Maps\CMBW-Root\CMBW-maps\campaign\Tutorials\1. Further Notice.scx)";
    const char* replay_path = R"(C:\Cosmonarchy\Starcraft\Maps\replays\release_sp_replay.rep)";

    auto& game_runner = GameRunner::instance();
    auto result = game_runner.create_multiplayer_game(melee_map, GameType::Unranked, 1, "test");
    // auto result = game_runner.create_multiplayer_game(scenario_path, GameType::Scenario);
    // auto result = game_runner.create_multiplayer_game(replay_path);
    return;
}

// This function is for documentation only
void example_join_lobby() {
    CharacterManager::instance().set_player_name("test_joiner");

    auto& provider_manager = snp::ProviderManager::instance();
    provider_manager.initialize_provider('CNLK');

    // Setting up a status callback to both wait for crownlink to be ready but also
    // to disable the "status lobby" that's normally used
    snp::CrownLinkStatus crownlink_status{};
    provider_manager.register_crownlink_status_callback([&crownlink_status](snp::CrownLinkStatus status) {
        crownlink_status = status;
    }, false, false);

    while (crownlink_status != snp::CrownLinkStatus::Ready) {
        std::this_thread::sleep_for(100ms);
    }

    auto& game_manager = GameRunner::instance();
    auto& adfiles = game_manager.adfiles();

    const auto start_time = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start_time < 30s) {
        game_manager.update_adfiles();
        
        if (game_manager.adfiles().size()) {
            // Check if it's joinable yet (crownlink specific)
            // Non-joinable lobbies have colorbyte::gray (0x5) as the first character
            if (game_manager.adfiles()[0].game_name[0] != 0x5) {
                auto result = game_manager.join_lobby(0);
                if (result == GameRunner::Result::Success) {
                    return;
                }
            }


        }
        std::this_thread::sleep_for(500ms);
    }
}