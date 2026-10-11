#pragma once
#include "../script_common.h"

namespace lua::api::core {

class Prints {
public:
    static void install(ScriptManager& manager, sol::state& lua) {
        lua.set_function("print", [&manager](sol::this_state state, sol::variadic_args args) {
            bw::println(TextColor::White, "{}", varargs_to_string(state, args));
        });
        lua.set_function("cprint", [&manager](sol::this_state state, TextColor color, sol::variadic_args args) {
            bw::println(color, "{}", varargs_to_string(state, args));
        });
        lua.set_function("debug", [&manager](sol::this_state state, sol::variadic_args args) {
            bw::println("{}{}\n{}{}\n", TextColor::Yellow, manager.current_script().name(), TextColor::White, varargs_to_string(state, args));
        });
    }

    static const char* lsp_annotations() {
        return R"lua(
---Prints a message in white.
---@param ... any Values to print.
function print(...) end

---Prints a message in the selected color.
---@param color TextColor Message color.
---@param ... any Values to print.
function cprint(color, ...) end

---Prints a message with the current script name.
---@param ... any Values to print.
function debug(...) end
)lua";
    }

    std::string to_string() const {
        return "Prints";
    }

private:
    static std::string varargs_to_string(sol::this_state state, sol::variadic_args args) {
        sol::function to_string = sol::state_view{state}["tostring"] ;
        std::stringstream ss;
        bool first = true;
        for (const auto argument : args) {
            if (first) {
                first = false;
            } else {
                ss << " ";
            }
            sol::function_result result = to_string(argument);
            if (!result.valid()) {
                sol::error error = result;
                throw error;
            }
            ss << result.get<std::string>();
        }
        return ss.str();
    }
};

} // namespace lua::api::core
