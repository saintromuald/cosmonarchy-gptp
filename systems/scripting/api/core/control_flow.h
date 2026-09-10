#pragma once
#include "script_common.h"
#include "core/player.h"

namespace lua::api {

class ControlFlow {
public:
    static void install(ScriptManager& manager, sol::state& lua) {
        class Script_ {};
        lua.new_usertype<Script_>("Script", sol::no_constructor,
            "id", sol::property([&manager]() { return manager.current_script().id; }),
            "type", sol::property([&manager]() { return manager.current_script().type_name(); }),
            "is_multirun", sol::property([&manager]() { return manager.current_script().is_multirun; }),
            "name", sol::property([&manager]() { return manager.current_script().name(); }),
            "label", sol::property([&manager]() { return manager.current_script().label; }),
            "player", sol::property([&manager]() { return core::Player{manager.current_script().player}; })
        );
        lua["Script"] = Script_{};

        lua.new_usertype<ScriptHandle>("ScriptHandle", sol::no_constructor,
            "id", sol::readonly(&ScriptHandle::id),
            "type", sol::property([&manager](ScriptHandle handle) { auto script = manager.find_script(handle); return script ? script->type_name() : ""; }),
            "is_multirun", sol::property([&manager](ScriptHandle handle) { auto script = manager.find_script(handle); return script ? script->is_multirun : false; }),
            "name", sol::property([&manager](ScriptHandle handle) { auto script = manager.find_script(handle); return script ? script->name() : ""; }),
            "label", sol::property([&manager](ScriptHandle handle) { auto script = manager.find_script(handle); return script ? script->label : ""; }),
            "player", sol::property([&manager](ScriptHandle handle) -> std::optional<core::Player> {
                auto script = manager.find_script(handle);
                return script ? std::optional<core::Player>{core::Player{script->player}} : std::nullopt;
            }),
            "is_alive", sol::property([&manager](ScriptHandle handle) {
                auto script = manager.find_script(handle);
                return script ? script->is_alive : false;
            }),
            "kill", [&manager](ScriptHandle handle) {
                auto script = manager.find_script(handle);
                if (script) {
                    script->is_alive = false;
                }
            }
        );

        lua.set_function("multirun", sol::yielding([&manager](sol::function func, sol::variadic_args args) {
            auto& script = manager.load_script_multirun(func, {args.begin(), args.end()}, manager.current_script());
            return ScriptHandle{script.id};
        }));
        lua.set_function("wait", sol::yielding([&manager](u32 ticks) {
            manager.current_script().sleep_until = g_elapsed_time_frames + ticks;
        }));
        lua.set_function("wait_until", sol::yielding([&manager](sol::function func, std::optional<u32> interval) {
            auto& script = manager.current_script();
            script.sleep_func = [&manager, func, interval](Script& script) {
                sol::function_result result = func();
                if (!result.valid()) {
                    manager.report_error("wait_until", result);
                    return SleepState::Error;
                }
                return !result.get<bool>() ? script.sleep(interval.value_or(1)) : SleepState::Continue;
            };
        }));
        lua.set_function("wait_while", sol::yielding([&manager](sol::function func, std::optional<u32> interval) {
            auto& script = manager.current_script();
            script.sleep_func = [&manager, func, interval](Script& script) {
                sol::function_result result = func();
                if (!result.valid()) {
                    manager.report_error("wait_while", result);
                    return SleepState::Error;
                }
                return result.get<bool>() ? script.sleep(interval.value_or(1)) : SleepState::Continue;
            };
        }));
        lua.set_function("kill_if", [&manager](sol::function func) {
            auto& script = manager.current_script();
            script.kill_conditions.emplace_back([&manager, func](Script&) {
                sol::function_result result = func();
                if (!result.valid()) {
                    manager.report_error("kill_if", result);
                    return true;
                }
                return result.get<bool>();
            });
        });
        lua.set_function("run_script", [&manager](const char* path) {
            std::string path_str = path;
            std::ranges::replace(path_str, '.', '/');
            path_str = "lua/" + path_str + ".lua";
            manager.load_script_mpq(path_str.c_str(), manager.current_script().player);
        });
    }

    static const char* lsp_annotations() {
        return R"lua(
---The currently running script.
---@class Script
---@field id integer Script ID. Read-only.
---@field type string Script type. Read-only.
---@field is_multirun boolean Whether this is a child script. Read-only.
---@field name string Script name. Read-only.
---@field label string Script label. Read-only.
---@field player Player Script player. Read-only.
Script = {}

---A reference to a script.
---@class ScriptHandle
---@field id integer Script ID. Read-only.
---@field type string Script type, or an empty string if it no longer exists. Read-only.
---@field is_multirun boolean Whether this is a child script. Read-only.
---@field name string Script name, or an empty string if it no longer exists. Read-only.
---@field label string Script label, or an empty string if it no longer exists. Read-only.
---@field player Player? Script player, or nil if it no longer exists. Read-only.
---@field is_alive boolean Whether the script is still running. Read-only.
ScriptHandle = {}

---Stops the script if it is still running.
function ScriptHandle:kill() end

---Runs a child script.
---@param func fun(...) Function to run.
---@param ... any Arguments passed to the function.
---@return ScriptHandle handle Child script handle.
function multirun(func, ...) end

---Waits for a number of game ticks.
---@param ticks integer Number of ticks to wait.
function wait(ticks) end

---Waits until a function returns true.
---@param func fun(): boolean Function to check.
---@param interval? integer Ticks between checks. Defaults to 1.
function wait_until(func, interval) end

---Waits while a function returns true.
---@param func fun(): boolean Function to check.
---@param interval? integer Ticks between checks. Defaults to 1.
function wait_while(func, interval) end

---Stops the current script when a function returns true.
---@param func fun(): boolean Function to check.
function kill_if(func) end
)lua";
    }

    std::string to_string() const {
        return "ControlFlow";
    }
};

} // namespace lua::api
