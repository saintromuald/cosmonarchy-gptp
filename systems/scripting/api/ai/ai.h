#pragma once
#include "../script_common.h"
#include "../core_fwd.h"
#include "attack_wave.h"
#include "defense.h"
#include "town.h"

namespace lua::api::ai {

class AI {
public:
    explicit AI(u8 player) : m_player{player} {
        // TODO: Only call this once per player per game
        //       Also store main town position in the same singleton space -Veeq7
        g_aiscript_controller[m_player].ai_flags.is_use_map_settings = false;
        g_aiscript_controller[m_player].default_min = 0;
        g_aiscript_controller[m_player].ai_flags.is_default_build_off = true;
        g_aiscript_controller[m_player].ai_flags.is_transports_off = false;
        g_aiscript_controller[m_player].ai_max_force = 0;
    }

    static void install(ScriptManager& manager, sol::state& lua) {
        lua.new_usertype<AI>("AI",
            sol::call_constructor, sol::factories(
                [&manager]() { return AI{manager.current_script().player}; },
                [](u8 player) {
                    player -= 1;
                    if (player >= PlayerLimit) {
                        throw sol::error(std::format("AI player {} is out of range [1, {}]", player+1, PlayerLimit));
                    }
                    return AI{player};
                }
            ),

            "player", sol::property([](const AI& self) { return core::Player{self.m_player}; }),
            "campaign_mode", sol::property(
                [](const AI& self) { return g_aiscript_controller[self.m_player].ai_flags.is_use_map_settings; },
                [](AI& self, bool value) { g_aiscript_controller[self.m_player].ai_flags.is_use_map_settings = value; }
            ),
            "default_min", sol::property(
                [](const AI& self) { return g_aiscript_controller[self.m_player].default_min; },
                [](AI& self, u8 value) { g_aiscript_controller[self.m_player].default_min = value; }
            ),
            "default_build", sol::property(
                [](const AI& self) { return !g_aiscript_controller[self.m_player].ai_flags.is_default_build_off; },
                [](AI& self, bool value) { g_aiscript_controller[self.m_player].ai_flags.is_default_build_off = !value; }
            ),
            "use_transports", sol::property(
                [](const AI& self) { return !g_aiscript_controller[self.m_player].ai_flags.is_transports_off; },
                [](AI& self, bool value) { g_aiscript_controller[self.m_player].ai_flags.is_transports_off = !value; }
            ),
            "max_force", sol::property(
                [](const AI& self) { return g_aiscript_controller[self.m_player].ai_max_force; },
                [](AI& self, u8 value) { g_aiscript_controller[self.m_player].ai_max_force = value; }
            ),

            "start_town", &AI::start_town,
            "start_area_town", &AI::start_area_town,
            "expand", &AI::expand,
            "defense", sol::property(&AI::defense),
            "new_attack", &AI::new_attack,

            "minerals", sol::property([](const AI& self) { return g_ai_minerals_reserve[self.m_player]; }),
            "gas", sol::property([](const AI& self) { return g_ai_vespene_reserve[self.m_player]; })
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---Controls the AI for a player. Player IDs are 1-based.
---@class AI
---@overload fun(): AI
---@overload fun(player: integer): AI
---@field player Player Computer player. Read-only.
---@field campaign_mode boolean Whether the AI is in campaign mode. Defaults to false.
---@field default_min integer Minimum number of workers to keep. Defaults to 0.
---@field default_build boolean Whether the AI builds units automatically. Defaults to false.
---@field use_transports boolean Whether the AI uses transports. Defaults to true.
---@field max_force integer Maximum number of units the AI can control in a region. Defaults to 0 (no limit).
---@field defense Defense Defense settings.
---@field minerals integer Player AI available minerals. Read-only.
---@field gas integer Player AI available gas. Read-only.
AI = {}

---Starts an AI town.
---@param pos? Point Town position. Defaults to the resource area or player start position.
---@param resarea? integer Resource area. Defaults to the nearest resource area.
---@return Town town Created town.
---@nodiscard
function AI:start_town(pos, resarea) end

---Starts an AI town within an area.
---@param area Area Town area.
---@param resarea? integer Resource area. Defaults to the nearest resource area.
---@return Town town Created town.
---@nodiscard
function AI:start_area_town(area, resarea) end

---Starts a new expansion town.
---@param cap? integer Maximum number of expansions. Defaults to 255.
---@return Town town Created town, or an invalid town if no expansion is available.
---@nodiscard
function AI:expand(cap) end

---Creates a new attack wave.
---
---Without `budget`, the counts passed to `AttackWave:add()` are treated as exact unit counts.
---
---With `budget`, those counts are treated as relative composition weights instead.
---The total budget is distributed between unit types according to their weights and
---combined mineral and gas costs, then converted into whole unit counts.
---
---`bias` controls how strongly unit cost influences the budget allocated to each unit type.
---Unit cost still affects the final unit count because more expensive units consume more
---of the allocated budget per unit.
---
---Useful reference points:
--- - `-0.5` strongly amplifies the preference for cheaper units.
--- - `-0.25` amplifies the preference for cheaper units.
--- - `0` gives equal budget shares to equally weighted unit types.
--- - `0.25` shifts more budget toward expensive units, reducing the cheap-unit advantage.
--- - `0.5` balances budget and unit cost, favoring cheaper units without letting them
---   dominate as strongly. This is the default.
--- - `0.75` shifts the composition further toward expensive units and closer to the
---   requested weight ratios.
--- - `1` fully compensates for unit cost, so weights approximately represent unit-count ratios.
--- - `1.25` favors more expensive units beyond their cost compensation.
--- - `1.5` strongly favors more expensive units.
---
---@param budget? integer Approximate combined mineral and gas cost of the wave.
---@param bias? number Exponential unit-cost bias used with `budget`. Defaults to 0.5.
---@return AttackWave wave New attack wave.
---@nodiscard
function AI:new_attack(budget, bias) end
)lua";
    }

    std::string to_string() const {
        return std::format("AI(player={})", m_player+1);
    }

    Town start_town(std::optional<Point> pos, std::optional<u32> resarea) {
        if (!pos) pos = resarea ? g_resource_areas->areas[*resarea].position : g_start_positions[m_player];
        if (!resarea) resarea = bw::get_closest_resarea(*pos);

        auto* town = helper::aiscript_start_town(pos->x, pos->y, *resarea, m_player);
        if (!town) throw sol::error(std::format("Failed to start main town for player {}", m_player+1));

        g_aiscript_controller[m_player].ai_flags.is_town_started = true;
        helper::ai_update_town_regions(m_player);
        if (!m_main_town_pos) m_main_town_pos = *pos;

        return Town{town, m_player};
    }

    Town start_area_town(Area area, std::optional<u32> resarea) {
        auto box = area.box32_ltrb();
        if (!resarea) resarea = bw::get_closest_resarea(box.center());

        auto* town = helper::aiscript_start_areatown(&box, *resarea, m_player);
        if (!town) throw sol::error(std::format("Failed to start area town for player {}", m_player+1));

        g_aiscript_controller[m_player].ai_flags.is_town_started = true;
        helper::ai_update_town_regions(m_player);

        return Town{town, m_player};
    }

    Town expand(std::optional<u32> cap) {
        if (auto resarea = helper::ai_find_expansion(m_player, bw::get_closest_resarea(m_main_town_pos), cap.value_or(255))) {
            // TODO: we probably should call the region updates from allies watch
            return start_town({}, resarea);
        }
        return Town{m_player};
    }

    Defense defense() const {
        return Defense{m_player};
    }

    AttackWave new_attack(std::optional<f32> budget, std::optional<f32> bias) const { // TODO: use fixed point in lua -Veeq7
        return AttackWave{m_player, m_main_town_pos,
            budget.transform([](f32 b) { return fp32{b}; }),
            bias.transform([](f32 b) { return fp32{b}; })};
    }

private:
    u8 m_player;
    v2u16 m_main_town_pos{}; // TODO: Perhaps move this out to global state, so AI is just a stateless proxy type
};

} // namespace lua::api::ai
