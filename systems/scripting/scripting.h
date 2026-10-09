#pragma once
#include "BW/api.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <list>
#include <string>
#include <memory>
#include <optional>
#include <format>
#include <tuple>
#include <vector>
#include <magic_enum/magic_enum.hpp>

#include <sol/sol.hpp>

namespace lua {

enum class ScriptType { Mpq, Crigs };
enum class SleepState { Sleep, Continue, Error };

struct ScriptHandle {
    u32 id;

    operator bool() const { return id != 0; }
};

struct Script {
    // Indentification
    u32 id;
    ScriptType type;
    bool is_multirun;
    std::string label;

    // Game state
    bool is_alive = true;
    u8 player;

    // Lua
    sol::thread thread;
    sol::coroutine coroutine;
    std::vector<sol::object> args;

    // Wait state
    u32 sleep_until = 0;
    std::function<SleepState(Script&)> sleep_func;
    std::vector<std::function<bool(Script&)>> kill_conditions;

    SleepState sleep(u32 ticks) {
        sleep_until = g_elapsed_time_frames + ticks;
        return SleepState::Sleep;
    }

    std::string type_name() const {
        switch (type) {
            case ScriptType::Mpq:   return is_multirun ? "mpq [multirun]" : "mpq";
            case ScriptType::Crigs: return is_multirun ? "crigs [multirun]" : "crigs";
        }
        return "unknown";
    }

    std::string name() const {
        if (label.empty()) {
            return std::format("Script #{} ({})", id, type_name());
        }
        return std::format("Script #{} ({}: {})", id, type_name(), label);
    }
};

class ScriptManager {
public:
    ScriptManager();

    Script* load_script_mpq(const char* path, u8 player);
    Script* load_script_string(std::string name, std::string_view code, u8 player);
    Script& load_script_multirun(sol::function function, std::vector<sol::object> args, const Script& parent);

    static void generate_lsp_annotations();
    bool tick();

    std::string reporting_label(std::string_view label) {
        auto* script = maybe_current_script();
        if (script && !label.empty()) return std::format(" {}@{}", label, script->name());
        if (script)                   return std::format(" {}", script->name());
        if (!label.empty())           return std::format(" {}", label);
        return "";
    }

    template <typename... Args>
    void debug(std::string_view label, std::format_string<Args...> fmt, Args&&... args) {
        auto msg = std::format(fmt, std::forward<Args>(args)...);
        bw::println("{}[Debug]{}\n{}{}\n", TextColor::Yellow, reporting_label(label), TextColor::White, msg);
    }

    template <typename... Args>
    void report_error(std::string_view label, std::format_string<Args...> fmt, Args&&... args) {
        auto msg = std::format(fmt, std::forward<Args>(args)...);
        bw::println("{}[Error]{}\n{}{}\n", TextColor::Red, reporting_label(label), TextColor::White, msg);
    }

    void report_error(std::string_view label, sol::error error) {
        report_error(label, "{}", error.what());
    }

    template <typename Enum> requires std::is_enum_v<Enum>
    void install_enum(std::string_view name = magic_enum::enum_type_name<Enum>()) {
        constexpr auto entries = magic_enum::enum_entries<Enum>();
        sol::table values = m_root.create_table(0, static_cast<int>(entries.size()));
        for (auto [value, enum_name] : entries) {
            values.set(enum_name, value);
        }
        values.set_function("name", [](Enum value) {
            auto name = magic_enum::enum_name(value);
            if (name.empty()) return std::format("{}", value);
            return std::string{name};
        });
        values.set_function("long_name", [](Enum value) {
            auto name = magic_enum::enum_name(value);
            if (name.empty()) return std::format("{}({})", magic_enum::enum_type_name<Enum>(), value);
            return std::format("{}.{}", magic_enum::enum_type_name<Enum>(), name);
        });

        sol::table mt = m_root.create_table_with(
            sol::meta_function::new_index, sol::detail::fail_on_newindex,
            sol::meta_function::index, values
        );
        m_root.create_named_table(name, sol::metatable_key, mt);
    }

    Script* maybe_current_script() const { return m_current_script != m_scripts.end() ? &*m_current_script : nullptr; }
    Script& current_script() const { if (m_current_script == m_scripts.end()) throw sol::error("No current script"); return *m_current_script; }

    Script* find_script(ScriptHandle handle) {
        auto it = std::find_if(m_scripts.begin(), m_scripts.end(), [handle](const Script& script) { return script.id == handle; });
        if (it != m_scripts.end()) {
            return &*it;
        }
        return nullptr;
    }

private:
    void install_handlers();

private:
    u32 m_next_script_id = 0;
    sol::state m_root;
    std::list<Script> m_scripts;
    std::list<Script>::iterator m_current_script = m_scripts.end();
};

inline std::unique_ptr<ScriptManager> g_script_manager;

} // namespace lua
