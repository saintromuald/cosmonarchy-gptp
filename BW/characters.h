#pragma once
#include "../BW/bwhelper.h"
#include "../BW/bwdata.h"
#include "api.h"
#include <ctime>

class CharacterManager {
public:
    static CharacterManager& instance() {
        static CharacterManager character_list;
        return character_list;
    };

    inline std::vector<CharacterFile>& current_list() { return g_multiplayer_mode ? m_multiplayer : m_singleplayer; };

    static void __stdcall add_character_to_list(CharacterFile* character);
    void recalculate_lists();
    bool set_player_name(const char* name);
    bool ensure_character(const char* name, bool multiplayer = g_multiplayer_mode);
    bool save_character_file(const char* name, bool multiplayer = g_multiplayer_mode);
    bool delete_character_file(const char* name, bool multiplayer = g_multiplayer_mode);

private:
    CharacterManager() = default;

private:
    std::vector<CharacterFile> m_singleplayer;
    std::vector<CharacterFile> m_multiplayer;
};