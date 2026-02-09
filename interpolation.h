#pragma once

#include "_vendor_modified/fpm/fixed.hpp"

#include <concepts>

namespace utils {

template <typename T>
concept FloatLike = std::floating_point<T> || fpm::is_fixed_v<T>;

template <FloatLike T>
constexpr T map(T value, T from_a, T from_b, T to_a, T to_b) {
    T t = (value - from_a) / (from_b - from_a);
    return t * (to_b - to_a) + to_a;
}

template <FloatLike T>
constexpr T normalize(T value, T a, T b) {
    return (value - a) / (b - a);
}

template <FloatLike T>
constexpr T lerp(T t, T a, T b) {
    return t * (b - a) + a;
}

template <FloatLike T>
constexpr T clamp(T value, T a, T b) {
    if (value < a) return a;
    if (value > b) return b;
    return value;
}

template <FloatLike T>
constexpr T ease_in(T t) {
    return t * t;
}

template <FloatLike T>
constexpr T ease_out(T t) {
    return T{1} - (t - T{1}) * (t - T{1});
}

template <FloatLike T>
constexpr T ease_in_out(T t) {
    return lerp(t, ease_in(t), ease_out(t));
}

template <FloatLike T>
constexpr T smoothstep(T t, T a, T b) {
    return lerp(ease_in_out(t), a, b);
}

} // namespace utils
