#pragma once
#include "common.h"
#pragma pack(1)

struct Unit;
struct Target
{
  Point16       pt;
  Unit*         unit;
};

#pragma pack()