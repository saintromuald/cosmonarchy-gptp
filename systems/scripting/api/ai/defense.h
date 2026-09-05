#pragma once
#include "../script_common.h"

namespace lua::api::ai {

class DefenseArray {
public:
    static constexpr size_t Capacity = 20;
    using DefenseArrayType = u16[DefenseArray::Capacity];

    DefenseArray(DefenseArrayType& defense) : m_defense{defense} {}

    static void install(ScriptManager& manager, sol::state& lua) {
        lua.new_usertype<DefenseArray>("DefenseArray", sol::no_constructor,
            "count", &DefenseArray::count,
            "set", &DefenseArray::set,
            "add", &DefenseArray::add,
            "remove", &DefenseArray::remove,
            "clear", &DefenseArray::clear
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---A list of units used for defense.
---@class DefenseArray
DefenseArray = {}

---Returns how many times a unit appears in this list.
---@param unit_id UnitId Unit type.
---@return integer count Unit count.
---@nodiscard
function DefenseArray:count(unit_id) end

---Sets the number of units of this type.
---@param unit_id UnitId Unit type.
---@param count? integer Unit count. Defaults to 1.
function DefenseArray:set(unit_id, count) end

---Adds units to this list.
---@param unit_id UnitId Unit type.
---@param count? integer Unit count. Defaults to 1.
function DefenseArray:add(unit_id, count) end

---Removes units from this list.
---@param unit_id UnitId Unit type.
---@param count? integer Unit count. Removes all by default.
function DefenseArray:remove(unit_id, count) end

---Clears this list.
function DefenseArray:clear() end
)lua";
    }

    std::string to_string() const {
        std::stringstream ss;
        bool first = true;
        for (u32 i = 0; i < Capacity; ++i) {
            if (m_defense[i]) {
                if (!first) {
                    ss << ", ";
                }
                first = false;
                ss << magic_enum::enum_name<UnitId::Enum>(UnitId::Enum(m_defense[i] - 1)).data();
            }
        }
        return std::format("DefenseArray([{}])", ss.str());
    }

    u32 count(UnitId::Enum unit_id) const {
        assert_unit_id(unit_id);

        u32 count = 0;
        for (u32 i = 0; i < Capacity; ++i) {
            if (!m_defense[i]) {
                break;
            }
            if (m_defense[i] == u16(unit_id) + 1) {
                ++count;
            }
        }
        return count;
    }

    void set(UnitId::Enum unit_id, std::optional<u32> count) {
        assert_unit_id(unit_id);

        u32 current = this->count(unit_id);
        u32 expected = count.value_or(1);
        if (current < expected) {
            this->add(unit_id, expected - current);
        } else if (current > expected) {
            this->remove(unit_id, current - expected);
        }
    }

    void add(UnitId::Enum unit_id, std::optional<u32> count) {
        assert_unit_id(unit_id);

        for (u32 i = 0; i < Capacity; ++i) {
            if (!m_defense[i]) {
                for (u32 j = 0; j < count.value_or(1); ++j) {
                    u32 index = i + j;
                    if (index >= Capacity) {
                        break;
                    }
                    m_defense[index] = u16(unit_id) + 1;
                }
                break;
            }
        }
    }

    void remove(UnitId::Enum unit_id, std::optional<u32> count) {
        assert_unit_id(unit_id);

        u32 last_index = find_last_index();
        u32 left_to_remove = count.value_or(Capacity);
        for (u32 i = 0; i < last_index && left_to_remove; ++i) {
            if (m_defense[i] == u16(unit_id) + 1) {
                std::swap(m_defense[i], m_defense[last_index]);
                m_defense[last_index] = 0;
                --last_index;
                --left_to_remove;
            }
        }
    }

    void clear() {
        for (u32 i = 0; i < Capacity; ++i) {
            m_defense[i] = 0;
        }
    }

    u32 find_last_index() {
        u32 last_index = 0;
        for (u32 i = 0; i < Capacity; ++i) {
            if (m_defense[i] != 0) {
                last_index = i;
            } else {
                break;
            }
        }
        return last_index;
    }

private:
    u16* m_defense;
};

class DefenseBuild {
public:
    DefenseBuild(u8 player) : m_player{player} {}

    static void install(ScriptManager& manager, sol::state& lua) {
        lua.new_usertype<DefenseBuild>("DefenseBuild", sol::no_constructor,
            "gg", sol::property([](DefenseBuild& self) { return DefenseArray{g_aiscript_controller[self.m_player].ai_defensebuild_gg}; }),
            "ga", sol::property([](DefenseBuild& self) { return DefenseArray{g_aiscript_controller[self.m_player].ai_defensebuild_ga}; }),
            "ag", sol::property([](DefenseBuild& self) { return DefenseArray{g_aiscript_controller[self.m_player].ai_defensebuild_ag}; }),
            "aa", sol::property([](DefenseBuild& self) { return DefenseArray{g_aiscript_controller[self.m_player].ai_defensebuild_aa}; })
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---Units the AI can build for defense.
---@class DefenseBuild
---@field gg DefenseArray Ground targets on reachable terrain.
---@field ga DefenseArray Ground targets on unreachable terrain.
---@field ag DefenseArray Air targets over reachable terrain.
---@field aa DefenseArray Air targets over unreachable terrain.
DefenseBuild = {}
)lua";
    }

    std::string to_string() const {
        return std::format("DefenseBuild(player={})", m_player+1);
    }

private:
    u8 m_player;
};

class DefenseUse {
public:
    DefenseUse(u8 player) : m_player{player} {}

    std::string to_string() const {
        return std::format("DefenseUse(player={})", m_player+1);
    }
    
    static void install(ScriptManager& manager, sol::state& lua) {
        lua.new_usertype<DefenseUse>("DefenseUse", sol::no_constructor,
            "gg", sol::property([](DefenseUse& self) { return DefenseArray{g_aiscript_controller[self.m_player].ai_defenseuse_gg}; }),
            "ga", sol::property([](DefenseUse& self) { return DefenseArray{g_aiscript_controller[self.m_player].ai_defenseuse_ga}; }),
            "ag", sol::property([](DefenseUse& self) { return DefenseArray{g_aiscript_controller[self.m_player].ai_defenseuse_ag}; }),
            "aa", sol::property([](DefenseUse& self) { return DefenseArray{g_aiscript_controller[self.m_player].ai_defenseuse_aa}; })
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---Units the AI can use for defense.
---@class DefenseUse
---@field gg DefenseArray Ground targets on reachable terrain.
---@field ga DefenseArray Ground targets on unreachable terrain.
---@field ag DefenseArray Air targets over reachable terrain.
---@field aa DefenseArray Air targets over unreachable terrain.
DefenseUse = {}
)lua";
    }

private:
    u8 m_player;
};

class Defense {
public:
    Defense(u8 player) : m_player{player} {}

    static void install(ScriptManager& manager, sol::state& lua) {
        lua.new_usertype<Defense>("Defense", sol::no_constructor,
            "build", sol::property([](Defense& self) { return DefenseBuild{self.m_player}; }),
            "use", sol::property([](Defense& self) { return DefenseUse{self.m_player}; }),

            "auto_set", &Defense::auto_set,
            "auto_remove", &Defense::auto_remove
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---AI defense settings.
---@class Defense
---@field build DefenseBuild Units the AI can build. Read-only.
---@field use DefenseUse Units the AI can use. Read-only.
Defense = {}

---Adds a unit to the appropriate defense lists.
---@param unit_id UnitId Unit type.
---@param count? integer Unit count. Defaults to 1.
function Defense:auto_set(unit_id, count) end

---Removes a unit from all defense lists.
---@param unit_id UnitId Unit type.
---@param count? integer Unit count. Removes all by default.
function Defense:auto_remove(unit_id, count) end
)lua";
    }
    
    std::string to_string() const {
        return std::format("Defense(player={})", m_player+1);
    }

    void auto_set(UnitId::Enum unit_id, std::optional<u32> count) {
        assert_unit_id(unit_id);

        const auto unit_property = units_dat::base_property[unit_id];
        if (unit_property & UnitProperty::Building || !unit_id_is_combatant(unit_id)) {
            throw sol::error(std::format("UnitId.{} is not a combatant unit", magic_enum::enum_name(unit_id)));
        }
        
        const auto set = [this, unit_id, count](DefenseArray array) {
            array.set(unit_id, count);
        };

        // First letter is responsible for target type, second letter is responsible for movement capability
        // gg - ground target, reachable terrain
        // ga - ground target, unreachable terrain
        // ag - air target, reachable terrain
        // aa - air target, unreachable terrain
        if (unit_id_can_attack_ground(unit_id)) {
            set(g_aiscript_controller[m_player].ai_defensebuild_gg);
            set(g_aiscript_controller[m_player].ai_defenseuse_gg);
        }
        if (unit_id_can_attack_air(unit_id)) {
            set(g_aiscript_controller[m_player].ai_defensebuild_ag);
            set(g_aiscript_controller[m_player].ai_defenseuse_ag);
        }
        if (unit_property & UnitProperty::Flyer) {
            if (unit_id_can_attack_ground(unit_id)) {
                set(g_aiscript_controller[m_player].ai_defensebuild_ga);
                set(g_aiscript_controller[m_player].ai_defenseuse_ga);
            }
            if (unit_id_can_attack_air(unit_id)) {
                set(g_aiscript_controller[m_player].ai_defensebuild_aa);
                set(g_aiscript_controller[m_player].ai_defenseuse_aa);
            }
        }
    }

    void auto_remove(UnitId::Enum unit_id, std::optional<u32> count) {
        assert_unit_id(unit_id);
        const auto remove = [this, unit_id, count](DefenseArray array) {
            array.remove(unit_id, count);
        };
        remove(g_aiscript_controller[m_player].ai_defensebuild_gg);
        remove(g_aiscript_controller[m_player].ai_defenseuse_gg);
        remove(g_aiscript_controller[m_player].ai_defensebuild_ag);
        remove(g_aiscript_controller[m_player].ai_defenseuse_ag);
        remove(g_aiscript_controller[m_player].ai_defensebuild_ga);
        remove(g_aiscript_controller[m_player].ai_defenseuse_ga);
        remove(g_aiscript_controller[m_player].ai_defensebuild_aa);
        remove(g_aiscript_controller[m_player].ai_defenseuse_aa);
    }

private:
    u8 m_player;
};

} // namespace lua::api::ai
