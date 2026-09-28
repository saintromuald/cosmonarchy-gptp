// Please include this header instead of including rfl.hpp directly, since this header provides important extensions.
#pragma once
#include <string>
#include <string_view>
#include <ranges>
#include "enum.h"
#include "BW/enumerations.h"
#include <rfl.hpp>

template <typename E> requires std::is_enum_v<E>
struct SnakeCaseEnumReflector {
    using ReflType = std::string;

    static ReflType from(const E& value) {
        return enum_name_to_snake_case(rfl::enum_to_string(value));
    }

    static E to(const ReflType& str) {
        for (const auto& [name, value] : rfl::get_enumerator_array<E>()) {
            if (enum_name_to_snake_case(name) == str) {
                return value;
            }
        }
        throw std::runtime_error{"Invalid enum value: " + str};
    }
};

#define ReflectSnakeCaseEnum(EnumType) \
    template <> struct rfl::Reflector<EnumType> : SnakeCaseEnumReflector<EnumType> {}

// Note: we could technically overload all enums to be snake case automatically,
// but I am not sure if we don't already use non-snake-case names in some places,
// and I don't want to break existing code until we can eliminate the potential outliers. -Veeq7
ReflectSnakeCaseEnum(RaceId::Enum);

template <typename E> requires std::is_enum_v<E>
struct EnumOrString {
    std::variant<E, std::string> value;

    EnumOrString(E enum_) : value{enum_} {}
    EnumOrString(std::string str) : value{std::move(str)} {}

    auto operator<=>(const EnumOrString&) const = default;

    std::optional<E> get_enum() const {
        if (std::holds_alternative<E>(value)) {
            return std::get<E>(value);
        }
        return {};
    }

    std::optional<std::string> get_string() const {
        if (std::holds_alternative<std::string>(value)) {
            return std::get<std::string>(value);
        }
        return {};
    }
};

template <typename E> requires std::is_enum_v<E>
struct std::hash<EnumOrString<E>> {
    size_t operator()(const EnumOrString<E>& value) const {
        if (auto enum_value = value.get_enum()) {
            return std::hash<E>{}(*enum_value);
        } else if (auto str_value = value.get_string()) {
            return std::hash<std::string>{}(*str_value);
        }
        throw std::runtime_error{"Invalid EnumOrString value"};
    }
};

template <typename E> requires std::is_enum_v<E>
struct rfl::Reflector<EnumOrString<E>> {
    using ReflType = std::string;

    static ReflType from(const EnumOrString<E>& value) {
        if (auto enum_value = value.get_enum()) {
            return rfl::Reflector<E>::from(*enum_value);
        } else if (auto str_value = value.get_string()) {
            return *str_value;
        }
        throw std::runtime_error{"Invalid EnumOrString value"};
    }

    static EnumOrString<E> to(const ReflType& str) {
        try {
            E enum_value = rfl::Reflector<E>::to(str);
            return EnumOrString<E>{enum_value};
        } catch (const std::runtime_error&) {
            return EnumOrString<E>{str};
        }
    }
};

// Serializes as forward slash, deserializes as backslash, to match the behavior of the original SFile path handling code
struct SFilePath : std::string {
    using std::string::string;
    using std::string::operator=;

    SFilePath(const std::string& str) : std::string{str} {}
    SFilePath(std::string&& str) : std::string{std::move(str)} {}
};

template <>
struct rfl::Reflector<SFilePath> {
    using ReflType = std::string;

    static ReflType from(const SFilePath& value) {
        std::string result = value;
        std::ranges::replace(result, '\\', '/');
        return result;
    }

    static SFilePath to(const ReflType& str) {
        std::string result = str;
        std::ranges::replace(result, '/', '\\');
        return SFilePath{std::move(result)};
    }
};
