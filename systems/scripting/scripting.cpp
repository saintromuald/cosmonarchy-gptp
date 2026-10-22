#include "scripting.h"
#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <sstream>
#include <filesystem>

#include <sol/sol.hpp>

#include "../BW/api.h"
#include "../../utils/starcraft_path.h"
#include "../storm/sfile.h"
#include "api/script_api.h"

namespace lua {

ScriptManager::ScriptManager() {
    m_root.open_libraries(
        sol::lib::base,
        sol::lib::package,
        sol::lib::math,
        sol::lib::coroutine,
        sol::lib::string,
        sol::lib::table,
        sol::lib::utf8
    );
    install_handlers();
    api::install(*this, m_root);
}

void ScriptManager::install_handlers() {
    m_root.set_function("__error_handler", [this](const std::string& msg) {
        auto& script = current_script();
        bw::println("{}[Error] {}\n{}{}\n", TextColor::Red, script.name(), TextColor::White, msg);
        return msg;
    });
    sol::function::set_default_handler(m_root["__error_handler"]);

    m_root.add_package_loader([](sol::this_state state, std::string path) {
        std::ranges::replace(path, '.', '/');
        path = "lua/" + path + ".lua";
        auto code = sfile::try_load_file(path.c_str());
        if (!code) {
            return sol::make_object(state, "\n\tno MPQ file: '" + path + "'");
        }
        auto result = sol::state_view{state}.load_buffer(code.data.get(), code.size, path);
        if (!result.valid()) {
            sol::error err = result;
            throw err;
        }
        return sol::make_object(state, sol::function{result});
    }, true);

    m_root["math"]["random"] = sol::overload(
        []() -> double {
            // This stinks, lua doesn't support fixed point, but eh dunno what we can do about that?
            // TODO: Investigate replacing lua interal float type, or use usertype if not possible -Veeq7
            return bw::random_float<double>();
        },
        [](lua_Integer max) -> lua_Integer {
            if (max < 1) {
                throw sol::error("bad argument #1 to 'random'");
            }
            return bw::rand_between(1, max);
        },
        [](lua_Integer min, lua_Integer max) -> lua_Integer {
            if (min > max) {
                throw sol::error("interval is empty");
            }
            return bw::rand_between(min, max);
        }
    );
    m_root["math"]["randomseed"] = sol::nil;
}


Script* ScriptManager::load_script_mpq(const char* path, u8 player) {
    auto thread = sol::thread::create(m_root);
    auto code = sfile::try_load_file(path);
    if (!code) {
        report_error("load_script_mpq", "MPQ file not found: '{}'", path);
        return nullptr;
    }
    auto result = thread.state().load_buffer(code.data.get(), code.size, path);
    if (!result.valid()) {
        report_error("load_script_mpq", result);
        return nullptr;
    }
    m_scripts.push_back(Script{
        .id = ++m_next_script_id,
        .type = ScriptType::Mpq,
        .is_multirun = false,
        .label = path,
        .player = player,
        .thread = thread,
        .coroutine = result,
    });
    return &m_scripts.back();
}

Script* ScriptManager::load_script_string(std::string name, std::string_view code, u8 player) {
    auto thread = sol::thread::create(m_root);
    auto result = thread.state().load(code, name);
    if (!result.valid()) {
        report_error("load_script_string", result);
        return nullptr;
    }
    m_scripts.push_back(Script{
        .id = ++m_next_script_id,
        .type = ScriptType::Crigs,
        .is_multirun = false,
        .label = std::move(name),
        .player = player,
        .thread = thread,
        .coroutine = result,
    });
    return &m_scripts.back();
}

Script& ScriptManager::load_script_multirun(sol::function function, std::vector<sol::object> args, const Script& parent) {
    auto thread = sol::thread::create(m_root);
    auto coro = sol::coroutine{thread.state(), function};
    m_current_script = m_scripts.insert(m_current_script, Script{
        .id = ++m_next_script_id,
        .type = parent.type,
        .is_multirun = true,
        .label = parent.label,
        .player = parent.player,
        .thread = thread,
        .coroutine = coro,
        .args = std::move(args),
    });
    return *m_current_script;
}

void ScriptManager::generate_lsp_annotations() {
    std::error_code ec;
    const auto dir = bw::get_starcraft_path() / "lua_lsp";
    fs::create_directories(dir, ec);

    std::ofstream api(dir / "api.lua");
    api::generate_lsp_annotations(api);

    auto lib_path = fs::canonical(dir).string();
    std::ranges::replace(lib_path, '\\', '/');

    std::ofstream luarc(dir / ".luarc.json");
    luarc << std::format(R"json({{
  "$schema": "https://raw.githubusercontent.com/LuaLS/vscode-lua/master/setting/schema.json",
  "runtime.version": "Lua 5.4",
  "format.enable": true,
  "workspace.library": ["{}"]
}})json", lib_path);
}

bool ScriptManager::tick() {
    for (m_current_script = m_scripts.begin(); m_current_script != m_scripts.end();) {
        auto& script = *m_current_script;
        if (!script.is_alive) {
            m_current_script = m_scripts.erase(m_current_script);
            continue;
        }
        if (script.sleep_until > g_elapsed_time_frames) {
            ++m_current_script;
            continue;
        }
        bool should_continue = false;
        for (auto& condition : script.kill_conditions) {
            if (condition(script)) {
                m_current_script = m_scripts.erase(m_current_script);
                should_continue = true;
                break;
            }
        }
        if (should_continue) {
            continue;
        }
        if (script.sleep_func) {
            auto result = script.sleep_func(script);
            if (result == SleepState::Error) {
                m_current_script = m_scripts.erase(m_current_script);
                continue;
            } else if (result == SleepState::Sleep) {
                ++m_current_script;
                continue;
            }
            script.sleep_func = {};
        }

        auto result = !script.args.empty() ? script.coroutine(sol::as_args(script.args)) : script.coroutine();
        script.args.clear();
        if (!result.valid()) {
            report_error("", result);
            m_current_script = m_scripts.erase(m_current_script);
            continue;
        }
        if (result.status() == sol::call_status::yielded) {
            // After yielding m_current_script may have changed, or have set a sleep, or maybe we wanted to just continue execution on same script
            // Therefore we don't increment the iterator here, so we can check the script again on the next tick
            continue;
        }

        m_current_script = m_scripts.erase(m_current_script);
    }
    m_root.collect_garbage();
    return !m_scripts.empty();
}

} // namespace lua
