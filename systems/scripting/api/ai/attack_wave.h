#pragma once
#include "../script_common.h"
#include "../core_fwd.h"
#include <algorithm>   // required for std::ranges::transform
#include <iterator>    // for std::back_inserter
#include <ranges>
#include "../../../../types.h"

namespace lua::api::ai {

class AttackComposer {
public:
    AttackComposer(fp32 bias) : m_bias{bias} {}

    void add(fp32 weight, UnitId::Enum unit_id) {
        assert_unit_id(unit_id);
        m_weights[unit_id] += weight * allocation_weight(unit_id);
    }

    std::vector<std::pair<u32, UnitId::Enum>> build(fp32 budget) const {
        const auto total_weights = std::ranges::fold_left(m_weights, 0fp, [](fp32 acc, const auto& pair) { return acc + pair.second; });

        std::vector<std::pair<u32, UnitId::Enum>> result;
        for (const auto& [unit_id, weight] : m_weights) {
            const auto allocated_budget = budget * (weight / total_weights);
            const auto count = static_cast<u32>(allocated_budget / std::max(unit_value(unit_id), 1fp));
            if (count > 0) {
                result.emplace_back(count, unit_id);
            }
        }
        return result;
    }

private:
    static fp32 unit_value(UnitId::Enum unit_id) {
        return units_dat::mineral_cost[u32(unit_id)]
             + units_dat::vespene_cost[u32(unit_id)];
    }

    fp32 allocation_weight(UnitId::Enum unit_id) const {
        return fpm::pow(unit_value(unit_id), m_bias);
    }

private:
    fp32 m_bias;
    std::map<UnitId::Enum, fp32> m_weights;
};

class AttackWave {
public:
    AttackWave(u8 player, v2u16 from, std::optional<fp32> budget, std::optional<fp32> bias)
        : m_player{player}
        , m_from{from}
        , m_timeout{180}
        , m_budget{budget.value_or(0fp)}
        , m_bias{bias.value_or(0.5fp)} {}

    static void install(ScriptManager&, sol::state& lua) {
        lua.new_usertype<AttackWave>("AttackWave", sol::no_constructor,
            "budget", sol::property([](const AttackWave& self) { return f32(self.m_budget); }, [](AttackWave& self, f32 value) { self.m_budget = fp32(value); }), // TODO: use fixed point in lua -Veeq7
            "bias", sol::property([](const AttackWave& self) { return f32(self.m_bias); }, [](AttackWave& self, f32 value) { self.m_bias = fp32(value); }), // TODO: use fixed point in lua -Veeq7
            "from", sol::property([](const AttackWave& self) { return Point(self.m_from); }, [](AttackWave& self, Point value) { self.m_from = value; }),
            "timeout", &AttackWave::m_timeout,
            "add", &AttackWave::add,
            "compose", &AttackWave::compose,
            "send", sol::yielding(&AttackWave::send)
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---An AI attack wave.
---@class AttackWave
---@field budget integer Approximate combined mineral and gas cost of the wave. 
---@field bias number Bias for unit allocation when composing the wave. Defaults to 0.5.
---@field from Point Starting position.
---@field timeout integer Timeout in seconds for sending the wave. Defaults to 180.
AttackWave = {}

---Adds units to the attack wave.
---@param count number Number of units.
---@param unit_id UnitId Unit type.
function AttackWave:add(count, unit_id) end

---Adds unit types to the attack wave composition, for use with `budget` composing.
---@param weight number Weight of the unit type.
---@param unit_id UnitId Unit type.
function AttackWave:compose(weight, unit_id) end

---Sends the attack wave when it is ready. Waits until the wave is sent.
function AttackWave:send() end
)lua";
    }

    std::string to_string() const {
        if (m_budget == 0fp) {
            const auto units = array_to_string(m_units, [](const auto& pair) {
                return std::format("{}x{}", pair.first, magic_enum::enum_name<UnitId::Enum>(pair.second).data());
            });
            return std::format("AttackWave(player={}, from=({}, {}), timeout={}s,\n  units=[{}])", m_player + 1, m_from.x, m_from.y, m_timeout, units);
        }

        std::map<UnitId::Enum, u32> composed_units_counts;
        for (const auto& [count, unit_id] : m_units) {
            composed_units_counts[unit_id] += count;
        }
        for (const auto& [count, unit_id] : composed_units()) {
            composed_units_counts[unit_id] += count;
        }
        const auto weights = array_to_string(m_weights, [](const auto& pair) {
            return std::format("{}x{}", f32(pair.first), magic_enum::enum_name<UnitId::Enum>(pair.second).data());
        });
        const auto units_composed = array_to_string(composed_units_counts, [](const auto& pair) {
            return std::format("{}x{}", pair.second, magic_enum::enum_name<UnitId::Enum>(pair.first).data());
        });

        return std::format("AttackWave(player={}, from=({}, {}), timeout={}s, budget={}, bias={},\n  weights=[{}],\n  units=[{}])",
            m_player+1, m_from.x, m_from.y, m_timeout, f32(m_budget), f32(m_bias), weights, units_composed);
    }

    void add(u32 count, UnitId::Enum unit_id) {
        assert_unit_id(unit_id);
        assert_positive("count", count);
        if (count == 0) return;

        m_units.emplace_back(count, unit_id);
    }

    void compose(f32 weight, UnitId::Enum unit_id) { // TODO: use fixed point in lua -Veeq7
        assert_unit_id(unit_id);
        assert_positive("weight", weight);
        if (weight == 0) return;

        m_weights.emplace_back(fp32(weight), unit_id);
    }

    bool prepare() {
        return helper::ai_attack_manager(m_player, m_from.x, m_from.y, true, false);
    }

    void clear() {
        memset(&g_aiscript_controller[m_player].attackers, 0, sizeof(g_aiscript_controller[m_player].attackers));
        g_aiscript_controller[m_player].ai_attack_group_region = 0;
        g_aiscript_controller[m_player].last_attack_seconds = 0;
        helper::ai_attack_clear(m_player);
    }

    void send() {
        std::ranges::sort(m_units, std::greater{}, &std::pair<u32, UnitId::Enum>::second);
        for (auto [count, unit_id] : m_units) {
            if (!count) {
                continue;
            }
            helper::ai_attack_add(count, unit_id, m_player);
        }
        for (auto [count, unit_id] : composed_units()) {
            if (!count) {
                continue;
            }
            helper::ai_attack_add(u32(count), unit_id, m_player);
        }

        bool success = prepare();
        g_aiscript_controller[m_player].last_attack_seconds = std::max(s32(*g_elapsed_time_seconds) - 180 + s32(m_timeout), 0);
        if (success) {
            clear();
            return;
        }

        auto& script = g_script_manager->current_script();
        script.sleep(24);
        script.sleep_func = [this](Script& script) mutable {
            if (prepare()) {
                clear();
                return SleepState::Continue;
            }
            return script.sleep(24);
        };
    }

private:
    std::vector<std::pair<u32, UnitId::Enum>> composed_units() const {
        AttackComposer composer{m_bias};
        for (auto [weight, unit_id] : m_weights) {
            composer.add(weight, unit_id);
        }
        auto result = composer.build(m_budget);
        std::ranges::sort(result, std::greater{}, &std::pair<u32, UnitId::Enum>::second);
        return result;
    }

private:
    u8 m_player;
    v2u16 m_from;
    u32 m_timeout;
    std::vector<std::pair<u32, UnitId::Enum>> m_units;

    fp32 m_budget;
    fp32 m_bias;
    std::vector<std::pair<fp32, UnitId::Enum>> m_weights;
};

} // namespace lua::api::ai
