#pragma once
#include "../scripting.h"
#include <fstream>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <sstream>

namespace lua::api {

namespace detail {

template <typename T>
concept Stringable = requires(const T t) {
    { t.to_string() } -> std::same_as<std::string>;
};

template <typename T>
void install(ScriptManager& manager, sol::state& lua) {
    if constexpr (requires { T::install(manager, lua); }) {
        static_assert(Stringable<T>, "Scripting type must have a 'std::string to_string() const' method");
        T::install(manager, lua);
    } else if constexpr (std::is_enum_v<T>) {
        manager.install_enum<T>();
    } else {
        static_assert(false, "Scripting type must have a static install method or be an enum");
    }
}

template <typename T>
void lsp_annotations(std::ofstream& out) {
    if constexpr (requires { std::declval<std::ofstream&>() << T::lsp_annotations(); }) {
        out << T::lsp_annotations();
    } else if constexpr (requires { T::lsp_annotations(out); }) {
        T::lsp_annotations(out);
    } else if constexpr (std::is_enum_v<T>) {
        const auto type_name = magic_enum::enum_type_name<T>();
        out << "---@enum " << type_name << "\n";
        out << type_name << " = {\n";
        std::string example_value = "";
        for (auto [value, name] : magic_enum::enum_entries<T>()) {
            if (example_value.empty()) {
                example_value = name;
            }
            out << "    " << name << " = " << static_cast<s64>(value) << ",\n";
        }
        out << "}\n\n";
        out << "---Returns short name for specified value, for example `" << example_value << "`.\n";
        out << "---@param value " << type_name << "\n";
        out << "---@return string name\n";
        out << "function " << type_name << ".name(value) end\n\n";
        out << "---Returns long name for specified value, for example `" << type_name << "." << example_value << "`.\n";
        out << "---@param value " << type_name << "\n";
        out << "---@return string name\n";
        out << "function " << type_name << ".long_name(value) end\n\n";
    } else {
        static_assert(false, "Scripting type must have a static lsp_annotations method or be an enum");
    }
}

} // namespace detail

template <typename... Ts>
struct ScriptModule {
    static void install(ScriptManager& manager, sol::state& lua) {
        (detail::install<Ts>(manager, lua), ...);
    }

    static void lsp_annotations(std::ofstream& out) {
        (detail::lsp_annotations<Ts>(out), ...);
    }
};

inline void assert_unit_id(UnitId::Enum unit_id) {
    if (unit_id == UnitId::None) {
        throw sol::error("UnitId.None is not a valid unit id");
    }
    if (unit_id < 0 || unit_id >= UnitLimit) {
        throw sol::error(std::format("UnitId {} is out of range [0, {}]", u32(unit_id), UnitLimit));
    }
}

inline void assert_positive(const char* name, auto value) {
    if (value < 0) {
        throw sol::error(std::format("{} must be positive, got {}", name, value));
    }
}

template <typename Func>
inline std::string array_to_string(std::ranges::input_range auto&& range, Func format_func = std::identity{}) {
    std::stringstream ss;
    bool first = true;
    for (const auto& elem : range) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        ss << format_func(elem);
    }
    return ss.str();
}

} // namespace lua::api
