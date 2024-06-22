#pragma once
#include "BW\bwdata.h"

namespace TechId {
    namespace dat = techdata_dat;

    inline u32 energy_cost(Enum id) {
        return dat::energy_cost[id] * 256;
    }
}