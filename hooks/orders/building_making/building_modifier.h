#pragma once
#include "BW\api.h"

struct BuildingModifier {
	f32 multiplier;
	u32 decrease;
};

BuildingModifier get_building_modifier(Unit* building, Unit* builder, bool constructing);