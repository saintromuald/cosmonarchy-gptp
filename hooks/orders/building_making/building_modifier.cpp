#include "building_modifier.h"
#include <globals.h>

BuildingModifier get_building_modifier(Unit* building, Unit* builder) {
	BuildingModifier modifier = {};
	modifier.multiplier = 1;
	if (!building) {
		return modifier;
	}

	UnitExt* buildingExt = globals.get_unit_ext(building);
	modifier.multiplier *= 1 + 0.1 * buildingExt->unit_vectors[UnitVectorCrucibles].size();

	if (builder && builder->id == UnitId::TerranTalos) {
		modifier.multiplier *= 1.5;
	}

	// First time this is called in a frame, update the value
	if (buildingExt->building_tick_last_update != *elapsedTimeFrames) {
		buildingExt->building_tick_progress += (modifier.multiplier - 1);
		buildingExt->building_tick_last_progress = buildingExt->building_tick_progress;
	}
	buildingExt->building_tick_last_update = *elapsedTimeFrames;

	// Recall the value to calculate the new one and return properly
	buildingExt->building_tick_progress = buildingExt->building_tick_last_progress;
	while (buildingExt->building_tick_progress >= 1) {
		buildingExt->building_tick_progress -= 1;
		modifier.decrease += 1;
	}

	return modifier;
}