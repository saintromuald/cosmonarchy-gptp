#pragma once
#include "../definitions.h"
#include "../hook_tools.h"
#include "BW/structures.h"
#include "BW/bwdata.h"

#define MAX_SIGHT_RANGE 20

extern SightRange g_sight_ranges[MAX_SIGHT_RANGE + 1];

void set_max_sight_range();