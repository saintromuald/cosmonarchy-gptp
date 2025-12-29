#include "game_runner.h"

GameRunner::Result GameRunner::create_singleplayer_game(const char* map_path, u16 game_type = GameType::Unranked, u16 game_subtype = 1) {
    if (!g_player_name) return Result::PlayerNameNotSet;
    g_multiplayer_mode = false;

    load_map(map_path);
    std::string_view path_sv{map_path};
    if (path_sv.substr(path_sv.length() - 3) != "rep") {
        setup_game(&g_player_name, game_type, game_subtype);
        g_is_in_replay = false;
    } else {
        g_is_in_replay = true;
    }

    auto game_data = &m_mapfile.game_data;
    auto melee = is_melee_game(game_data->game_type, game_data->game_subtype);
    if (!helper::prep_singleplayer_game()) return Result::UnableToCreateGame;
    if (!helper::start_singleplayer_game(game_data)) return Result::UnableToCreateGame;

    if (melee) {
        for (auto i = 0; i < 8; i++) {
            g_selected_singleplayer_races[i] = m_player_slots[i].race;
            g_player_table[i].race = m_player_slots[i].race;
            if (i) { // Don't change slot 0 (Player)'s type
                g_player_table[i].type = m_player_slots[i].type;
            }
        }
        helper::randomize_player_slot();
    } else {
        g_custom_singleplayer[0] = 1;
        for (auto i = 1; i < 8; i++) {
            g_selected_singleplayer_races[i] = RaceId::None;
        }
    }

    if (game_data->victory_condition == 0 && game_data->starting_units == 0
        && game_data->tournament_mode_enabled == 0 && g_is_in_replay == 0 && !melee) {
        g_gl_glues_mode = race_ready_room(g_player_table[0].race);
    } else {
        g_sc_main_state = MainState::Init;
    }

    m_player_slots = {};

    helper::change_menu();
    return Result::Success;
}

GameRunner::Result GameRunner::create_multiplayer_game(const char* map_path, u16 game_type = GameType::Unranked, u16 game_subtype = 1, const char* game_name = &g_player_name) {
    if (!g_player_name) return Result::PlayerNameNotSet;
    if (!g_spi_provider) return Result::ProviderNotInitialized;
    g_multiplayer_mode = true;

    auto result = load_map(map_path);
    if (result != Result::Success) return result;

    std::string_view path_sv{map_path};
    if (path_sv.substr(path_sv.length() - 3) != "rep") {
        setup_game(game_name, game_type, game_subtype);
        g_is_in_replay = false;
    } else {
        g_is_in_replay = true;
    }

    g_is_host = 0;
    if (!helper::create_multiplayer_lobby(&m_mapfile.game_data, 0)) return Result::UnableToCreateGame;

    g_gl_glues_mode = MenuId::GlueChatroom;
    g_glue_background_id = -1;

    helper::change_menu();
    return Result::Success;
}

GameRunner::Result GameRunner::join_lobby(u32 index) {
    if (!g_player_name) return Result::PlayerNameNotSet;
    if (!g_spi_provider) return Result::ProviderNotInitialized;
    if (index > m_adfiles.size()) return Result::BadLobbyIndex;
    g_multiplayer_mode = true;

    load_adfile(m_adfiles[index]);

    if (helper::join_game(&m_mapfile.game_data)) {
        g_gl_glues_mode = MenuId::GlueChatroom;
        g_glue_background_id = -1;
        helper::change_menu();
        return Result::Success;
    }

    return Result::UnableToJoinLobby;
}

GameRunner::Result GameRunner::update_adfiles() {
    if (!g_spi_functions) return Result::ProviderNotInitialized;
    m_adfiles.clear();
    AdFile* adfile_list{};
    // The SNP provider will lock its mutex/critsec and provide a pointer to its lobby list
    // If the list is empty it will return false and unlock its mutex so we can return
    if (!g_spi_functions->lock_game_list(0, 0, &adfile_list)) return Result::Success;
    for (auto ad_file = adfile_list; ad_file; ad_file = (AdFile*)ad_file->next) {
        m_adfiles.push_back(*ad_file);
    }
    // Unlock the SNP provider's mutex/critsec
    g_spi_functions->unlock_game_list(0, 0);
    return Result::Success;
}

GameRunner::Result GameRunner::load_map(const char* path) {
    safe_copy(m_mapfile.full_path, path);
    std::string_view path_sv{path};
    auto last_pathsep = path_sv.find_last_of("\\");
    if (last_pathsep == std::string_view::npos) return Result::BadMapPath;
    safe_copy(m_mapfile.full_filename, path_sv.substr(last_pathsep + 1).data());
    safe_copy(m_mapfile.display_filename, path_sv.substr(last_pathsep + 1).data());

    auto extension = path_sv.substr(path_sv.length() - 3);
    if (extension == "rep") {
        helper::load_replay_info(&m_mapfile);
        return Result::Success;
    } else if (extension == "scx" || extension == "scm") {
        helper::load_map_info(&m_mapfile);
    } else {
        return Result::BadMapPath;
    }

    safe_copy(m_mapfile.game_data.map_title, m_mapfile.map_name);
    m_mapfile.game_data.map_height_tiles = m_mapfile.map_height_tiles;
    m_mapfile.game_data.map_width_tiles = m_mapfile.map_width_tiles;
    m_mapfile.game_data.max_player_count = m_mapfile.max_players;
    m_mapfile.game_data.game_state = m_mapfile.game_state;
    m_mapfile.game_data.tileset = g_tileset_id; // This is Set in load_map_info
    return Result::Success;
}

void GameRunner::load_adfile(const AdFile& ad_file) {
    auto& game_data = m_mapfile.game_data;
    game_data.index = ad_file.game_index;
    // Parse description string
    safe_copy(game_data.game_name, ad_file.game_name);

    // Example statstring: ",33,18,7,,1e,,1,cb2edaab,5,,Server\rStatus\r"
    std::string_view sv{ad_file.game_description};
    auto statstring = sv | std::views::split(',')
        | std::views::transform([](auto range) { return std::string_view(range.begin(), range.end()); })
        | std::ranges::to<std::vector>();

    load_number(game_data.save_timestamp, statstring[0]);

    u32 width_height{};
    load_number(width_height, statstring[1]);
    if (width_height) {
        game_data.map_width_tiles = width_height / 16 * 32;
        game_data.map_height_tiles = width_height % 16 * 32;
    }

    u16 active_max{};
    load_number(active_max, statstring[2]);
    if (active_max) {
        game_data.active_player_count = active_max / 16;
        game_data.max_player_count = active_max % 16;
    } else {
        game_data.max_player_count = 8;
    }
    
    load_number(game_data.game_speed, statstring[3]);
    load_number(game_data.game_state, statstring[4]);
    load_number(game_data.game_type, statstring[5]);
    // Skipping unused index 6
    load_number(game_data.game_subtype, statstring[7]);
    load_number(game_data.cdkey_hash, statstring[8]);
    load_number(game_data.tileset, statstring[9]);
    load_number(game_data.is_replay, statstring[10]);

    if (!statstring[11].empty()) {
        auto parts = statstring[11] | std::views::split('\r')
            | std::views::transform([](auto range) { return std::string_view(range.begin(), range.end()); })
            | std::ranges::to<std::vector>();

        safe_copy(game_data.host_name, parts[0].data());
        if (parts.size() >= 2) safe_copy(game_data.map_title, parts[1].data());
    }
}

bool GameRunner::setup_game(const char* game_name, u16 game_type, u16 game_subtype) {
    auto& game_data = m_mapfile.game_data;
    safe_copy(game_data.game_name, game_name);
    game_data.game_type = game_type;
    game_data.game_subtype = game_subtype;
    game_data.active_player_count = 1;
    auto success = helper::load_map_forces(m_mapfile.full_path);
    auto error = helper::load_map_to_globals(&game_data, 3);
    return (!error && success);
}

u32 GameRunner::race_ready_room(u32 race) {
    switch (race) {
    case RaceId::Zerg: return MenuId::GlueReadyZ;
    case RaceId::Terran: return MenuId::GlueReadyT;
    case RaceId::Protoss: return MenuId::GlueReadyP;
    }
    return MenuId::GlueReadyT;
}

bool GameRunner::is_melee_game(u8 game_type, u8 game_subtype) {
    if (!g_game_type_list) {
        return false;
    }
    for (auto current_type = g_game_type_list; current_type; current_type = current_type->next) {
        if (current_type->game_type == game_type && current_type->sub_type == game_subtype) {
            return current_type->mode == GameTypeListMode::Melee;
        }
    }
    return false;
}