#pragma once
#include "bwhelper.h"
#include "bwdata.h"
#include "storm/storm.h"
#include "characters.h"
#include "logger.h"
#include "api.h"
#include <hooks/snp.h>
#include <thread>
#include <type_traits>
#include <ranges>

using namespace std::chrono_literals;

class GameRunner {
public:
    static GameRunner& instance() {
        static GameRunner lobby_manager;
        return lobby_manager;
    };

    enum class Result {
        Success,
        PlayerNameNotSet,
        ProviderNotInitialized,
        UnableToLoadMap,
        BadMapPath,
        UnableToJoinLobby,
        UnableToCreateGame,
        BadLobbyIndex,
    };

    struct PlayerSlot {
        RaceId::Enum race = RaceId::Random;
        u8 type = PlayerType::OpenSlot;
    };

    using SinglePlayerSlots = std::array<PlayerSlot, 8>;

    Result create_singleplayer_game(const char* map_path, u16 game_type, u16 game_subtype);
    Result create_multiplayer_game(const char* map_path, u16 game_type, u16 game_subtype, const char* game_name);
    Result join_lobby(u32 index);
    bool load_adfile(u32 index);
    
    GameData& game_data() { return m_mapfile.game_data; };
    Result update_adfiles();
    std::vector<AdFileEx>& adfiles() { return m_adfiles; };
    SinglePlayerSlots& player_slots() { return m_player_slots; };

private:
    GameRunner() = default;

    Result load_map(const char* path);
    bool setup_game(const char* game_name, u16 game_type, u16 game_subtype);
    u32 race_ready_room(u32 race);

    bool is_melee_game(u8 game_type, u8 game_subtype);

private:
    MapFile m_mapfile{};
    std::vector<AdFileEx> m_adfiles;
    SinglePlayerSlots m_player_slots;
};