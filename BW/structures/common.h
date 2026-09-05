// Based on BWAPI's BW/Position.h

#pragma once
#include "../../types.h"
#include "../../interpolation.h"
#include "../rand.h"
#include <algorithm>
#include <concepts>
#include <numbers>

#pragma pack(push, 1)

struct Point8 {
    s8 x;
    s8 y;
};

struct TileCoord {
    u8 x;
    u8 y;

    bool operator==(const TileCoord&) const = default;
};

template <typename T>
void hash_combine(size_t& hash, const T& data) {
    hash = ((hash << 7) | (hash >> (sizeof(size_t) - 7))) ^ std::hash<T>{}(data);
}

template <>
struct std::hash<TileCoord> {
    size_t operator()(TileCoord tile) const {
        size_t hash = 0;
        hash_combine(hash, tile.x);
        hash_combine(hash, tile.y);
        return hash;
    }
};

template <typename>
struct PI;

template <utils::FixedPoint F>
struct PI<F> { static constexpr F value = F::pi(); };

template <std::floating_point F>
struct PI<F> { static constexpr F value = std::numbers::pi_v<F>; };

template <utils::FixedPoint F>
inline auto degs_to_rads(F degs) { return degs * PI<F>::value / F{180}; }

template <utils::FixedPoint F>
inline auto rads_to_degs(F degs) { return degs * F{180} / PI<F>::value; }

namespace detail {
using utils::FloatLike;

inline const u16& g_map_tile_width  = *(u16*)0x0057F1D4;
inline const u16& g_map_tile_height = *(u16*)0x0057F1D6;

// TODO: consider replacing DefaultFloat with PromotionType (v2<T>), and use `using default_float = PromotionType::backing_type;`
template <typename T, FloatLike DefaultFloat = T>
struct v2 {
    T x;
    T y;

    using backing_type = T;
    using default_float_type = DefaultFloat;
    using v2t = v2<T, DefaultFloat>;

    constexpr v2() = default;
    constexpr v2(T x, T y) : x{x}, y{y} {}
    
    template <typename Y>
    explicit constexpr v2(v2<Y> v) : x(v.x), y(v.y) {}

    constexpr bool operator==(const v2<T, DefaultFloat>& b) const { return x == b.x && y == b.y; }
    constexpr operator bool() const { return *this != v2t{}; }

    template <typename T, FloatLike F>
    explicit constexpr operator v2<T, F>() const { return v2<T, F>{T(x), T(y)}; }

    constexpr v2t operator+(v2t b) const { return v2t{x + b.x, y + b.y}; }
    constexpr v2t operator-(v2t b) const { return v2t{x - b.x, y - b.y}; }
    constexpr v2t operator*(T b) const { return v2t{x * b, y * b}; }
    constexpr v2t operator/(T b) const { return v2t{x / b, y / b}; }
    template <FloatLike F = DefaultFloat> constexpr v2<F> operator*(F b) const { return v2<F>{x * b, y * b}; }
    template <FloatLike F = DefaultFloat> constexpr v2<F> operator/(F b) const { return v2<F>{x / b, y / b}; }

    constexpr v2t& operator+=(v2t b) { return *this = *this + b; }
    constexpr v2t& operator-=(v2t b) { return *this = *this - b; }
    constexpr v2t& operator*=(T b) { return *this = *this * b; }
    constexpr v2t& operator/=(T b) { return *this = *this / b; }
    template <FloatLike F = DefaultFloat> constexpr v2<F>& operator*=(F b) { return *this = *this * b; }
    template <FloatLike F = DefaultFloat> constexpr v2<F>& operator/=(F b) { return *this = *this / b; }

    template <FloatLike F = DefaultFloat>
    inline F len() const {
        if constexpr (fpm::is_fixed_v<F>) {
            F dx = abs(x); F dy = abs(y);

            if (dy > dx) {
                const auto temp = dx;
                dx = dy;
                dy = temp;
            }
            if (!dx) return 0;

            return dx * fpm::sqrt(F{1} + (dy / dx) * (dy / dx));
        } else {
            return hypot(F{x}, F{y});
        }
    }

    template <FloatLike F = DefaultFloat>
    inline F distance_to(v2t b) const {
        return vec_to(b).len<F>();
    }

    template <FloatLike F = DefaultFloat>
    inline v2<F> norm() const {
        if (len<F>()) {
            return *this / len<F>();
        }
        return *this;
    }

    template <FloatLike F = DefaultFloat>
    constexpr inline v2<F> vec_to(v2t b) const {
        return v2<F>(b) - v2<F>(*this);
    }

    // NOTE: in this case we always promote to float vector,
    //       since we need negative numbers and integer vectors we currently only use unsigned
    //       and we probably don't want to add signed ones
    //       (let's not multiply types if not needed, also they would have to be bigger, and so on)
    template <FloatLike F = DefaultFloat>
    inline v2<F> dir_to(v2t b) const {
        return vec_to(b).norm<F>();
    }

    constexpr inline T dot(v2t b) const {
        return x * b.x + y * b.y;
    }

    template <FloatLike F = DefaultFloat>
    inline F dot_norm(v2t b) const {
        return norm<F>().dot(b.norm<F>());
    }

    constexpr inline T determinant(v2t b) const {
        return x * b.y - y * b.x;
    }

    template <FloatLike F = DefaultFloat>
    inline F determinant_norm(v2t b) const {
        return norm<F>().determinant(b.norm<F>());
    }

    template <FloatLike F = DefaultFloat>
    constexpr inline v2<F> perpendicular() const {
        return {-y, x};
    }

    template <FloatLike F = DefaultFloat>
    constexpr inline v2<F> reversed() const {
        return {-x, -y};
    }

    template <FloatLike F = DefaultFloat>
    inline v2t rotate(v2t anchor, F angle_degrees) const {
        using namespace std;
        using namespace fpm;
        F x1 = x - anchor.x;
        F y1 = y - anchor.y;
        F radians = degs_to_rads(angle_degrees);
        F x2 = x1 * cos(radians) - y1 * fpm::sin(radians);
        F y2 = x1 * sin(radians) + y1 * fpm::cos(radians);
        return {T(x2 + anchor.x), T(y2 + anchor.y)};
    }

    template <FloatLike F = DefaultFloat>
    inline v2t polar_point(F angle_degrees, F distance) const {
        using namespace std;
        using namespace fpm;
        F rads = degs_to_rads(angle_degrees);
        return {
            T(x + cos(rads) * distance),
            T(y + sin(rads) * distance),
        };
    }

    template <FloatLike F = DefaultFloat>
    inline v2t random_polar_point(F max_distance) const {
        using namespace std;
        using namespace fpm;
        return polar_point<F>(bw::random_float<F>() * 360, max_distance * sqrt(bw::random_float<F>()));
    }

    constexpr inline v2t clamp_to_map_bounds() const {
        return {
            std::clamp<T>(x, T{0}, T(g_map_tile_width * 32)),
            std::clamp<T>(y, T{0}, T(g_map_tile_height * 32)),
        };
    }
};

}; // namespace detail

using v2u16  = detail::v2<u16, fp32>; // Used to be named Point16, uses u16,  default point type used by broodwar
using v2s16  = detail::v2<s16, fp32>; // Signed Point16,           uses s16,  used by some ui clamping / screen space transforms
using v2s32  = detail::v2<s32, fp32>; // Used to be named Vec32,   uses s32,  used by broodwar
using v2     = detail::v2<fp32>;      // Similar to Vec32f,        uses fp32, use as default vector type
using v2fp64 = detail::v2<fp64>;      // Similar to Vec32,         uses fp64, use when double precision fixed point is needed for complex math

using v2f32 = detail::v2<f32>; // Use for UI math

union Quad {
    struct {
        v2 top_left;
        v2 top_right;
        v2 bottom_right;
        v2 bottom_left;
    };
    v2 corners[4]; // Should be clockwise!
};

union Box16 {
    struct {
        s16 left;
        s16 top;
        s16 right;
        s16 bottom;
    };
    struct {
        s16 x;
        s16 y;
        s16 w;
        s16 h;
    };

    bool contains_ltrb(v2u16 point) const {
        return point.x >= left && point.x <= right &&
               point.y >= top && point.y <= bottom;
    }

    bool contains_xywh(v2u16 point) const {
        return point.x >= x && point.x < x + w &&
               point.y >= y && point.y < y + h;
    }
};

union Box32 {
    struct {
        s32 left;
        s32 top;
        s32 right;
        s32 bottom;
    };
    struct {
        s32 x;
        s32 y;
        s32 w;
        s32 h;
    };

    v2u16 center() const {
        return v2u16(
            left + (right  - left) / 2,
            top  + (bottom - top)  / 2);
    }

    Box32 offsets_from_center() const {
        const auto c = center();
        return {
            .left   = left   - c.x,
            .top    = top    - c.y,
            .right  = right  - c.x,
            .bottom = bottom - c.y,
        };
    }
};

struct Bounds {
    union {
        struct {
            s16 left;
            s16 top;
            s16 right;
            s16 bottom;
        };
        struct {
            s16 x;
            s16 y;
            s16 w;
            s16 h;
        };
    };
    u16 width;
    u16 height;
};

C_ASSERT(sizeof(Point8) == 2);
C_ASSERT(sizeof(v2u16) == 4);
C_ASSERT(sizeof(v2s32) == 8);
C_ASSERT(sizeof(Box16) == 8);
C_ASSERT(sizeof(Box32) == 16);
C_ASSERT(sizeof(Bounds) == 12);

#pragma pack(pop)
