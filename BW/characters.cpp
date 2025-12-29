#include "characters.h"

void __stdcall CharacterManager::add_character_to_list(CharacterFile* character) {
    auto& character_manager = CharacterManager::instance();
    auto& current_list = character_manager.current_list();
    current_list.push_back(*character);
}

void CharacterManager::recalculate_lists() {
    auto& character_list = CharacterManager::instance();
    character_list.m_multiplayer.clear();
    character_list.m_singleplayer.clear();
    helper::iterate_characters(add_character_to_list);
    g_multiplayer_mode = !g_multiplayer_mode;
    helper::iterate_characters(add_character_to_list);
    g_multiplayer_mode = !g_multiplayer_mode;
}

bool CharacterManager::set_player_name(const char* name) {
    strncpy(&g_player_name, name, 25);
    return ensure_character(name);
}

bool CharacterManager::ensure_character(const char* name, bool multiplayer) {
    recalculate_lists();
    for (auto& character : current_list()) {
        if (std::string_view{name} == character.name) return true;
    }
    return save_character_file(name, multiplayer);
}

bool CharacterManager::save_character_file(const char* name, bool multiplayer) {
    bool original_mp_mode = g_multiplayer_mode;
    g_multiplayer_mode = multiplayer;

    CharacterFile char_file{};
    char_file.system_time = time(0);
    char_file.is_multiplayer = g_multiplayer_mode;
    safe_copy(char_file.name, name);
    
    auto bytes_written = helper::save_character_file(&char_file);
    g_multiplayer_mode = original_mp_mode;
    return bytes_written;
}

bool CharacterManager::delete_character_file(const char* name, bool multiplayer) {
    bool original_mp_mode = multiplayer;
    g_multiplayer_mode = multiplayer;

    char path[260]{};
    helper::character_file_path(name, path, sizeof(path));
    
    auto result = DeleteFileA(path);
    g_multiplayer_mode = original_mp_mode;
    return result;
}