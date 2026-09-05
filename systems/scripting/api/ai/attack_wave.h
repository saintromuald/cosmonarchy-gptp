#pragma once
#include "../script_common.h"
#include "../core_fwd.h"

namespace lua::api::ai {

class AttackWave {
public:
    AttackWave(u8 player, v2u16 from) : m_player{player}, m_from{from} {}

    static void install(ScriptManager&, sol::state& lua) {
        lua.new_usertype<AttackWave>("AttackWave", sol::no_constructor,
            "from", sol::property(&AttackWave::from, &AttackWave::set_from),
            "add", &AttackWave::add,
            "send", sol::yielding(&AttackWave::send)
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---An AI attack wave.
---@class AttackWave
---@field from Point Starting position.
AttackWave = {}

---Adds units to the attack wave.
---@param count integer Number of units.
---@param unit_id UnitId Unit type.
function AttackWave:add(count, unit_id) end

---Sends the attack wave when it is ready. Waits until the wave can be sent.
function AttackWave:send() end
)lua";
    }

    std::string to_string() const {
        std::stringstream ss;
        bool first = true;
        for (auto [count, unit_id] : m_units) {
            if (!first) {
                ss << ", ";
            }
            first = false;
            ss << std::format("{}x{}", count, magic_enum::enum_name<UnitId::Enum>(unit_id).data());
        }
        return std::format("AttackWave(player={}, from=({}, {}), units=[{}])", m_player+1, m_from.x, m_from.y, ss.str());
    }

    v2u16 from() const {
        return m_from;
    }

    void set_from(Point from) {
        m_from = from;
    }

    void add(u32 count, UnitId::Enum unit_id) {
        assert_unit_id(unit_id);
        m_units.emplace_back(count, unit_id);
    }

    bool prepare() {
        for (auto [count, unit_id] : m_units) {
            helper::ai_attack_add(count, unit_id, m_player);
        }
        return helper::ai_attack_manager(m_player, m_from.x, m_from.y, true, false);
    }

    void clear() {
        memset(&g_aiscript_controller[m_player].attackers, 0, sizeof(g_aiscript_controller[m_player].attackers));
        g_aiscript_controller[m_player].ai_attack_group_region = 0;
        g_aiscript_controller[m_player].last_attack_seconds = 0;
        helper::ai_attack_clear(m_player);
    }

    void send() {
        auto& script = g_script_manager->current_script();
        script.sleep_func = [this](Script& script) mutable {
            if (prepare()) {
                clear();
                return SleepState::Continue;
            }
            return script.sleep(24);
        };
    }

private:
    u8 m_player;
    v2u16 m_from;
    std::vector<std::pair<u32, UnitId::Enum>> m_units;
};

} // namespace lua::api::ai
