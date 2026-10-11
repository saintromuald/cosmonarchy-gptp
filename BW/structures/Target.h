#pragma once
#include "common.h"
#pragma pack(push, 1)

struct Unit;

struct Target {
    v2u16 pt;
    Unit*   unit;

    bool operator==(const Target& other) const = default;
};

#pragma pack(pop)