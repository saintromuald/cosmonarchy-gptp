#include "building_modifier.h"
#include <globals.h>
#include "trigger_system\trigger_system.h"

BuildingModifier get_building_modifier(Unit* unit, Unit* builder, bool constructing) {
    BuildingModifier modifier = {};
    modifier.multiplier = 1;
    if (!unit) {
        return modifier;
    }

    UnitExt* building_ext = get_unit_ext(unit);
    if (constructing) {
        /*
        if ((builder && builder->id == UnitId::TerranTalos)
            || building->create_origin == UnitId::ProtossArtisan
            || building->create_origin == UnitId::ZergGosvileth) {
            modifier.multiplier *= 1.5;
        }
        */
    } else if (building_ext->unit_vectors[UnitVectorCrucibles].size()) {
        modifier.multiplier *= 1.5;
    }
    
    const auto& cm_player = g_globals->players[unit->player];
    modifier.multiplier *= cm_player.build_multiplier;
    if (bw::get_game_kind() == GameKind_Debug) {
        modifier.multiplier *= 32;
    }

    if (building_ext->building_tick_last_update != g_elapsed_time_frames) {
        building_ext->building_tick_last_update = g_elapsed_time_frames;
        building_ext->building_tick_progress += (modifier.multiplier - 1);
    }
    modifier.decrease = floor(building_ext->building_tick_progress);
    building_ext->building_tick_progress -= modifier.decrease;

    return modifier;
}