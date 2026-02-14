// Based on BWAPI's BW/Position.h

#pragma once
#include "../../types.h"
#include "../../interpolation.h"
#include <algorithm>
#include <source_location>
#pragma pack(1)

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

struct Point16 {
	u16 x;
	u16 y;

	bool operator==(const Point16& other) const = default;
	operator bool() const { return *this != Point16{}; }
};

struct Vec32 {
	s32 x;
	s32 y;
};

struct Vec32f {
	f32 x;
	f32 y;

	Vec32f() = default;
	Vec32f(f32 x, f32 y) : x(x), y(y) {}
	Vec32f(Point16 point) : x(point.x), y(point.y) {}

	bool operator==(const Vec32f& other) const = default;
	operator bool() const { return *this != Vec32f{}; }
	operator Point16() const { return Point16{u16(x), u16(y)}; }
};

// Implemented in api.cpp, declaration moved here to avoid circular include
namespace bw {
#define MaxRandom 32767.0
/// Generates a pseudorandom number between 0 and 32767, inclusive. This is
/// identical to the function used internally by StarCraft, and is guaranteed to
/// generate the same sequence in a replay.
u16 random(std::source_location src = std::source_location::current());

template <utils::FloatLike F = f32> // TODO: remove f32
F random_float(std::source_location src = std::source_location::current()) {
    return F(random()) / F(MaxRandom);
}
}; // namespace bw

namespace detail {
using utils::FloatLike;

inline const u16& g_map_tile_width  = *(u16*)0x0057F1D4;
inline const u16& g_map_tile_height = *(u16*)0x0057F1D6;

template <typename T, FloatLike DefaultFloat = T>
struct v2 {
	T x{};
	T y{};

	constexpr v2() = default;
	constexpr v2(T x, T y) : x{x}, y{y} {}
	constexpr v2(Point16 p) : x(p.x), y(p.y) {}
	constexpr v2(Vec32f v) : x(v.x), y(v.y) {}
	
	template <typename Y>
	explicit constexpr v2(v2<Y> v) : x(v.x), y(v.y) {}

	constexpr bool operator==(const v2<T>& b) const { return x == b.x && y == b.y; }
	constexpr operator bool() const { return *this != v2<T>{}; }
	constexpr operator Point16() const { return Point16{u16(x), u16(y)}; }

	constexpr v2<T> operator+(v2<T> b) const { return v2<T>{x + b.x, y + b.y}; }
	constexpr v2<T> operator-(v2<T> b) const { return v2<T>{x - b.x, y - b.y}; }
	constexpr v2<T> operator*(T b) const { return v2<T>{x * b, y * b}; }
	constexpr v2<T> operator/(T b) const { return v2<T>{x / b, y / b}; }
	template <FloatLike F = DefaultFloat> constexpr v2<F> operator*(F b) const { return v2<F>{x * b, y * b}; }
	template <FloatLike F = DefaultFloat> constexpr v2<F> operator/(F b) const { return v2<F>{x / b, y / b}; }

	constexpr v2<T>& operator+=(v2<T> b) { return *this = *this + b; }
	constexpr v2<T>& operator-=(v2<T> b) { return *this = *this - b; }
	constexpr v2<T>& operator*=(T b) { return *this = *this + b; }
	constexpr v2<T>& operator/=(T b) { return *this = *this - b; }
	template <FloatLike F = DefaultFloat> constexpr v2<F>& operator*=(F b) { return *this = *this + b; }
	template <FloatLike F = DefaultFloat> constexpr v2<F>& operator/=(F b) { return *this = *this - b; }

	template <FloatLike F = DefaultFloat>
	inline F distance() const {
		return fpm::hypot(F{x}, F{y});
	}

	template <FloatLike F = DefaultFloat>
	inline F distance_to(v2<T> b) const {
		return fpm::hypot(F{b.x - x}, F{b.y - y});
	}

	template <FloatLike F = DefaultFloat>
	inline v2<F> normalize() const {
		return *this / distance<F>();
	}

	constexpr inline v2<T> vector_to(v2<T> b) const {
		return (b - *this);
	}

	template <FloatLike F = DefaultFloat>
	inline v2<F> dir_to(v2<T> b) const {
		return vector_to(b).normalize<F>();
	}

	constexpr inline T dot(v2<T> b) const {
		return x * b.x + y * b.y;
	}

	template <FloatLike F = DefaultFloat>
	inline F dot_norm(v2<T> b) const {
		return normalize<F>().dot(b.normalize<F>());
	}

	constexpr inline T determinant(v2<T> b) const {
		return x * b.y - y * b.x;
	}

	template <FloatLike F = DefaultFloat>
	inline F determinant_norm(v2<T> b) const {
		return normalize<F>().determinant(b.normalize<F>());
	}

	constexpr inline v2<T> perpendicular() const {
		return {-y, x};
	}

	constexpr inline v2<T> reversed() const {
		return {-x, -y};
	}

	template <FloatLike F = DefaultFloat>
	inline v2<T> rotate(v2<T> anchor, F angle_degrees) const {
        F x1 = x - anchor.x;
        F y1 = y - anchor.y;
		F radians = angle_degrees / F{57.296};
        F x2 = x1 * fpm::cos(radians) - y1 * fpm::sin(radians);
        F y2 = x1 * fpm::sin(radians) + y1 * fpm::cos(radians);
        return {T(x2 + anchor.x), T(y2 + anchor.y)};
	}

	template <FloatLike F = DefaultFloat>
	inline v2<T> polar_point(F angle_degrees, F distance) const {
		F rads = angle_degrees / F{57.296};
        return {
            T(x + fpm::cos(rads) * distance),
            T(y + fpm::sin(rads) * distance),
        };
    }

	template <FloatLike F = DefaultFloat>
    inline v2<T> random_polar_point(F max_distance) const {
        return polar_point<F>(bw::random_float<F>() * 360, max_distance * fpm::sqrt(bw::random_float<F>()));
    }

	constexpr inline v2<T> clamp_to_map_bounds() const {
		return {
			std::clamp<T>(x, T{0}, T(g_map_tile_width * 32)),
			std::clamp<T>(y, T{0}, T(g_map_tile_height * 32)),
		};
	}
};

}; // namespace detail

// using Point16 = detail::v2<u16, fp32>;
// using Vec32 = detail::v2<u32, fp32>;
using v2  = detail::v2<fp32>;
using v2d = detail::v2<fp64>;
// using Vec32f = v2; // TODO: Remove this later

union Quad {
	struct {
		Vec32f top_left;
		Vec32f top_right;
		Vec32f bottom_right;
		Vec32f bottom_left;
	};
	Vec32f corners[4]; // Should be clockwise!
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

	bool contains_ltrb(Point16 point) const {
		return point.x >= left && point.x <= right &&
			point.y >= top && point.y <= bottom;
	}

	bool contains_xywh(Point16 point) const {
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

    Point16 center() const {
        return {
            .x = u16(left + (right  - left) / 2),
            .y = u16(top  + (bottom - top)  / 2),
		};
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
C_ASSERT(sizeof(Point16) == 4);
C_ASSERT(sizeof(Vec32) == 8);
C_ASSERT(sizeof(Box16) == 8);
C_ASSERT(sizeof(Box32) == 16);
C_ASSERT(sizeof(Bounds) == 12);

#pragma pack()
