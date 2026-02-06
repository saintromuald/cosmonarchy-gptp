#pragma once
#include "BW\api.h"

namespace cm {
struct BuildingModifier {
	fp64 factor;
	u32 integer_step;
};

struct ProgressBuildOptions {
    Unit* builder;
    bool is_constructing;
    bool already_ticked;
    bool use_stock_speed;
    u32 hp_gain;
    u32 shield_gain;
    bool is_askosi = false;
};

BuildingModifier advance_building_modifier(Unit* unit, const ProgressBuildOptions& options = {});

bool progress_build(Unit* unit, const ProgressBuildOptions& options = {});
} // namespace cm