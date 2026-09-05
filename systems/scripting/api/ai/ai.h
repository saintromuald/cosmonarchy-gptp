#pragma once
#include "../script_common.h"
#include "../core_fwd.h"
#include "attack_wave.h"
#include "defense.h"
#include "town.h"

namespace lua::api::ai {

class AI {
public:
    explicit AI(u8 player) : m_player{player} {}

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

            "start_town", &AI::start_town,
            "start_area_town", &AI::start_area_town,
            "expand", &AI::expand,
            "defense", sol::property(&AI::defense),
            "new_attack", &AI::new_attack
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---Controls the AI for a player. Player IDs are 1-based.
---@class AI
---@overload fun(): AI
---@overload fun(player: integer): AI
---@field defense Defense Defense settings. Read-only.
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
---@return AttackWave wave New attack wave.
---@nodiscard
function AI:new_attack() end
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

        return Town{town};
    }

    Town start_area_town(Area area, std::optional<u32> resarea) {
        auto box = area.box32_ltrb();
        if (!resarea) resarea = bw::get_closest_resarea(box.center());

        auto* town = helper::aiscript_start_areatown(&box, *resarea, m_player);
        if (!town) throw sol::error(std::format("Failed to start area town for player {}", m_player+1));

        g_aiscript_controller[m_player].ai_flags.is_town_started = true;
        helper::ai_update_town_regions(m_player);

        return Town{town};
    }

    Town expand(std::optional<u32> cap) {
        if (auto resarea = helper::ai_find_expansion(m_player, bw::get_closest_resarea(m_main_town_pos), cap.value_or(255))) {
            // TODO: we probably should call the region updates from allies watch
            return start_town({}, resarea);
        }
        return {};
    }

    Defense defense() const {
        return Defense{m_player};
    }

    AttackWave new_attack() const {
        return AttackWave{m_player, m_main_town_pos};
    }

private:
    u8 m_player;
    v2u16 m_main_town_pos{}; // TODO: Perhaps move this out to global state, so AI is just a stateless proxy type
};

} // namespace lua::api::ai
