#pragma once
#include "../scripting.h"
#include <fstream>
#include <magic_enum/magic_enum.hpp>

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
        out << T::lsp_annotations() << "\n";
    } else if constexpr (requires { T::lsp_annotations(out); }) {
        T::lsp_annotations(out);
    } else if constexpr (std::is_enum_v<T>) {
        const auto type_name = magic_enum::enum_type_name<T>();
        out << "---@enum " << type_name << "\n";
        out << type_name << " = {\n";
        for (auto [value, name] : magic_enum::enum_entries<T>()) {
            out << "    " << name << " = " << static_cast<s64>(value) << ",\n";
        }
        out << "}\n\n";
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

} // namespace lua::api
