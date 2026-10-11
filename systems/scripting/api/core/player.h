#pragma once
#include "../script_common.h"
#include "../../../triggers/trigger_system.h"
#include "../../../../hooks/unit_stats/new_requirements.h"

namespace lua::api::core {

// In BW player is 0 based, but in lua we want it 1 based, because:
// - Lua uses 1 based indexing for arrays, so it is more consistent to use 1 based indexing for players as well
// - We like more human friendly player numbers, same as we did in crigs
// We must remember to convert to subtract one when taking parameters and add one in properties and to_string methods
class Player {
public:
    Player() : m_player{g_script_manager->current_script().player} {}
    explicit Player(u8 player) : m_player{player} {}

    static void install(ScriptManager& manager, sol::state& lua) {
        lua.new_usertype<Player>("Player",
            sol::call_constructor, sol::factories(
                [&manager]() { return Player{manager.current_script().player}; },
                [](u8 player) {
                    player -= 1;
                    if (player >= PlayerLimit) {
                        throw sol::error(std::format("Player {} is out of range [1, {}]", player+1, PlayerLimit));
                    }
                    return Player{player};
                }
            ),

            "id", sol::property([](const Player& self) { return self.m_player+1; }),
            "race", sol::property(&Player::race),
            "faction", sol::property(&Player::faction),
            "type", sol::property(&Player::type),
            "force", sol::property(&Player::force),
            "name", sol::property(&Player::name),

            "minerals", sol::property([](const Player& self) { return g_game_minerals[self.m_player]; },
                [](const Player& self, u32 value) {
                    g_total_minerals[self.m_player] += value - g_game_minerals[self.m_player];
                    g_game_minerals[self.m_player] = value;
                }),
            "gas", sol::property([](const Player& self) { return g_game_vespene[self.m_player]; },
                [](const Player& self, u32 value) {
                    g_total_vespene[self.m_player] += value - g_game_vespene[self.m_player];
                    g_game_vespene[self.m_player] = value;
                }),
            
            "unit_count", &Player::unit_count,
            "can_make", &Player::can_make
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---A player. Player IDs are 1-based.
---@class Player
---@overload fun(): Player
---@overload fun(player: integer): Player
---@field id integer Player ID (1-based). Read-only.
---@field race RaceId Player race. Read-only.
---@field type PlayerType Player type. Read-only.
---@field force integer Player force. Read-only.
---@field name string Player name. Read-only.
---@field minerals integer Player minerals.
---@field gas integer Player gas.
Player = {}

---Returns the number of units of a given type owned by this player.
---@param unit_id UnitId Unit type to count.
---@param completed? boolean Count only completed units. Defaults to `true`.
---@return integer count Unit count.
---@nodiscard
function Player:unit_count(unit_id, completed) end

---Returns whether this player can make a given unit type.
---@param unit_id UnitId Unit type to check.
---@return boolean can_make Whether the player can make the unit type.
---@nodiscard
function Player:can_make(unit_id) end
)lua";
    }

    std::string to_string() const {
        return std::format("Player({}, race={}, faction={}, type={}, force={}, name={})",
            m_player+1, magic_enum::enum_name(race()), magic_enum::enum_name(faction()), magic_enum::enum_name(type()), force(), name());
    }

    RaceId::Enum race() const {
        return RaceId::Enum(g_player_table[m_player].race);
    }

    Faction faction() const {
        return g_globals->players[m_player].faction;
    }

    PlayerType::Enum type() const {
        return PlayerType::Enum(g_player_table[m_player].type);
    }

    u8 force() const {
        return g_player_table[m_player].force;
    }

    std::string name() const {
        return g_player_table[m_player].name;
    }

    u32 unit_count(UnitId::Enum unit_id, std::optional<bool> completed) const {
        assert_unit_id(unit_id);
        return completed.value_or(true)
            ? ::Player::number_of_completed_units_of_id(m_player, unit_id)
            : ::Player::number_of_units_of_id(m_player, unit_id);
    }

    bool can_make(UnitId::Enum unit_id) const {
        assert_unit_id(unit_id);
        return hooks::create_unit_id_allowed(unit_id, m_player, CheckCreator::Factories);
    }

private:
    u8 m_player;
};

} // namespace lua::api::core
