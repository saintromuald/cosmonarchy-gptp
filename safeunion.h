#pragma once

#include <type_traits>
#include <algorithm>

namespace safeunion {
template <auto E, typename T> requires std::is_enum_v<decltype(E)>
struct Entry {
    using EnumType = std::remove_cvref_t<decltype(E)>;
    using Type = T;
    static constexpr auto EnumValue = E;
};

template <typename T>
concept IsEntry = std::is_same_v<T, Entry<T::EnumValue, typename T::Type>>;

template <IsEntry... Entries>
requires (std::is_same_v<typename Entries::EnumType,
std::common_type_t<typename Entries::EnumType...>> && ...)
class Union {
public:
    using EnumType = std::common_type_t<typename Entries::EnumType...>;
    static constexpr size_t Size = std::max({sizeof(typename Entries::Type)...});

    template <typename T> requires (std::is_same_v<T, typename Entries::Type> || ...)
    T* cast(EnumType validating_enum) {
        const auto matches = ((std::is_same_v<T, typename Entries::Type>
            && validating_enum == Entries::EnumValue) || ...);
        return matches ? reinterpret_cast<T*>(data) : nullptr;
    }

    template <typename T> requires (std::is_same_v<T, typename Entries::Type> || ...)
    const T* cast(EnumType validating_enum) const {
        const auto matches = ((std::is_same_v<T, typename Entries::Type>
            && validating_enum == Entries::EnumValue) || ...);
        return matches ? reinterpret_cast<const T*>(data) : nullptr;
    }

private:
    alignas(typename Entries::Type...) char data[Size];
};
} // namespace safeunion