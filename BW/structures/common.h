// Based on BWAPI's BW/Position.h

#pragma once
#include "../../types.h"
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
