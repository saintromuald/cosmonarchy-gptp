#pragma once
#include "../script_common.h"

namespace lua::api::ai {

class Town {
public:
    Town(u8 player) : m_town{nullptr}, m_generation{0}, m_player{player} {} // dead town
    Town(AiTown* town, u8 player) : m_town{town}, m_generation{get_town_ext(town).generation}, m_player{player} {}

    static void install(ScriptManager& manager, sol::state& lua) {
        lua.new_usertype<Town>("Town", sol::no_constructor,
            "is_alive", sol::property(&Town::is_alive),
            "player", sol::property([](const Town& self) { return core::Player{self.m_player}; }),
            "max_workers", sol::property(&Town::max_workers),
            "build", &Town::build,
            // TODO: build_at -Veeq7
            "wait_build", sol::yielding(&Town::wait_build),
            "transfer_workers", &Town::transfer_workers,
            "auto_kill_script", [&manager](Town& self) {
                auto& script = manager.current_script();
                script.kill_conditions.emplace_back([self](Script&) {
                    return !self.is_alive();
                });
            },
            "multirun", sol::yielding([&manager](Town& self, sol::function func, sol::variadic_args va_args) {
                if (!self.is_alive()) {
                    return ScriptHandle{};
                }

                auto& script = manager.current_script();
                std::vector<sol::object> args{sol::make_object(script.thread.state(), self)};
                args.append_range(va_args);

                auto& new_script = manager.load_script_multirun(func, std::move(args), script);
                new_script.kill_conditions.emplace_back([self](Script&) {
                    return !self.is_alive();
                });
                return ScriptHandle{new_script.id};
            })
        );
    }

    static const char* lsp_annotations() {
        return R"lua(
---An AI town.
---@class Town
---@field is_alive boolean Whether the town exists. Read-only.
---@field player Player Computer player. Read-only.
---@field max_workers integer Maximum number of workers this town can have. Read-only.
Town = {}

---Requests units or buildings from this town.
---@param count integer Number to build.
---@param unit_id UnitId Unit or building type.
---@param priority integer Build priority.
---@param if_needed? boolean Build only when needed. Defaults to `false`.
function Town:build(count, unit_id, priority, if_needed) end

---Waits until this town has the requested number of units.
---@param count integer Required unit count.
---@param unit_id UnitId Unit type to count.
---@param is_completed? boolean Count only completed units. Defaults to `true`.
function Town:wait_build(count, unit_id, is_completed) end

---Transfers workers from older towns into this town.
---@param count integer Number of workers.
function Town:transfer_workers(count) end

---Stops the current script when this town no longer exists.
function Town:auto_kill_script() end

---Runs a child script associated with this town.
---@param func function Function called with this town followed by `...`.
---@param ... any Additional arguments.
---@return ScriptHandle handle Child script handle.
function Town:multirun(func, ...) end
)lua";
    }

    std::string to_string() const {
        if (!is_alive()) {
            return "Town(nil)";
        }
        return std::format("Town(id={}, player={}, pos=({}, {}))", u32(m_town - g_ai_town_array), m_town->player+1, m_town->position.x, m_town->position.y);
    }

    bool is_alive() const {
        return m_town && m_town->ext().generation == m_generation;
    }

    void build(u8 count, UnitId::Enum unit_id, u8 priority, std::optional<bool> if_needed) {
        if (!is_alive()) return;
        if (units_dat::base_property[unit_id] & UnitProperty::Worker) {
            count = std::min<u8>(count, max_workers());
        }
        helper::ai_build(u32(unit_id), if_needed.value_or(false), count, m_town, priority);
    }

    void wait_build(u32 count, UnitId::Enum unit_id, std::optional<bool> is_completed) {
        if (!is_alive()) return;
        if (units_dat::base_property[unit_id] & UnitProperty::Worker) {
            count = std::min<u8>(count, max_workers());
        }
        g_script_manager->current_script().sleep_func = [this, count, unit_id, is_completed](Script& script) {
            if (!is_alive()) return SleepState::Continue;
            auto unit_count = helper::ai_count_units_in_town(m_town, u32(unit_id), is_completed.value_or(true));
            // For some reason we have garbage on 7th bit
            // TODO: Investigate why this happens -Veeq7
            unit_count &= 0xFFFFFF; 
            return unit_count < count ? script.sleep(1) : SleepState::Continue;
        };
    }

    void transfer_workers(u32 count) {
        if (!is_alive()) return;
        helper::ai_get_old_peons(m_town, count);
    }

    u32 max_workers() const {
        if (!is_alive()) return 0;
        auto& resarea = g_resource_areas->areas[m_town->resarea];
        return (resarea.mineral_field_count + resarea.geyser_count) * 3;
    }

private:
    AiTown* m_town;
    u32 m_generation;
    u8 m_player;
};

} // namespace lua::api::ai
