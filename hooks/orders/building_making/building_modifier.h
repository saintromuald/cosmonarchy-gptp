#pragma once
#include "SCBW\api.h"

struct BuildingModifier
{
	f32 multiplier;
	u32 decrease;
};

BuildingModifier get_building_modifier(Unit* building, Unit* builder);