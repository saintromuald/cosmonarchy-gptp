//Based on BWAPI's BW/Position.h

#pragma once
#include "../../types.h"
#pragma pack(1)

//V241 for VS2008

struct Point8
{
	s8 x;
	s8 y;
};

struct Point16
{
	u16 x;
	u16 y;

	bool operator ==(const Point16& rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}
	bool operator !=(const Point16& rhs) const
	{
		return !(*this == rhs);
	}
};

struct Vector32
{
	s32 x;
	s32 y;
};

struct Vector32f
{
	f32 x;
	f32 y;
};

struct Box16
{
	union
	{
		struct
		{
			s16 left;
			s16 top;
			s16 right;
			s16 bottom;
		};
		struct
		{
			s16 x;
			s16 y;
			s16 w;
			s16 h;
		};
	};
};

struct Box32
{
	union
	{
		struct
		{
			s32 left;
			s32 top;
			s32 right;
			s32 bottom;
		};
		struct
		{
			s32 x;
			s32 y;
			s32 w;
			s32 h;
		};
	};
};

struct Bounds
{
	union
	{
		struct
		{
			s16 left;
			s16 top;
			s16 right;
			s16 bottom;
		};
		struct
		{
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
C_ASSERT(sizeof(Vector32) == 8);
C_ASSERT(sizeof(Box16) == 8);
C_ASSERT(sizeof(Box32) == 16);
C_ASSERT(sizeof(Bounds) == 12);

//static_assert(sizeof(Point8)  == 2,   "The size of the Point8 structure is invalid");
//static_assert(sizeof(Point16) == 4,   "The size of the Point16 structure is invalid");
//static_assert(sizeof(Point32) == 8,   "The size of the Point32 structure is invalid");
//static_assert(sizeof(Box16)   == 8,   "The size of the Box16 structure is invalid");
//static_assert(sizeof(Box32)   == 16,  "The size of the Box32 structure is invalid");
//static_assert(sizeof(Bounds) == 12, "The size of the Bounds structure is invalid");

#pragma pack()
