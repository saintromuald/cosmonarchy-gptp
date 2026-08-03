#pragma once
#include <format>
#include <source_location>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <utility>
#include <ranges>
#include <magic_enum/magic_enum.hpp>
#include "types.h"
#include "interpolation.h"

#define MaxRandom 32767.0

namespace bw {

struct RandMessage {
    std::string message;

    RandMessage() = default;

    // Intentionally deleting copy to enforce move for performance
    RandMessage(const RandMessage&) = delete;
    RandMessage& operator=(const RandMessage&) = delete;
    RandMessage(RandMessage&&) = default;
    RandMessage& operator=(RandMessage&&) = default;

    template <typename... Ts>
    RandMessage(std::format_string<Ts...> fmt, Ts&&... args) : message(
        std::vformat(fmt.get(), std::make_format_args(args...))
    ) {}

    template <typename... Ts>
    void assign_if_empty(std::format_string<Ts...> fmt, Ts&&... args) {
        if (message.empty()) {
            message = std::vformat(fmt.get(), std::make_format_args(args...));
        }
    }
};

enum class LogType {
    Sync  = 1,
    Local = 2,
};

class RandomLogger {
public:
    void open();

    u32 log_random(LogType log_type, u32 value, RandMessage message, std::source_location src, u32 old_last_random);

    std::string buffered_string() const { return m_buffer.str(); }

private:
    std::fstream m_output;
    std::stringstream m_buffer;
};

inline RandomLogger g_random_logger;

// Generates a pseudorandom number between 0 and 32767, inclusive. This is
// identical to the function used internally by StarCraft, and is guaranteed to
// generate the same sequence in a replay.
u16 random(RandMessage message = {"random"}, std::source_location src = std::source_location::current());

// Generates a pseudorandom number between min and max, inclusive. This is
// identical to the function used internally by StarCraft, and is guaranteed to
// generate the same sequence in a replay.
// NOTE: The RNG has a maximum distance of 0 - 32767.
u32 rand_between(u32 min, u32 max, RandMessage message = {}, std::source_location src = std::source_location::current());

template <typename T> requires std::is_enum_v<T>
std::string enum_name_or_value(T value) {
    const auto name = magic_enum::enum_name(value);
    return !name.empty() ? std::string{name} : std::to_string(std::to_underlying(value));
}

template <typename T> requires std::is_enum_v<T>
T rand_between(T min, T max, RandMessage message = {}, std::source_location src = std::source_location::current()) {
    message.assign_if_empty("rand_between {} - {}", enum_name_or_value(min), enum_name_or_value(max));
    return static_cast<T>(rand_between(static_cast<u32>(min), static_cast<u32>(max), std::move(message), src));
}

// Similar to above, but isn't synced, meaning it should be used for local only code
// Uses C stdlib rand
u32 rand_between_local(u32 min, u32 max, RandMessage message = {}, std::source_location src = std::source_location::current());

template <typename T> requires std::is_enum_v<T>
T rand_between_local(T min, T max, RandMessage message = {}, std::source_location src = std::source_location::current()) {
    message.assign_if_empty("rand_between_local {} - {}", enum_name_or_value(min), enum_name_or_value(max));
    return static_cast<T>(rand_between_local(static_cast<u32>(min), static_cast<u32>(max), std::move(message), src));
}

template <utils::FloatLike F = fp32>
F random_float(RandMessage message = {"random_float"}, std::source_location src = std::source_location::current()) {
    return F(random(std::move(message), src)) / F(MaxRandom);
}

std::string range_str(const std::ranges::range auto& range) {
    std::stringstream ss;
    ss << "[";
    for (const auto& [i, it] : range | std::views::enumerate) {
        if (i != 0) {
            ss << ", ";
        }
        ss << it;
    }
    ss << "]";
    return ss.str();
}

auto rand_one(const std::ranges::range auto& range, RandMessage message = {}, std::source_location src = std::source_location::current()) -> decltype(*std::ranges::begin(range)) {
    message.assign_if_empty("rand_one {}", range_str(range));
    return *std::ranges::next(std::ranges::begin(range), bw::rand_between(0, std::ranges::size(range) - 1, std::move(message), src));
}

auto rand_one_local(const std::ranges::range auto& range, RandMessage message = {}, std::source_location src = std::source_location::current()) -> decltype(*std::ranges::begin(range)) {
    message.assign_if_empty("rand_one_local {}", range_str(range));
    return *std::ranges::next(std::ranges::begin(range), bw::rand_between_local(0, std::ranges::size(range) - 1, std::move(message), src));
}

}; // namespace bw
