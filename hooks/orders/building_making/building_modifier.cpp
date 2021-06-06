#include "building_modifier.h"
#include <globals.h>

BuildingModifier get_building_modifier(Unit* unit)
{
	BuildingModifier modifier = {};

	UnitExt* unitExt = globals.GetUnitExt(unit);
	modifier.multiplier = 1 + 0.05 * unitExt->unitVectors[UnitVectorCrucibles].size();
	if (unitExt->unitVectors[UnitVectorCrucibles].size())
	{
		modifier.decrease = 1 + (unitExt->unitVectors[UnitVectorCrucibles].size() - 1) / 20;
		modifier.period = modifier.decrease / 0.05 / unitExt->unitVectors[UnitVectorCrucibles].size();
	}

	return modifier;
}