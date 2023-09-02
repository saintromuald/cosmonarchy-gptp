#include "building_modifier.h"
#include <globals.h>

BuildingModifier get_building_modifier(Unit* building, Unit* builder, bool constructing) {
	BuildingModifier modifier = {};
	modifier.multiplier = 1;
	if (!building) {
		return modifier;
	}

	UnitExt* building_ext = get_unit_ext(building);
	if (constructing) {
		/*
		if ((builder &&  builder->id == UnitId::TerranTalos)
		||   building->create_origin == UnitId::ProtossArtisan
		||   building->create_origin == UnitId::ZergGosvileth)
		{
			modifier.multiplier *= 1.5;
		}
		*/
	} else {
		modifier.multiplier *= 1 + 0.25 * building_ext->unit_vectors[UnitVectorCrucibles].size();
	}

	// First time this is called in a frame, update the value
	if (building_ext->building_tick_last_update != *g_elapsed_time_frames) {
		building_ext->building_tick_progress += (modifier.multiplier - 1);
		building_ext->building_tick_last_progress = building_ext->building_tick_progress;
	}
	building_ext->building_tick_last_update = *g_elapsed_time_frames;

	// DimensionalRecall the value to calculate the new one and return properly
	building_ext->building_tick_progress = building_ext->building_tick_last_progress;
	while (building_ext->building_tick_progress >= 1) {
		building_ext->building_tick_progress -= 1;
		modifier.decrease += 1;
	}

	return modifier;
}