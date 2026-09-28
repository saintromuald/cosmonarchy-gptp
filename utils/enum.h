#pragma once
#include <string>
#include <string_view>
#include <type_traits>

// Convert a PascalCase/camelCase enum name to snake_case (e.g. "ReadyRoom" -> "ready_room")
std::string enum_name_to_snake_case(std::string_view name);

template <typename E> requires std::is_enum_v<E>
std::string enum_to_snake_case(E value) {
    auto name = magic_enum::enum_name(value);
    return enum_name_to_snake_case(name);
}