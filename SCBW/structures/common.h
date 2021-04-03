//Based on BWAPI's BW/Position.h

#pragma once
#include "../../types.h"
#pragma pack(1)

//V241 for VS2008

struct Point8 {
  int8 x;
  int8 y;
};

struct Point16 {
  uint16 x;
  uint16 y;

  bool operator ==(const Point16 &rhs) const {
    return x == rhs.x && y == rhs.y;
  }
  bool operator !=(const Point16 &rhs) const {
    return !(*this == rhs);
  }
};

struct Point32 {
  int x;
  int y;
};

struct Box16 {
  int16 left;     // x
  int16 top;      // y
  int16 right;    // w
  int16 bottom;   // h
};

struct Box32 {
  int32 left;
  int32 top;
  int32 right;
  int32 bottom;
};

struct Bounds {
  uint16 left;
  uint16 top;
  uint16 right;
  uint16 bottom;
  uint16 width;
  uint16 height;
};

C_ASSERT(sizeof(Point8) == 2);
C_ASSERT(sizeof(Point16) == 4);
C_ASSERT(sizeof(Point32) == 8);
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
