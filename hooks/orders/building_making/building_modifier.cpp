#include "building_modifier.h"
#include <globals.h>

BuildingModifier get_building_modifier(Unit* building, Unit* builder, bool constructing) {
	BuildingModifier modifier = {};
	modifier.multiplier = 1;
	if (!building) {
		return modifier;
	}

	UnitExt* buildingExt = get_unit_ext(building);
	if (constructing) {
		if ((builder && builder->id == UnitId::TerranTalos)
			|| building->create_origin == UnitId::ProtossArtisan
			|| building->create_origin == UnitId::ZergGosvileth) {
			modifier.multiplier *= 1.5;
		}
	} else {
		modifier.multiplier *= 1 + 0.15 * buildingExt->unit_vectors[UnitVectorCrucibles].size();
	}

	// First time this is called in a frame, update the value
	if (buildingExt->building_tick_last_update != *g_elapsed_time_frames) {
		buildingExt->building_tick_progress += (modifier.multiplier - 1);
		buildingExt->building_tick_last_progress = buildingExt->building_tick_progress;
	}
	buildingExt->building_tick_last_update = *g_elapsed_time_frames;

	// Recall the value to calculate the new one and return properly
	buildingExt->building_tick_progress = buildingExt->building_tick_last_progress;
	while (buildingExt->building_tick_progress >= 1) {
		buildingExt->building_tick_progress -= 1;
		modifier.decrease += 1;
	}

	return modifier;
}