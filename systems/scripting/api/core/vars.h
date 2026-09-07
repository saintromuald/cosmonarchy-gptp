#pragma once
#include "../script_common.h"
#include <map>

namespace lua::api {

class PlayerVars {
public:
    PlayerVars(sol::state& lua) {
        for (u8 i = 0; i < PlayerLimit; ++i) {
            m_player_tables[i] = lua.create_table();
        }
    }

    static void install(ScriptManager& manager, sol::state& lua) {
        lua.new_usertype<PlayerVars>("PlayerVars",
            sol::meta_function::call, [&manager](const PlayerVars& self, std::optional<u8> player) -> sol::table {
                if (player) {
                    *player -= 1;
                    if (*player < 0 || *player >= PlayerLimit) {
                        throw sol::error(std::format("Player {} is out of range [1, {}]", *player + 1, PlayerLimit));
                    }
                } else {
                    player = manager.current_script().player;
                }
                return self.m_player_tables[*player];
            }
        );
        lua["PlayerVars"] = PlayerVars{lua};
    }

    static const char* lsp_annotations() {
        return R"lua(
---Global table for per-player variable storage.
---@class PlayerVars
PlayerVars = {}

---Returns the variable table for the specified player.
---@param player? integer Player ID (1-based). Returns the current script's player table if not specified.
---@return table player_vars Variable table for the specified player.
PlayerVars.__call = function(player) end
)lua";
    }

    std::string to_string() const {
        return "PlayerVars";
    }

private:
    sol::table m_player_tables[PlayerLimit];
};

class Vars {
public:
    static void install(ScriptManager& manager, sol::state& lua) {
        lua["Vars"] = lua.create_table();
    }

    static const char* lsp_annotations() {
        return R"lua(
---Global table for user variable storage.
Vars = {}
)lua";
    }

    std::string to_string() const {
        return "Vars";
    }
};

} // namespace lua::api
